#!/usr/bin/env ruby

require_relative '../lib/pdfium'
require 'pathname'

puts "Waiting for profiler attachment (PID: #{Process.pid})\nPress enter to continue"
gets

path = Pathname.new(__FILE__).dirname.join('pdfs','example_images.pdf').to_s

guide = PDFium::Document.new( path )
`rm /tmp/images/*`
page = guide.page_at(0)
page.each_image do |img|
    img.save("/tmp/images/#{img.index}.png")
end

# count = 0
# pdf.bookmarks.each do | bm |
#     count +=1
#     print count.to_s + "   "
#     puts bm.title
# end

# pdf.each_page do | page |
#     page.width
# end

# GC.start(full_mark: true, immediate_sweep: true)
