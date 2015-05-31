require "mkmf"
require 'rbconfig'

def existing(dirs)
  dirs.select{|dir| Dir.exist?(dir) }
end

LIB_DIRS=[]
# if ENV['PDFIUM']
#     LIB_DIRS    = [ "#{ENV['PDFIUM']}/out/Debug/lib.target" ]
#     HEADER_DIRS = [
#       "#{ENV['PDFIUM']}/fpdfsdk/include",
#       "#{ENV['PDFIUM']}/core/include",
#       "#{ENV['PDFIUM']}"
#     ]

# else
#     LIB_DIRS = [
#       "/usr/local/lib/pdfium",
#       "/usr/lib/pdfium"
#     ]

#     HEADER_DIRS = [
#       "/usr/include/pdfium",
#       "/usr/local/include/pdfium",
#       "/usr/local/include/pdfium/fpdfsdk/include",
#       "/usr/local/include/pdfium/core/include"
#     ]
# end

HEADER_DIRS=[
    "/home/nas/pdfium/deb-package/pdfium/fpdfsdk/include",
    "/home/nas/pdfium/deb-package/pdfium/core/include",
    "/home/nas/pdfium/deb-package/pdfium"
]

have_library('pthread')

DEBUG = ENV['DEBUG'] == '1'

$CPPFLAGS += " -Wall "
$CPPFLAGS += " -g" #if DEBUG

# The order that the libs are listed matters for Linux!
# to debug missing symbols you can run:
#    for l in `ls /usr/lib/pdfium/*.a`; do echo $l; nm $l | grep '<missing symbol>'; done
# The listing with a "T" contains the symbol, the ones with a "U"
# depend on it.  The "U" libs must come after the "T"
# LIBS=%w{
#   javascript bigint     freetype fpdfdoc     fpdftext        formfiller  icudata    icuuc
#   icui18n    v8_libbase v8_base  v8_snapshot v8_libplatform  jsapi       pdfwindow  fxedit
#   fxcrt      fxcodec    fpdfdoc  fdrm fxge   fpdfapi         freetype    pdfium
#   pthread    freeimage
# }
LIBS=%w{pdfium freeimage}

dir_config("libs", existing(HEADER_DIRS), existing(LIB_DIRS))

LIBS.each do | lib |
  have_library(lib) or abort "Didn't find library lib#{lib}"
end

if RUBY_PLATFORM =~ /darwin/
  have_library('objc')
  FRAMEWORKS = %w{AppKit CoreFoundation}
  $LDFLAGS << FRAMEWORKS.map { |f| " -framework #{f}" }.join
else
  $CPPFLAGS += " -fPIC"
end

$CPPFLAGS += " -std=c++11"
$defs.push "-DDEBUG=1" if DEBUG

create_header

create_makefile "pdfium_ext"
