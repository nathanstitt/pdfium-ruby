require_relative 'spec_helper'
require 'image_science'

describe PDFium::Image do
    let(:guide){ load_document("example_images") }
    let(:page) { guide.page_at(0) }

    it "saves as various formats" do
        %w{png jpeg tiff bmp gif}.each do | ext |
            file = Tempfile.new(['test', ".#{ext}"])
            width  = rand(200) + 100
            height = rand(300) + 100
            page.as_image(width: width, height: height).save(file.path)
            assert_size "#{width}x#{height}", file.path
        end
    end


    it "dumps to string" do
        file = Tempfile.new(['test',".jpeg"])
        file.write page.as_image(height: 120).data('jpg')
        file.flush
        assert_size "84x120", file.path
    end

    it "iterates over page images" do
        valid_sizes = [567, 284, 386, 227, 500, 939, 950, 959]
        count = 0
        page.each_image do |img|
            count += 1
            next if count % 4 == 0 # to speed up spec runs only sample 1/4 of time
            assert_kind_of PDFium::Image, img
            file = Tempfile.new(['test',".png"])
            img.save(file.path)
            assert_includes valid_sizes, FastImage.size(file).first
            assert_includes valid_sizes, FastImage.size(file).last
        end
        assert_equal 26, count, "Incorrect # of images counted"
    end

    it "can return an ImageScience instance" do
        image = page.images.first
        assert image
        ims = image.as_science
        file = Tempfile.new(['test',"jpg"])
        ims.cropped_thumbnail(100) do |thumb|
           thumb.save file.path
           assert_size "100x100", file.path
        end

    end

end
