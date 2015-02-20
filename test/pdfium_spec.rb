require_relative 'spec_helper'
require 'tempfile'

# def memory
#   `ps -o vsz -p #{$$}`.strip.split.last.to_i
# end
# GC.start(full_mark: true, immediate_sweep: true)
# puts "Press enter to continue"
# gets

describe PDFium do

  it "creates classes" do
    assert PDFium::Document
    assert PDFium::Page
  end

  it "document creates pages" do
    pdf = PDFium::Document.new( pdf_path("basicapi") )
    page = pdf.page_at(0)
    assert page
    assert_raises(ArgumentError) do
      pdf.page_at(1234567)
    end
  end

  it "iterates over the pages" do
    called = 0
    pdf = PDFium::Document.new( pdf_path("basicapi") )
    pdf.each_page do | page |
      assert page.is_a?(PDFium::Page)
      assert_equal called, page.number
      called += 1
    end
    assert_equal 3, called
  end

  it "throws exception on invalid pdf" do
    assert_raises(ArgumentError) do
      PDFium::Document.new( pdf_path("invalid") )
    end
  end


  it "counts pages" do
    {
      "S2"        => 1,
      "basicapi"  => 3,
      "SDK_Guide" => 90
    }.each do | name, pgs |
      pdf = PDFium::Document.new( pdf_path(name) )
      assert_equal pgs, pdf.page_count, "#{name} page count is incorrect"
      pdf = nil
    end
  end


  it "reads page dimensions" do
    pdf = PDFium::Document.new( pdf_path("basicapi") );
    page = PDFium::Page.new(pdf,0)

    assert_in_delta 841.89, page.height
    assert_in_delta 595.28, page.width
  end

  it "saves a page" do
    pdf = PDFium::Document.new( pdf_path("OoPdfFormExample") );
    page = PDFium::Page.new(pdf,0)
    formats = %w{gif png jpeg bmp}
    formats.each do | format |
      tf = Tempfile.new(["pdfium-page-test",".#{format}"])
      assert page.render(tf.path, width:1500, height:2300), "failed to render #{format}"
      type = `identify #{tf.path}`
      assert_match( format, type)
      assert_match( "1500x2300", type)
    end
  end

  it "renders a pdf" do
    pdf = PDFium::Document.new( pdf_path("basicapi") )
    page_number = 0
    pdf.each_page do | page |
      assert_equal page_number, page.number
      tf = Tempfile.new(["pdfium-page-test",".gif"])
      assert page.render(tf.path, width:150, height:230), "failed to render #{page.number}"
      page_number += 1
    end
  end

  it "rendering a page tolerates incorrect args" do
    pdf = PDFium::Document.new( pdf_path("basicapi") )
    page = pdf.page_at(0)
    # 'a' width?
    tf = Tempfile.new(["pdfium-page-test",".gif"])
    assert_raises(TypeError) do
      page.render(tf.path, width: 'a' )
    end
    # neither height nor width
    assert_raises(TypeError) do
      refute page.render(tf.path ), "rendered without placeholders"
    end
  end

  it "scales the image if height/width is not given" do
    pdf  = PDFium::Document.new( pdf_path("basicapi") )
    page = pdf.page_at(0)

    tf = Tempfile.new(["pdfium-page-test",".gif"])
    assert_raises(ArgumentError) do
      refute page.render(tf.path, width:0, height: 0), "rendered even though height/width were both 0"
    end
    assert page.render(tf.path, width:0, height:230)
    assert_match("162x230",`identify #{tf.path}`)

    assert page.render(tf.path, width:85, height:0)
    assert_match("85x120",`identify #{tf.path}`)

    assert page.render(tf.path, width:180)
    assert_match("180x254",`identify #{tf.path}`)
  end



end
