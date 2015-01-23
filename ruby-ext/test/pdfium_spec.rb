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


  it "loads pages" do
    pdf = PDFium::Pdf.new( pdf_path("basicapi") );
    page = PDFium::Page.new(pdf,0)
    assert page
  end

  it "reads page dimensions" do
    pdf = PDFium::Pdf.new( pdf_path("basicapi") );
    page = PDFium::Page.new(pdf,0)
    height, width = page.dimensions
    assert_in_delta 841.89, height
    assert_in_delta 595.28, width
  end

  it "saves a page" do
    pdf = PDFium::Pdf.new( pdf_path("basicapi") );
    page = PDFium::Page.new(pdf,0)
    tf=Tempfile.new(['page', '.bmp'])
    assert page.render(tf.path,100,100), "failed to render"
    assert_match( "100 x 100 x 24", `file #{tf.path}` )
  end

  it "warns un unsupported" do
    pdf = PDFium::Pdf.new( pdf_path("AGreatDayForFreedom_LITE") )
    0.upto(pdf.page_count-1) do | pg |
      page = PDFium::Page.new(pdf,pg)
      tf="/tmp/pg-#{pg}.bmp" #Tempfile.new(['page', '.bmp'])
      p tf
      assert page.render(tf,500,800), "failed to render pg #{pg}"
      `open #{tf}`
    end

  end

end
