# OSX
INCLUDES=/usr/local/include/pdfium/fpdfsdk/include
LIBS=/usr/local/lib/pdfium/*.a -framework AppKit -framework CoreFoundation

# Linux (Debian/Ubuntu)
INCLUDES=/usr/include/pdfium
# Note that the libs are all specified here,
# order is important with GCC, but not Clang (appearently).
LIBS= -L/usr/lib/pdfium -lpdfium -lbigint -lfreetype -lfpdfdoc -lfpdftext -lformfiller \
-ljavascript -lv8_base -lv8_libbase -lv8_snapshot -lv8_libplatform -ljsapi \
-licui18n -licuuc -licudata -lpdfwindow -lfxedit -lfxcodec -lfxcrt -lfpdfdoc -lfpdfapi -lfdrm \
-lfxge -lfreetype -lpthread


clean:
	rm -rf *o pdfium

pdfium: pdfium.cc
	g++ -I$(INCLUDES) pdfium.cc $(LIBS) -o pdfium
