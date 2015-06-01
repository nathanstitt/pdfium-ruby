require "mkmf"
require 'rbconfig'

def existing(dirs)
  dirs.select{|dir| Dir.exist?(dir) }
end


if ENV['PDFIUM']
    LIB_DIRS    = [ "#{ENV['PDFIUM']}/out/Debug/lib.target" ]
    HEADER_DIRS = [
      "#{ENV['PDFIUM']}/public",
      "#{ENV['PDFIUM']}/core/include",
      "#{ENV['PDFIUM']}"
    ]

else
    LIB_DIRS = [
        "/usr/local/lib/pdfium",
        "/usr/lib/pdfium"
    ]

    HEADER_DIRS = [
        "/usr/include/pdfium",
        "/usr/include/pdfium/core/include",
        "/usr/local/include/pdfium",
        "/usr/local/include/pdfium/core/include"
    ]
end

have_library('pthread')

DEBUG = ENV['DEBUG']

$CPPFLAGS += " -Wall "
$CPPFLAGS += " -g" if DEBUG

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
