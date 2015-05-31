#!/usr/bin/env ruby

require_relative '../lib/pdfium'
require 'pathname'


path = Pathname.new(__FILE__).dirname.join('pdfs','example_images.pdf')

# guide = PDFium::Document.new( path )
# `rm /tmp/images/*`
# page = guide.page_at(0)


# path.write page.as_image(height: 120).data('jpg')

data = path.read
puts data.length
pdf = PDFium::Document.from_memory(data)

puts pdf.page_count

# page.each_image do |img|
#     img.save("/tmp/images/#{img.index}.png")
# end
