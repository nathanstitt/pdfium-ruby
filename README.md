# The Ruby gem currently supports all features discussed

The Debian package currently copies:

  * Header files from fpdfsdk/include to /usr/include/pdfium
  * Header files from v8/include/* to /usr/include/pdfium/v8
  * All of the static libraries from out/Release/*.a  to /usr/lib/pdfium
  * The test application from out/Release/pdfium_test to /usr/bin/pdfium_test

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


Results of a single pass from stress-test script:

```
Pass PDF                                                             Pgs  Memory  PDFium      GM      Diff      PDFium      GM      Diff
  1 11-17-14_mr14097_fwd_id1309293-000-pdf.pdf                        3   39,908    1.57    2.84    81.09%        0.65    1.68   160.04%
  1 12-16-14_mr14213_fwd-pdf.pdf                                      1   40,152    0.28    0.49    75.00%        0.22    0.51   134.21%
  1 12063390309-1223-121651-058.pdf                                   5   51,372    1.09    2.20   102.16%        0.90    2.72   201.32%
  1 a3433-transfer-agreement-graceville.pdf                           9   52,248    4.39    7.60    73.13%        1.66    6.59   296.61%
  1 ack-ltr-fa-15-0020.pdf                                            3   53,120    0.63    0.93    46.91%        0.59    2.98   403.06%
  1 acknowledgement-letter.pdf                                        1   64,060    0.26    0.35    36.36%        0.28    1.08   287.07%
  1 AGreatDayForFreedom_LITE.pdf                                      6   59,084    4.51    6.50    44.19%        0.85    9.80  1055.07%
  1 alphatrans.pdf                                                    1   47,004    0.23    0.38    68.97%        0.27    1.57   472.80%
  1 basicapi.pdf                                                      3   47,764    0.15    0.17    15.79%        0.57    2.12   273.39%
  1 beckwith-brown-release.pdf                                        1   44,880    0.38    0.39     2.04%        0.20    1.32   565.82%
  1 boston-invite.pdf                                                 2   47,640    0.70    0.82    17.98%        0.38    2.40   527.90%
  1 briggs-information-pdf.pdf                                        8   74,308    1.19    1.95    63.82%        1.94    5.27   171.09%
  1 brownlee-indictment-001.pdf                                       4   49,852    1.16    1.80    54.36%        0.64    2.28   257.09%
  1 dph-11-005fsor.pdf                                               46   52,440   31.35   39.49    25.97%        7.46   56.61   658.41%
  1 ellerbe-indictment.pdf                                            7   50,056    1.67    2.49    49.07%        1.13    3.88   242.17%
  1 fhfa-inspectors-docs.pdf                                         34   82,348   14.23   17.13    20.36%        5.66   52.73   831.95%
  1 july-26-1979-austin-city-council-regular-meeting.pdf             55  102,004   23.88   44.16    84.92%       11.29   43.00   281.00%
  1 march-24th-2014.pdf                                              76  456,324   86.62  114.51    32.19%       22.16  436.18  1868.01%
  1 musgrave14-02390_5_21_2014_9_8_44-pdf.pdf                         2  198,464    0.68    0.95    39.08%        0.42    1.92   357.06%
  1 OoPdfFormExample.pdf                                              1  202,132    0.30    0.42    42.11%        0.18    1.16   534.24%
  1 proposal-samrout-062514.pdf                                       3  199,588    0.42    0.73    72.22%        0.54    1.53   181.67%
  1 release_ltr1_1_1-pdf.pdf                                          1  195,776    0.36    0.41    15.22%        0.19    1.09   484.18%
  1 S2.pdf                                                            1  200,144    1.25    1.54    23.12%        0.30    2.53   732.79%
  1 SampleSignedPDFDocument.pdf                                       1  190,116    0.05    0.19   300.00%        0.16    1.05   564.56%
  1 sayre-ok-public-records-request-pdf.pdf                           1  190,272    0.33    0.48    45.24%        0.18    1.67   834.53%
  1 SDK_Guide.pdf                                                    90  208,220   28.33   25.91    -8.55%       13.44  105.01   681.50%
  1 sign-a-pdf-with-reader-enabled.pdf                                1  205,864    0.03    0.06   100.00%        0.19    1.05   448.95%
  1 state-department-cat-memo.pdf                                    45  238,800   22.62   42.62    88.43%       11.02   33.09   200.19%
  1 sunnyside-community-mitigation-strategy-april-2.pdf              20  237,592   10.09   12.93    28.20%        2.32   17.99   676.17%
  1 updated-proposal-kozlowski-071314.pdf                             3  193,868    1.34    2.47    83.72%        0.60    1.68   178.51%
  1 wi081313lakesidefoodspt2.pdf                                      7  195,584    3.23    5.96    84.75%        1.39    3.97   185.90%

```
