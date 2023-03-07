import os
import cv2
from core.fd258_ocr import OCR_LOCATIONS

class Fingerprint:
    """
        self.tmpdir # The temporary directory files are stored in
        self.name # The file name (without encoding)
        self.encoding # The base encoding
        self.converted # The compressed fingerprint file
        self.src # The CV2 image used to extract fingerprint
        self.img # The CV2 image of the extracted fingerprint
        self.ppi # Pixel density
        self.hll # HORIZONTAL LINE LENGTH
        self.vll # VERTICAL LINE LENGTH
        self.hps # HORIZONTAL PIXEL SCALE
        self.vps # VERTICAL PIXEL SCALE
        self.slc # SCALE UNITS
        self.cga # COMPRESSION ALGORITHM (JP2 default)
        self.bpx # BITS PER PIXEL
        self.fgp # Fingerprint position
    """
    def __init__(self, loc, src_img, tmpdir):
        self.tmpdir = tmpdir
        self.name = loc.id
        self.fgp = loc.fp_number
        self.encoding = 'png'
        self.converted = ""
        self.src = src_img
        self.extract_fp(loc.bbox) # Sets self.img 
        self.ppi = self.get_ppi()
        self.get_settings()

    def get_settings(self):
        self.hll = self.img.shape[0]# HORIZONTAL LINE LENGTH
        self.vll = self.img.shape[1] # VERTICAL LINE LENGTH
        self.slc = "1" # SCALE UNITS
        self.bpx = 8 # BITS PER PIXEL, should be 8

    def get_ppi(self):
        # Image should be 8x8"
        x = self.src.shape[0] / 8
        y = self.src.shape[1] / 8
        self.hps = round(x) # HORIZONTAL PIXEL SCALE
        self.vps = round(y) # VERTICAL PIXEL SCALE
        self.ppi = round((x+y)/2) 

    def extract_fp(self, bbox):
        (x,y,w,h) = bbox
        self.img = self.src[y:y+h, x:x+w]
        self.save_image()

    def save_image(self, encoding=None):
        if encoding == None:
            f = os.path.join(self.tmpdir.name, self.name) + '.' + self.encoding
        else:
            f = os.path.join(self.tmpdir.name, self.name) + '.' + encoding
        cv2.imwrite(f, self.img)

    def convert(self, encoding='jp2', ratio=10):
        i = os.path.join(self.tmpdir.name,self.name)
        o = i + '.' + encoding
        i = i + '.' + self.encoding
        x = ""
        if encoding == 'jp2':
            self.cga = "JP2" # COMPRESSION ALGORITHM [242-HQ-A6687913-SYSDOCU 3.82 value (ASCII)]
            x = "opj_compress -i {} -o {} -r {}".format(i,o, ratio)
        # Add other options later if needed.
        os.system(x)
        self.converted = o