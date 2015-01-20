require 'minitest/autorun'
require 'pdfium'

class MiniTest::Spec

    def pdf_path(name)
        Pathname.new(__FILE__).dirname.join('pdfs',name+'.pdf')
    end

end
