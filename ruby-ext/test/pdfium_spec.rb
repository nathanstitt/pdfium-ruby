require_relative 'spec_helper'
require 'tempfile'

# def memory
#   `ps -o vsz -p #{$$}`.strip.split.last.to_i
# end
# GC.start(full_mark: true, immediate_sweep: true)

describe PDFium do

  it "creates classes" do
    assert PDFium::Pdf
    assert PDFium::Page
  end

  it "throws exception on invalid pdf" do
    assert_raises(RuntimeError) do
      PDFium::Pdf.new( pdf_path("invalid") )
    end

  end

  it "counts pages" do
    {
      "S2"        => 1,
      "basicapi"  => 3,
      "SDK_Guide" => 90,
      "SlowestProcessingDoc" => 2390
    }.each do | name, pgs |
      pdf = PDFium::Pdf.new( pdf_path(name) )
      assert_equal pgs, pdf.page_count, "#{name} page count is incorrect"
      pdf = nil
    end
  end


  it "reads page dimensions" do
    pdf = PDFium::Pdf.new( pdf_path("basicapi") );
    page = PDFium::Page.new(pdf,0)
    height, width = page.dimensions
    assert_in_delta 841.89, height
    assert_in_delta 595.28, width

  end

  it "saves a page" do
    pdf = PDFium::Pdf.new( pdf_path("OoPdfFormExample") );
    page = PDFium::Page.new(pdf,0)
    formats = %w{gif png jpeg bmp}
    formats.each do | format |
      tf="/tmp/pdf-page-test.#{format}"
      assert page.render(tf, 1500, 2300), "failed to render #{format}"
      type = `identify #{tf}`
      assert_match( format, type)
      assert_match( "1500x2300", type)
      #`open #{tf}`

    end
  end

  it "renders a large pdf" do
    pdf = PDFium::Pdf.new( pdf_path("basicapi") ) #SlowestProcessingDoc") );
    0.upto(pdf.page_count-1) do | pg_num |
      page = PDFium::Page.new(pdf,pg_num)
      tf = Tempfile.new(["pdf-page-test",".gif"])
      assert page.render(tf.path, 150, 230), "failed to render #{pg_num}"
      page = nil
      GC.start(full_mark: true, immediate_sweep: true)
    end

  end


end
