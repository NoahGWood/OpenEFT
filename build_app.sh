
mkdir -R build/OpenEFT.OpenEFT/usr/bin

cp -R about build/OpenEFT.OpenEFT/about
cp -R browser build/OpenEFT.OpenEFT/browser
cp -R conversion build/OpenEFT.OpenEFT/conversion
cp -R about build/OpenEFT.OpenEFT/about
cp -R OpenEFT build/OpenEFT.OpenEFT/OpenEFT
cp -R viewer build/OpenEFT.OpenEFT/viewer
cp -R static build/OpenEFT.OpenEFT/static
cp -R templates build/OpenEFT.OpenEFT/templates
cp openeft.py build/OpenEFT.OpenEFT/usr/bin/openeft
cp -R build/bin  build/OpenEFT.OpenEFT/usr/bin/
cp appimagetool-x86_64.AppImage build/appimagetool.AppImage

python-appimage build app ./build