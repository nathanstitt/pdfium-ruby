# OSX
INCLUDES=/usr/local/include/pdfium/fpdfsdk/include
LIBS=/usr/local/lib/pdfium/*.a -framework AppKit -framework CoreFoundation

# Linux (Debian/Ubuntu)
INCLUDES=/usr/lib/pdfium
LIBS=/usr/lib/pdfium/*.a

clean:
	rm -rf *o pdfium

# -L/usr/lib/pdfium -I/usr/include/pdfium -I/usr/include/pdfium/fpdfsdk  /usr/lib/pdfium/*.a

pdfium: pdfium.cc
	g++ -I$(INCLUDES) $(LIBS) pdfium.cc -o pdfium
