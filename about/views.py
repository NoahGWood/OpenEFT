from django.shortcuts import render

# Create your views here.

def about(request):
    return render(request, "about/about.html")

def features(request):
    return render(request, "about/features.html")

def donate(request):
    return render(request, "about/donate.html")

