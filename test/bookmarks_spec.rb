require_relative 'spec_helper'

describe PDFium::Bookmark do
    let(:api){ load_document("with_bookmarks") }

    it "can be read" do
        bm = PDFium::Bookmark.new(document: api)
        assert_equal Encoding::UTF_16LE, bm.title.encoding
        assert_equal "INDEX", bm.title.encode!("ASCII-8BIT")
    end

    it "can create siblings" do
        bm = PDFium::Bookmark.new(document: api)
        second = bm.next_sibling
        assert_kind_of PDFium::Bookmark, second
        assert_equal "Chapter 1", second.title.encode!("ASCII-8BIT")
        refute second.next_sibling, "PDF shouldn't have 3 top level bookmarks"
    end

    it "can create children" do
        bm = PDFium::Bookmark.new(document: api)
        children = bm.next_sibling.children
        assert_kind_of PDFium::BookmarkList, children
        assert_equal 1, children.count
    end

    it "has destinations" do
        bm = PDFium::Bookmark.new(document: api).next_sibling
        assert_kind_of Hash, bm.destination
        assert_equal :destination, bm.destination[:type]
        assert_equal 1, bm.destination[:page_number]
    end

end
