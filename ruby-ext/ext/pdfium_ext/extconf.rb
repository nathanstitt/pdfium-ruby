require "mkmf"
require 'rbconfig'

LIB_DIRS = [
  "/usr/local/lib/pdfium",
  "/usr/lib/pdfium"
]
HEADER_DIRS = [
  "/usr/include/pdfium",
  "/usr/local/include/pdfium",
  "/usr/local/include/pdfium/fpdfsdk/include"
]
have_library('pthread')
have_library('objc') if RUBY_PLATFORM =~ /darwin/
$CPPFLAGS += " -Wall" unless $CPPFLAGS.split.include? "-Wall"
$CPPFLAGS += " -g" unless $CPPFLAGS.split.include? "-g"
$CPPFLAGS += " -rdynamic" unless $CPPFLAGS.split.include? "-rdynamic"
$CPPFLAGS += " -fPIC" unless $CPPFLAGS.split.include? "-rdynamic" or RUBY_PLATFORM =~ /darwin/

# The order that the libs are listed matters for Linux!
# to debug missing symbols you can run:
#    for l in `ls /usr/lib/pdfium/*.a`; do echo $l; nm $l | grep '<missing symbol>'; done
# The listing with a "T" contains the symbol, the ones with a "U"
# depend on it.  The "U" libs must come after the "T"
LIBS=%w{javascript bigint freetype fpdfdoc fpdftext formfiller
icudata icuuc icui18n v8_libbase v8_base v8_snapshot v8_libplatform  jsapi
pdfwindow fxedit fxcrt fxcodec fpdfdoc  fdrm fxge fpdfapi
freetype pdfium pthread freeimage}

dir_config("libs", HEADER_DIRS, LIB_DIRS)

LIBS.each do | lib |
  have_library(lib) or abort "Didn't find library lib#{lib}"
end

if `uname`.chomp == 'Darwin' then

  FRAMEWORKS = %w{AppKit CoreFoundation}

  $LDFLAGS << FRAMEWORKS.map { |f| " -framework #{f}" }.join
end

create_makefile "pdfium_ext"
