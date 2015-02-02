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
      tf="/tmp/pdfium-page-test.#{format}"
      assert page.render(tf, 1500, 2300), "failed to render #{format}"
      type = `identify #{tf}`
      assert_match( format, type)
      assert_match( "1500x2300", type)
    end
  end

  it "renders a large pdf" do
    pdf = PDFium::Document.new( pdf_path("basicapi") ) #SlowestProcessingDoc") );
    0.upto(pdf.page_count-1) do | pg_num |
      page = PDFium::Page.new(pdf,pg_num)
      tf = Tempfile.new(["pdfium-page-test",".gif"])
      assert page.render(tf.path, 150, 230), "failed to render #{pg_num}"
      page = nil
      GC.start(full_mark: true, immediate_sweep: true)
    end
  end

  it "renders a pdf to multiple sizes" do
    pdf = PDFium::Document.new( pdf_path("basicapi") )
    page = PDFium::Page.new(pdf,0)
    sizes = [
      [400,550], [300,200], [100,120]
    ]
    assert page.render_sizes("/tmp/pdfium-page-test-%w-%h.png", sizes ), "failed to render multiple sizes"
    sizes.each do | size |
      assert_match("#{size.first}x#{size.last}",
        `identify /tmp/pdfium-page-test-#{size.first}-#{size.last}.png`)
    end
  end

  it "rendering multiple sizes tolerates incorrect args" do
    pdf = PDFium::Document.new( pdf_path("basicapi") )
    page = PDFium::Page.new(pdf,0)
    # 'a' width?
    assert_raises(TypeError) do
      page.render_sizes("/tmp/pdfium-page-test-%w-%h.png", [ ['a',100] ] )
    end
    # no placeholders
    refute page.render_sizes("/tmp/pdfium-page-test.png", [[100,22]] ), "rendered without placeholders"
    # not an array passed
    refute page.render_sizes("/tmp/pdfium-page-test-%w-%h.png", { not_an_array: true } )
    # an array but with incorrect inner arrays
    refute page.render_sizes("/tmp/pdfium-page-test-%w-%h.png", [{ not_an_array: true }] )
    refute page.render_sizes("/tmp/pdfium-page-test-%w-%h.png", [[3]] )
  end

  it "scales the image if height/width is not given" do
    pdf = PDFium::Document.new( pdf_path("basicapi") )
    page = PDFium::Page.new(pdf,0)
    tf = Tempfile.new(["pdfium-page-test",".gif"])
    refute page.render(tf.path, 0, 0), "rendered even though height/width where both 0"

    assert page.render(tf.path, 0, 230)
    assert_match("162x230",`identify #{tf.path}`)

    assert page.render(tf.path, 85, 0)
    assert_match("85x120",`identify #{tf.path}`)

    tf = Tempfile.new(["pdfium-page-test-%h-%w",".gif"])
    refute page.render_sizes(tf.path, [[0, 0]]), "rendered multiple even though height/width where both 0"
    sizes = [
      [0,550], [300,0]
    ]
    assert page.render_sizes("/tmp/pdfium-page-test-%w-%h.png", sizes ), "failed to render multiple sizes with missing elements"
    assert_match("388x550",`identify /tmp/pdfium-page-test-388-550.png`)
    assert_match("300x424",`identify /tmp/pdfium-page-test-300-424.png`)
  end



end
