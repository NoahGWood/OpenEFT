import os
import cv2
import time
import json
import threading

from django.http import JsonResponse
from django.shortcuts import render, redirect
from django.conf import settings

from conversion.core.core import generate_eft, section_fp, manual_section

CWD = os.getcwd()
TMP_DIR = settings.TMP_DIR
FILES=[]

# Create your views here.

def index(request):
    return render(request, "conversion/index.html")

def new(request):
    return render(request, "conversion/new.html")

def process_fp():
    os.chdir(TMP_DIR)
    for each in RESULTS:
        each.convert()

def resection(request):
    if request.method == "POST":
        data = request.POST.dict()
        fname = os.path.join(TMP_DIR, 'input.png')
        out = manual_section(fname=fname, data=data)
        return JsonResponse({'values':out}, safe=False)
    return 200

def step1(request):
    global RESULTS
    if request.method == "POST":
        file = request.FILES.get("formFileLg")
        print(file)
        time.sleep(1)
        fname = os.path.join(TMP_DIR, 'input.png')
        with open(fname, 'wb+') as dest:
            for chunk in file.chunks():
                dest.write(chunk)
        try:
            out = section_fp(fname=fname)
        except:
            out = False;#[False,False,False]
        return JsonResponse({'values':out}, safe=False)
    return 200

def step2(request):
    global FILES
    if request.method == "POST":
        print(dir(request))
        print(request.body)
        data = request.POST.dict()
        fname = generate_eft(data)
        FILES.append(fname)
    return render(request, "conversion/download.html", context={'files':FILES})

def download(request):
    return render(request, "conversion/download.html", context={'files':FILES})