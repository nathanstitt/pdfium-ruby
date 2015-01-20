require_relative 'spec_helper'
require 'tempfile'

describe PDFium do

  it "creates PDF class" do
    assert PDFium::Pdf
    pdf = PDFium::Pdf.open("test.pdf")
    refute_nil pdf
    #refute pdf.valid?
  end

end
