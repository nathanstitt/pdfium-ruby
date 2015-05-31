require 'minitest/autorun'
require_relative '../lib/pdfium'
require 'pathname'
require 'fastimage'

class MiniTest::Spec

    def load_document(name)
        PDFium::Document.new(pdf_path(name))
    end

    def pdf_path(name)
        Pathname.new(__FILE__).dirname.join('pdfs',name+'.pdf')
    end

    def after_saving(pdf)
        Tempfile.open(['test','.pdf']) do |f|
            pdf.save(f.path)
            yield PDFium::Document.new(f.path)
        end
    end

    def assert_size(size, file)
        ext = File.extname(file).gsub(/^\./,'')
        img_size = FastImage.size(file)
        img_size = img_size ? img_size.join("x") : "0x0"
        assert_match size, img_size, "Expected size to be #{size} but was #{img_size}"
        img_type=FastImage.type(file).to_s
        assert_match ext,  img_type, "Expected type to be #{ext} but was #{img_type}"
    end
end
