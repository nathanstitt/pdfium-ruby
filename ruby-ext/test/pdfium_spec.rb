require_relative 'spec_helper'
require 'tempfile'

describe PDFium do

  it "creates PDF class" do
    assert PDFium::Pdf
    pdf = PDFium::Pdf.new("test.pdf")
    assert pdf.test
#    pdf = PDFium::Pdf.open("test.pdf")
#    pdf.test
    #refute pdf.valid?
  end

end
