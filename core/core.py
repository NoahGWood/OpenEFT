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
from core.fingerprint import Fingerprint

class Core:
    def __init__(self, ifile, out=None, debug=False, template="templates/fd-258.png"):
        self.cwd = os.getcwd()
        self.ifile = ifile
        if out:
            self.ofile=out
        else:
            self.ofile=str(uuid.uuid1())[0:10]+'.eft'
        self.dbg=debug
        self.tmpDir = tempfile.TemporaryDirectory()
        self.template = self.load_img(template)
    
    def __del__(self):
        # Force clean up tempdir
        self.tmpDir.cleanup()
    def load_img(self, fname):
        """Loads an image, verifies it was loaded, and returns the image."""
        self.img = cv2.imread(fname)
        self.check_img(self.img, fname)
        return self.img
    
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

        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
        for loc in OCR_LOCATIONS:
            f = Fingerprint(loc, src_img=img, tmpdir=self.tmpDir)
            results.append(f)
            if self.dbg:
                # Show the fingerprint that was sectioned
                cv2.imshow(loc.id, resize(f.img, width=500,height=500))
        if self.dbg:
            cv2.waitKey(0)
            cv2.destroyAllWindows()
        return results
    
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
        fingerprints = self.get_fp(aligned)
        # Convert to JP2
        for each in fingerprints:
            each.convert()
        return fingerprints

    def GenEFT(self):
        try:
            # Scan card into memory
            print("Beginning Card Scan...")
            fingerprints = self.ScanCard()
            print("Card scan complete.")
            # Create Type 1 Record
            print("Creating Type-1 Record.")
            t1 = Type1()
            # Create Type 2 record
            print("Creating Type-2 Record.")
            t2 = Type2(2) # Make second record
            # Get user data
            t2.get_user_input()
            t1.add_record(t2)
            t1.set_tcn(0)
            # Create Type14 records
            i=3 # create idc char, starts at 3 (type1, type2, type14...)
            for each in fingerprints:
                t14 = Type14(each,i)
                t14.fcd = t14.dat # Set date to today
                # Complete record
                t14.build()
                # Add record to Type1
                t1.add_record(t14)
                i+=1 # Increment idc number
            print("Writing EFT to file.")
            t1.write_to_file(self.ofile)
        except:
            if self.dbg:
                cv2.imshow("DEBUG", resize(self.img, width=500,height=500))
                cv2.waitKey(0)
                cv2.destroyAllWindows()