#!/usr/bin/env python3
"""OpenEFT"""
import os
import tempfile
import threading
from subprocess import check_output

def main(a):
    """Run administrative tasks."""
    x = tempfile.TemporaryDirectory()
    os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'OpenEFT.settings')
    os.environ.setdefault('TMP_DIR', x.name)
    try:
        from django.core.management import execute_from_command_line
    except ImportError as exc:
        raise ImportError(
            "Couldn't import Django. Are you sure it's installed and "
            "available on your PYTHONPATH environment variable? Did you "
            "forget to activate a virtual environment?"
        ) from exc
    execute_from_command_line(a)
    
def browser():
    print("RUNNING BROWSER")
    if 'nt' in os.name:
        if check_output("./browser/windows/chrome.exe --app=http://localhost:7100", shell=True) is not None:
            os._exit(1)
    elif 'posix' in os.name or 'linux' in os.name:
        if check_output("./browser/linux/chrome.AppImage --app=http://localhost:7100/", shell=True) is not None:
            os._exit(1)
    # Force exit if not already
    os._exit(1)
    
if __name__ == '__main__':
    br = threading.Thread(target=browser, args=())
    br.start()
    main(["openeft.py", "runserver", "7100","--noreload"])
