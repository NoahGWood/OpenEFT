"""
WSGI config for OpenEFT project.

It exposes the WSGI callable as a module-level variable named ``application``.

For more information on this file, see
https://docs.djangoproject.com/en/4.0/howto/deployment/wsgi/
"""

import os
import tempfile
x = tempfile.TemporaryDirectory()
os.environ.setdefault('TMP_DIR', x.name)

from django.core.wsgi import get_wsgi_application
os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'OpenEFT.settings')
application = get_wsgi_application()

