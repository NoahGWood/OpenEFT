import os
import uuid
import wsq
import cv2
import tempfile
from sys import stdout
from imutils import resize
from PIL import Image
from subprocess import check_output
from core.align import GetEFT
from core.fd258_ocr import OCR_LOCATIONS
from core.eft_helper import Type1, Type2, Type14, EFT

class Core:
    def __init__(self, ifile, out=None, debug=True, template="templates/fd-258.png",nbis="build/bin"):
        self.cwd = os.getcwd()
        self.ifile = ifile
        if out:
            self.ofile=out
        else:
            self.ofile=str(uuid.uuid1())[0:10]+'.eft'
        self.dbg=debug
        self.tmpDir = tempfile.TemporaryDirectory()
        self.nbis = os.path.abspath(nbis)
        self.tools_needed = ["nfseg","nfiq"]
        self.template = self.load_img(template)
        self.test_tools()
    
    def __del__(self):
        # Force clean up tempdir
        self.tmpDir.cleanup()

    def test_tools(self):
        """Test to ensure that required NBIS tools installed"""
        for each in self.tools_needed:
            if not os.path.exists(os.path.join(self.nbis, each)):
                raise FileNotFoundError("[ERROR] Required tool {} is not available at location {}".format(each, self.nbis))

    def load_img(self, fname):
        """Loads an image, verifies it was loaded, and returns the image."""
        img = cv2.imread(fname)
        self.check_img(img, fname)
        return img
    
    def check_img(self, img, fname=None):
        """Tests whether image was loaded by cv2 and raises exception if failed."""
        if(len(img)==0):
            # Image not found
            raise FileNotFoundError("File '{}' unable to be loaded. Check path and retry.".format(fname))

    def align_img(self, img):
        """Used to align images."""
        try:
            aligned = GetEFT(img)
        except:
            # EFT contours weren't found, assume matches template.
            aligned = img
        aligned = cv2.resize(aligned, (self.template.shape[0], self.template.shape[1]))
        return aligned
    
    def save_image(self, img, fname):
        """Saves an image to the tmp directory and returns the filename."""
        fname = os.path.join(self.tmpDir.name, fname)
        cv2.imwrite(fname,img)
        return fname
    
    def get_fp(self, img):
        """Sections fingerprints from a scanned fd258 image, saves to
            temp directory, and returns an array of fingerprint image
            locations for further processing.
        """
        results = []
        for loc in OCR_LOCATIONS:
            # Get bounding box
            (x,y,w,h) = loc.bbox
            # Extract areas of interest
            roi = img[y:y+h,x:x+w]
            # Convert to grayscale
            gray = cv2.cvtColor(roi, cv2.COLOR_RGB2GRAY)
            # Generate tmp file name
            # Note: Must use jpeg for NIST compatibility
            #   Thankfully, CV2 uses libjasper, so it should 
            #   be saved as a lossless jpg
            fname = loc.id + '.jpg'
            # Save grayscaled image to tmp file
            fname = self.save_image(gray, fname)
            results.append(fname)
            if self.dbg:
                # Show the fingerprint that was sectioned
                cv2.imshow(loc.id, resize(roi, width=500,height=500))
        if self.dbg:
            cv2.waitKey(0)
            cv2.destroyAllWindows()
        return results
    
    def segment(self, fname):
        """Segments a slap image into multiple files and converts them into
            .wsq format.
        """
        results = []
        # Change directory to temp directory
        os.chdir(self.tmpDir.name)
        # Create execution string
        exec_str = os.path.join(self.nbis, "nfseg")
        if "L" in fname:
            exec_str = exec_str + " 13 1 1 2 1 "
        elif "R" in fname:
            exec_str = exec_str + " 14 1 1 2 1 "
        else:
            # not sure if right or left
            exec_str = exec_str + " 00 1 1 2 1 "
        raw = check_output(exec_str + fname, shell=True).decode(stdout.encoding)
        r = raw.split("\n")
        for each in r:
            if len(each) > 0:
                # Get filename from stdout
                x = each.split(" -> ")[0].replace("FILE ","")
                results.append(os.path.abspath(x))
        # Return to base directory
        os.chdir(self.cwd)
        return results
    
    def _get_iq(self, fname):
        """Returns the NFIQ score (int, 1-5) of .wqs file fname."""
        os.chdir(self.tmpDir.name)
        # Create execution string
        exec_str = os.path.join(self.nbis, "nfiq") + " " + fname
        os.chdir(self.cwd)
        return check_output(exec_str, shell=True).decode(stdout.encoding).strip()

    def _convert(self, fname):
        img = Image.open(fname)
        # Convert to grayscale if not already
        img = img.convert("L")
        fname = fname.split(".")[0]+".wsq"
        img.save(fname)
        return fname

    def _get_wh(self, fname):
        """Returns the width and height of a .wsq file. """
        exec_str = os.path.join(self.nbis, "nfiq") + " " + fname
        x = check_output(exec_str,shell=True).decode(stdout.encoding).split(' ')
        w="-1"
        h="-1"
        for each in x:
            if "w=" in each:
                w=each.replace("w=",'').strip()
            elif "h=" in each:
                h=each.replace("h=",'').strip()
        return w,h

    def convert_to_wsq(self, fp):
        """Takes a list of fingerprint image filenames and converts them to
            wsq format.

            Returns a list of files in wsq format.
        """
        wsq_files = []
        for each in fp:
            # Compress file
            # Make sure not a 4 slap
            if('FOUR' not in each):
                wsq_files.append(self._convert(each))
            else:
                # Four finger slap needs nfseg
                # Outputs directly to wsq converted file
                wsq_files.extend(self.segment(each))    
        return wsq_files

    def get_ratings(self, fp):
        """Takes a list of fingerprint images (.wsq format) and runs the
            NIST NFIQ package to evaluate them. Returns an array of dicts
            for each fingerprint file.
        """
        d = []
        for each in fp:
            name = os.path.basename(each)
            score = self._get_iq(each)
            if int(score) < 3:
                print("WARNING: Fingerprint {}".format(name) + " Received A Low Score.")
                print("Consider replacing the print card.")
            w,h = self._get_wh(each)
            d.append({
                'name':name,
                'file':os.path.abspath(each),
                'score':score,
                'hll':w,
                'vll':h,
            })
        return d

    
    def ScanCard(self):
        """Scans an FD258 fingerprint card into memory."""
        # Load main image
        print("Loading Image")
        img = self.load_img(self.ifile)
        # Align image to template
        print("Aligning Image To Template...")
        aligned = self.align_img(img)
        # Section Fingerprints
        print("Sectioning Fingerprints...")
        fp_sections = self.get_fp(aligned)
        # Convert fingerprints to wsq
        print("Compressing Image Files...")
        fp_converted = self.convert_to_wsq(fp_sections)
        # Get Ratings
        print("Getting Fingerprint Scores...")
        return self.get_ratings(fp_converted)

    def GenEFT(self):
        # Scan card into memory
        print("Beginning Card Scan...")
        card = self.ScanCard()
        print("Card scan complete.")
        # Create Type 1 Record
        print("Creating Type-1 Record.")
        t1 = Type1()
        # Create Type 2 record
        print("Creating Type-2 Record.")
        t2 = Type2()
        # Get user data
        t2.get_user_input()
        # Create Type14 records
        for each in card:
            t14 = Type14()
            t14.fcd = t14.dat # Set date to today
            t14.file = each['file']
            t14.hll = each['hll']
            t14.vll = each['vll']
            # Complete record
            t14.build()
            # Add record to Type1
            t1.add_record(t14)
        print("Writing EFT to file.")
        t1.write_to_file(self.ofile)
