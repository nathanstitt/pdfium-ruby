require "mkmf"
require 'rbconfig'

LIB_DIRS = [
  "/usr/local/lib/pdfium"
]
HEADER_DIRS = [
  "/usr/local/include/pdfium/fpdfsdk/include"
]

LIBS=%w{pdfium bigint freetype fpdfdoc fpdftext formfiller
javascript v8_base v8_libbase v8_snapshot v8_libplatform jsapi
icui18n icuuc icudata pdfwindow fxedit fxcodec fxcrt fpdfdoc fpdfapi fdrm
fxge freetype pthread }

dir_config("libs", HEADER_DIRS, LIB_DIRS)

LIBS.each do | lib |
  have_library(lib) or abort "Didn't find library lib#{lib}"
end

if `uname`.chomp == 'Darwin' then

  FRAMEWORKS = %w{AppKit CoreFoundation}

  $LDFLAGS << FRAMEWORKS.map { |f| " -framework #{f}" }.join
end


create_makefile "pdfium_ext"
