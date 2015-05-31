require_relative 'spec_helper'

describe PDFium::PageList do

    let(:guide){ load_document("with_bookmarks") }

    it "can be empty" do
        pdf = PDFium::Document.new
        pages = pdf.pages
        assert_kind_of PDFium::PageList, pages
        assert pages.none?, "A freshly created Document shouldn't have any pages"
    end

    it "can iterate" do
        count = 0
        guide.pages.each{ count += 1 }
        assert_equal 3, count
    end

    it "supports access by index" do
        assert_equal 1, guide.pages[0].number
        assert_equal 2, guide.pages[1].number
    end
end
