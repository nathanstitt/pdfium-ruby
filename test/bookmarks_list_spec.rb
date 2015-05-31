require_relative 'spec_helper'

describe PDFium::BookmarkList do

    let(:api){ load_document("with_bookmarks") }
    let(:utf){ load_document("example_utf8")   }

    it "can be created" do
        assert_kind_of PDFium::BookmarkList, api.bookmarks
    end

    it "can be empty" do
        bm = PDFium::Bookmark.new(document: api)
        assert_kind_of PDFium::BookmarkList, bm.children
        assert bm.children.empty?, "First bookmark shouldn't have any children"
    end

    it "can iterate" do
        count = 0
        utf.bookmarks.each do | bm |
            count +=1
        end
        assert_equal 16, count
    end

end
