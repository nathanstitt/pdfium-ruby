require_relative 'spec_helper'

describe PDFium::ImageList do

    let(:image_doc){  load_document("example_images").page_at(0) }
    let(:blank_page){ load_document("example_utf8").page_at(0)   }

    it "can be empty" do
        assert blank_page.images.none?, "images found where there should not be"
    end

    it "can iterate" do
        count = 0
        image_doc.images.each{|i| count+=1 }
        assert_equal 26, count
    end

end
