#!/usr/bin/env ruby

require 'rusage'
require 'docsplit'


IMAGE_SIZES = {
  large:      '1000x',
  normal:     '700x',
  small:      '180x',
  thumbnail:  '60x75!'
}

output_directory = Dir.mktmpdir

start_time = Time.now
Dir.glob("test-pdfs/*.pdf").each do |pdf|
  Docsplit.extract_images(pdf, :format => :gif, :size => IMAGE_SIZES.values, :rolling => true, :output => output_directory)
end
elapsed_time = Time.now-start_time

# Process.crusage measures children
# Process.rusage measures self
usage = Process.crusage

rss = usage.maxrss.to_f
# OSX reports in terms of bytes, bsd & linux use kb. "man getrusage"
rss = rss/1024 if RUBY_PLATFORM =~ /darwin/

du = Dir.glob("#{output_directory}/*/**").inject(0.0){|x,img| x+File.stat(img).size }
FileUtils.rm_r output_directory

def report(label,value,specifier="")
  printf("%12s: %8.3f %s\n", label, value, specifier)
end

report "Elapsed",    elapsed_time, "Seconds"
report "System CPU", usage.stime
report "User CPU",   usage.utime
report "Max Memory", (rss/1024),"MB"
report "Disk Space", (du/1024/1024), "MB"
