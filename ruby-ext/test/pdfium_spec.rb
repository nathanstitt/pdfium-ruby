require_relative 'spec_helper'
require 'tempfile'

# def memory
#   `ps -o vsz -p #{$$}`.strip.split.last.to_i
# end
# GC.start(full_mark: true, immediate_sweep: true)

describe PDFium do

  it "creates PDF class" do
    assert PDFium::Pdf
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


  it "warns un unsupported" do
    pdf = PDFium::Pdf.new( pdf_path("AGreatDayForFreedom_LITE") )

  end


end
