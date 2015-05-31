require_relative 'spec_helper'

describe PDFium::Document do
    let(:guide){ load_document("example_images") }

    it "can create a new empty pdf" do
        pdf = PDFium::Document.new
        assert pdf
    end

    it "can be initialized from string" do
        data = pdf_path("with_bookmarks").read
        pdf = PDFium::Document.from_memory(data)
        assert pdf
        assert_equal 3, pdf.page_count
    end

    it "counts pdf pages" do
        assert_equal 3, guide.page_count
    end

    it "can save to a file" do
        pdf = PDFium::Document.new
        PDFium::Page.create(pdf,0)
        Tempfile.open(['test','.pdf']) do |f|
            pdf.save(f.path)
            reloaded = PDFium::Document.new(f.path)
            assert_equal 1, reloaded.page_count
        end
    end

    it "returns pages" do
        assert guide
        assert_kind_of PDFium::PageList, guide.pages
    end

    it "can read metadata" do
        assert_equal "mPDF 5.1", guide.metadata[:producer].encode!("ASCII-8BIT")
    end

    it "can write metadata" do
        guide.metadata do | md |
            md[:author] = "My Little Writer"
        end
        after_saving(guide) do | saved |
            assert_equal "My Little Writer", saved.metadata[:author].encode!("ASCII-8BIT")
        end
    end
end
