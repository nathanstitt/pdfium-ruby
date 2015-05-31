# -*- coding: utf-8 -*-
require_relative 'spec_helper'

describe PDFium::Page do
    let(:utfdoc){ PDFium::Document.new( pdf_path("example_utf8") ) }
    let(:textdoc){ PDFium::Document.new( pdf_path("example_divs") ) }

    let(:page) { PDFium::Page.open(utfdoc,0) }


    it "opens existing page" do
        assert_kind_of PDFium::Page, page
    end

    it "creates a page" do
        page = PDFium::Page.create(utfdoc,0)
        assert_kind_of PDFium::Page, page
    end

    it "adds pages to an existing document" do
        pdf = PDFium::Document.new
        PDFium::Page.create(pdf)
        PDFium::Page.create(pdf)
        assert_equal 2, pdf.page_count
    end

    it "has dimensions" do
        assert_in_delta 595.28, page.width
        assert_in_delta 841.89, page.height
    end

    it "refuses to open invalid page ranges" do
        assert_raises(RangeError) do
            PDFium::Page.open(utfdoc,-1)
        end
        assert_raises(RangeError) do
            PDFium::Page.open(utfdoc,90)
        end
    end

    it "creates new pages" do
        pdf = PDFium::Document.new
        page = PDFium::Page.create(pdf,0, width:100, height:180)
        assert_equal 100, page.width
        after_saving(pdf) do | saved |
            assert_equal 1, saved.page_count
            assert_equal 100.0, saved.page_at(0).width
        end
    end

    it "can't be created using new" do
        assert_raises(RuntimeError){ PDFium::Page.new }
    end

    it "can load/unload page" do
        pdf = PDFium::Document.new
        10.times do
          page = PDFium::Page.create(pdf)
          page.unload
        end
        assert_equal 10, pdf.page_count
        page = pdf.page_at(1)
        assert_equal 612.0, page.width
        page.unload
        assert_equal 612.0, page.width
    end

    it "can read text" do
        ascii_text = PDFium::Page.open(textdoc,1).text.encode!("ASCII-8BIT")
        assert_match /Cras tellus. Fusce aliquet/, ascii_text
    end

    it "can read utf text" do
        strings = [
            "Жълтата дюля беше щастлива",
            "Jove xef, porti whisky amb quinze glaçons d'hidrogen",
            "Příliš žluťoučký kůň úpěl ďábelské ódy",
            "Høj bly gom vandt fræk sexquiz på wc",
            "Doch Bep, flink sexy qua vorm, zwijgt",
            "Törkylempijä vongahdus",
            "Falsches Üben von Xylophonmusik quält jeden größeren Zwerg"
        ]
        text = PDFium::Page.open(utfdoc,0).text
        utf8 = text.encode("UTF-8")
        strings.each do | sentence |
            assert_match sentence, utf8
            assert_match sentence.encode("UTF-16LE"), text
        end
    end

end
