#!/usr/bin/env python3
"""OpenEFT"""
import os
import tempfile
import threading
import time
from subprocess import check_output
x = tempfile.TemporaryDirectory()
os.environ.setdefault('TMP_DIR', x.name)
print(x)
def main(a):
    """Run administrative tasks."""
    os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'OpenEFT.settings')
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
    time.sleep(3) # Give some time for django to fully load.
    print("RUNNING BROWSER")
    if 'nt' in os.name:
        if check_output("wsl ./browser/windows/chrome.exe --app=http://localhost:8080", shell=True) is not None:
            pass
    elif 'posix' in os.name or 'linux' in os.name:
        if check_output("./browser/linux/chrome.AppImage --app=http://localhost:8080/", shell=True) is not None:
            pass
    # Force exit if not already
    x.cleanup()
    os._exit(1)
    
if __name__ == '__main__':
    br = threading.Thread(target=browser, args=())
    br.start()
    main(["openeft.py", "runserver", "0.0.0.0:8080","--noreload"])
    x.cleanup()
