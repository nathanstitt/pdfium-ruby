# The Ruby gem currently supports all features discussed

The Debian package currently copies:

 * Header files from fpdfsdk/include to /usr/include/pdfium/fpdfsdk
 * All of the static libraries from out/Release/*.a  to /usr/lib/pdfium
 * The test application from out/Release/pdfium_test to /usr/bin/pdfium-test

To build the deb run:

    mkdir pdfium-deb
    cd pdfium-deb
    git clone https://pdfium.googlesource.com/pdfium.git pdfium-0.1+git20150128
    cd git pdfium-0.1+git20150128

    svn co http://gyp.googlecode.com/svn/trunk build/gyp
    svn co http://v8.googlecode.com/svn/trunk v8
    svn co https://src.chromium.org/chrome/trunk/deps/third_party/icu46 v8/third_party/icu

    cd ..
    tar czf pdfium-0.1+git20150128.gz pdfium-0.1+git20150128

    dh_make -c BSD -e nathan@stitt.org -f ../pdfium-0.1+git20150128.gz
    (select library type)

    cp <debian-config>/rules ./debian/
    cp <debian-config>/control ./debian/
    cp <debian-config>/libpdfium-dev.install ./debian/
    cp -r <debian-config>/patches ./debian/
    cp <debian-config>/changelog ./debian/
    (edit changelog if needed)

    dpkg-buildpackage
    (if lacking gpg keys, append "-us -uc" flags to dpkg-buildpackage)

To upload:

    dput  ppa:nathan-stitt/pdfium pdfium_0.1+git20150128-1_amd64.changes

To bump version:

    debchange -i
    dh_make --createorig
    dpkg-buildpackage -us -uc
