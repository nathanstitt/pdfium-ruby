require_relative 'spec_helper'
require 'tempfile'

def memory
  `ps -o rss -p #{$$}`.strip.split.last.to_i
end

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
    puts "Starting Memory: #{memory}"

    {
      "S2"        => 1,
      "basicapi"  => 3,
      "SDK_Guide" => 90,
      "SlowestProcessingDoc" => 2390
    }.each do | name, pgs |
      pdf = PDFium::Pdf.new( pdf_path(name) )
      assert_equal pgs, pdf.page_count, "#{name} page count is incorrect"
      puts "before GC #{name} -> #{memory}"
      pdf = nil
      GC.start(full_mark: true, immediate_sweep: true)
      puts "after  GC #{name} -> #{memory}"
    end
  end


  it "has stuff" do

  end


end
