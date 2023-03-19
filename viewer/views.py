import os
from django.template.defaulttags import register
from django.shortcuts import render
from viewer.records import RECORDS, DESCRIPTORS
from conversion.core.eft_helper import FS_CHAR, GS_CHAR, RS_CHAR, US_CHAR 
from django.conf import settings
CWD = os.getcwd()
TMP_DIR = settings.TMP_DIR

def sanitize(l):
    out = ""
    for c in l:
        if ord(c) not in (FS_CHAR, GS_CHAR, RS_CHAR, US_CHAR):
            out+=str(c)
    return out

def readfile(f):
    out = {}
    with open(f) as f:
        lines = f.readlines()
    print(len(lines))
    last = None
    i = 2
    for line in lines:
        rn = RECORDS[line.split('[')[1].split(']')[0]]
        if rn:
            record = sanitize(line.split('=')[1])
            if rn in out.keys():
                if rn != last:
                    i=2
                out[rn+str(i)]=record
                i+=1
                last = rn
            else:
                out[rn]=record
    return out


@register.filter
def get_item(key):
    k = ""
    append = ""
    # Remove numerals
    for i in range(len(key)-1,-1,-1):
        if not key[i].isnumeric():
            k = key[i] + k
        else:
            append = key[i] + append
    print(key)
    print(k)
    key = k
    v = DESCRIPTORS.get(key)
    v = v.split(' ')
    return ' '.join(x.capitalize() for x in v) + " " + append

def an2ktool(fname):
    os.chdir(TMP_DIR)
    try:
        os.system("an2ktool -print all {} > {}.out".format(fname, fname))
    except Exception as e:
        print("ERROR IN AN2KTOOL")
        print(e)
    os.chdir(CWD)
    return fname + '.out'

# Create your views here.
def viewer(request):
    fname = None
    data = None
    if request.method == "POST":
        file = request.FILES.get("eft")
        fname = os.path.join(TMP_DIR, str(file))
        with open(fname, "wb+") as dest:
            for chunk in file.chunks():
                dest.write(chunk)
        # Scan with an2ktool
        data = readfile(an2ktool(fname))
    return render(request, template_name="viewer/index.html", context={"fname":fname.split('/')[-1], "data":data})