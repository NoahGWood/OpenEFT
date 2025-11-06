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
    
if __name__ == '__main__':
    main(["openeft.py", "runserver", "0.0.0.0:8080","--noreload"])
    x.cleanup()
