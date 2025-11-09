import os
import cv2
import uuid
import numpy as np
from django.conf import settings
from conversion.core.align import GetEFT, four_point_transform
from conversion.core.fd258_ocr import OCR_LOCATIONS
from conversion.core.fingerprint import Fingerprint
from conversion.core.eft_helper import Type1, Type2, Type14
CWD = os.getcwd()
RESULTS = []
TMP_DIR = settings.TMP_DIR
THREAD=None

def format_date(datestring):
    year, month, day = datestring.split('-')
    return str(year)+str(month)+str(day)

def generate_eft(data):
    global RESULTS
    global THREAD
    t1 = Type1()
    t2 = Type2(0)
    # Pull data from form
    print(data)
    t2.fname = data.get("fname")
    t2.mname = data.get("mname")
    t2.lname = data.get("lname")
    t2.aka = data.get("alias")
    t2.addr = data.get("addr")
    t2.ssn = data.get("ssn")
    t2.pob = data.get("pob")
    t2.ctz = data.get("ctz")
    t2.dob = format_date(data.get("dob"))
    t2.dfp = format_date(data.get("dof"))
    t2.sex = data.get("sex")
    t2.race = data.get("race")
    t2.height = data.get("height")
    t2.weight = data.get("weight")
    t2.eye = data.get("eye")
    t2.hair = data.get("hair")
    t2.rsn = data.get("rsn")
    t2.amp = data.get("missing")
    t2.name = "{}, {} {}".format(t2.lname, t2.fname, t2.mname[0])
    # Attach
    t1.add_record(t2)
    # Generate tx number
    n = "{}-{}-{}-".format(t2.fname, t2.mname, t2.lname) + str(uuid.uuid1())[0:10]
    t1.set_tcn(n)
    # Get a file name
    fname = n+'.eft'
    # Convert the fingerprints
    os.chdir(TMP_DIR)
    for each in RESULTS:
        each.convert()
    os.chdir(CWD)
    # Generate type 14 records
    i = 1 # Create idc char, starts at 1
    for fp in RESULTS:
        t14 = Type14(fp, i)
        t14.fcd = t2.dfp
        t14.build()
        t1.add_record(t14)
        i+=1
    # Generate file
    t1.write_to_file(os.path.join(TMP_DIR, fname))
    # Clear results so it can be reused
    RESULTS=[]
    return fname


def process_fp():
    os.chdir(TMP_DIR)
    for each in RESULTS:
        each.convert()

def section_fp(fname):
    global RESULTS
    # Read image
    img = cv2.imread(fname)
    # Align image
    aligned = GetEFT(img)
    return _section(aligned)

def _section(img):
    t = os.path.join(os.getcwd(),'static')
    template = cv2.imread(os.path.join(t,'fd-258.png'))
    img = cv2.resize(img, (template.shape[0], template.shape[1]))
    # Section fingerprints
    out = []
    img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    for loc in OCR_LOCATIONS:
        out.append(loc.id + '.png')
        f = Fingerprint(loc, src_img=img, tmpdir=TMP_DIR)
        RESULTS.append(f)
    return out

def manual_section(fname, data):
    img = cv2.imread(fname)
    p1 = np.array([np.float32(float(x)) for x in data.get("p1").split(',')])
    p2 = np.array([np.float32(float(x)) for x in data.get("p2").split(',')])
    p3 = np.array([np.float32(float(x)) for x in data.get("p3").split(',')])
    p4 = np.array([np.float32(float(x)) for x in data.get("p4").split(',')])
    points = np.zeros((4,2), np.float32)
    points[0] = p1
    points[1] = p2
    points[2] = p3
    points[3] = p4
    aligned = four_point_transform(img, points)
    return _section(aligned)