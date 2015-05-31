require_relative 'spec_helper'
require 'tempfile'


describe PDFium do

    it "creates classes" do
        assert PDFium::Document
        assert PDFium::Page
        assert PDFium::Bookmark
        assert PDFium::BookmarkList
    end


end
