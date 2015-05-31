#include "page.h"
#include "page_wrapper.h"
#include "pdfium.h"
#include <FreeImage.h>

#include "fpdfview.h"
#include <limits.h>
#include <algorithm>
#include <string>
extern "C" {
#include "ruby/encoding.h"
}

static VALUE rb_sym_height;
static VALUE rb_sym_width;
//static VALUE RB_Document;


/////////////////////////////////////////////////////////////////////////
// The Page class                                                      //
/////////////////////////////////////////////////////////////////////////

/*
    * Document-class:  PDFium::Page
    *
    * A Page on a PDF Document
*/
static void
page_gc_free(PageWrapper* page)
{
    DEBUG_MSG("GC Free Page: " << page);
    // The page's destructor will remove itself from the Document, and perform all cleanup
    page->markUnused();
}


/*
 * call-seq:
 *   Page.new -> raises RuntimeError
 *
 * Pages cannot be created by using Page.new, instead Page.open or Page.create
 * should be used
 */
static VALUE
page_new(VALUE klass)
{
    rb_raise(rb_eRuntimeError, "Use Page.open or Page.create");
}


/*
 * call-seq:
 *   Page.open(PDFIum::Document, page_index) -> Page
 *
 * Opens a given page on a document
 */
static VALUE
page_open(VALUE klass, VALUE rb_document, VALUE rb_page_number)
{
    DocumentWrapper *doc_wrapper;
    Data_Get_Struct(rb_document, DocumentWrapper, doc_wrapper);

    int pg = FIX2INT(rb_page_number);
    if ( pg < 0 || pg >= doc_wrapper->document->GetPageCount() ){
        rb_raise(rb_eRangeError, "%d is out of range: 0...%d",
                 pg, doc_wrapper->document->GetPageCount() );
    }

    PageWrapper *page_wrapper = new PageWrapper(doc_wrapper, FIX2INT(rb_page_number));
    return Data_Wrap_Struct(klass, NULL, page_gc_free, page_wrapper);
}




/*
 * call-seq:
 *   Page.create(PDFIum::Document, page_number=document.page_count) -> Page
 *
 * Creates a new page on a document.  The page_number defaults to the
 * Document#page_count, causing pages to be appended to the back of the document
 * by default if no page_number is given.
 */
static VALUE
page_create(int argc, VALUE *argv, VALUE klass)
{
    VALUE rb_document, rb_page_number, options;
    rb_scan_args(argc, argv, "11:", &rb_document, &rb_page_number, &options);
    if (NIL_P(options)){
        options=rb_hash_new();
        rb_hash_aset(options, ID2SYM(rb_intern("size")),
                     rb_const_get(RB::PDFium(), rb_intern("LETTER")) );
    }

    VALUE size, rb_width, rb_height;
    if ( !NIL_P(size = RB::get_option(options,"size")) ){
        rb_width = rb_ary_entry(size, 0);
        rb_height = rb_ary_entry(size, 1);
    } else {
        rb_width = RB::get_option(options,"width");
        rb_height = RB::get_option(options,"height");
    }


    if ( NIL_P(rb_width) || NIL_P(rb_height) ){
        rb_raise(rb_eArgError, ":height or :width must be given");
    }

    DocumentWrapper *doc_wrapper;
    Data_Get_Struct(rb_document, DocumentWrapper, doc_wrapper);

    int page_number;
    if (NIL_P(rb_page_number)){
        page_number = doc_wrapper->document->GetPageCount();
    } else {
        page_number = FIX2INT(rb_page_number);
    }

    if ( page_number < 0 || page_number > doc_wrapper->document->GetPageCount() ){
        rb_raise(rb_eRangeError, "%d is out of range: 0...%d",
                 page_number, doc_wrapper->document->GetPageCount() );
    }


    CPDF_Page* newpage = (CPDF_Page*)FPDFPage_New(doc_wrapper->document, page_number,
                                                  FIX2INT(rb_width), FIX2INT(rb_height) );

    PageWrapper *page_wrapper = new PageWrapper(doc_wrapper, rb_page_number);
    page_wrapper->setPage(newpage);
    VALUE i=Data_Wrap_Struct(klass, NULL, page_gc_free, page_wrapper);
    return i;
}


/*
 * call-seq:
 *   width -> Float
 *
 * Returns the width of the page.
 * The width is given in terms of points, which are set to 72 per inch. (DPI)
 */
static VALUE
page_width(VALUE self)
{
    return rb_float_new( FPDF_GetPageWidth(RB2PG(self)) );
}

/*
 * call-seq:
 *   height -> Float
 *
 * Returns the height of the page.
 * The height is given in terms of points, which are set to 72 per inch. (DPI)
 */
static VALUE
page_height(VALUE self)
{
    return rb_float_new( FPDF_GetPageHeight(RB2PG(self)) );
}


/*
 * call-seq:
 *   text -> String
 *
 * Returns the text that is contained on the page as a UTF-16LE encoded string
 */
static VALUE
page_text(VALUE self)
{
    static rb_encoding *enc = rb_enc_find("UTF-16LE");


    PageWrapper *pw;
    Data_Get_Struct(self, PageWrapper, pw);

    CPDF_Page *page = pw->page();
    IPDF_TextPage *text_page = (IPDF_TextPage*)FPDFText_LoadPage(page);
    //
    unsigned int buff_size = text_page->CountChars()*2 + 1; // 16 bit per char, plus terminator
    char *buffer = ALLOC_N(char, buff_size );



    FPDFText_GetText((FPDF_TEXTPAGE)text_page, 0, text_page->CountChars(), (unsigned short*)buffer);


    VALUE ret = rb_enc_str_new(buffer, buff_size-1, enc);

    xfree(buffer);

    return ret;
}




/*
 * call-seq:
 *   number -> Fixnum
 *
 * Returns the page number that the page represents on the document.
 * It is *NOT* zero based, meaning that the first page#number will be 1.
 *
 * *Warning:* if pages are added/removed after the page is loaded, this value will be inaccurate.
 */
static VALUE
page_number(VALUE self)
{
    PageWrapper *pw;
    Data_Get_Struct(self, PageWrapper, pw);
    return INT2FIX(pw->_page_number+1);
}



/*
 call-seq:
    images -> ImageList

 Returns ImageList which contains all the images on the page.  Images are lazily loaded only when requested.

 === Example
    pdf = PDFium::Document.new( "test.pdf" )
    page = pdf.pages.first
    page.images.each do | image |
        image.save("pg-#{page.number}-#{image.index}.png")
    end

*/
static VALUE
page_images(VALUE self)
{
    VALUE args[1];
    args[0] = self;
    return rb_class_new_instance( 1, args, RB::ImageList() );
}

/*
 call-seq:
    as_image(width:nil, height:nil) -> Image

 Render a page as an image of width and height to the given file.  The image type
 will be auto-detected from the file_path's extension, and can be any of the
 formats supported by the FreeImage library http://freeimage.sourceforge.net/features.html

 If neither the height or width are given, it will be calculated to retain the
 approprate page scale.

 Returns an Image instance.

 === Example
    pdf = PDFium::Document.new( "test.pdf" )
    page = pdf.pages[0]
    page.as_image(height: 100, width: 75).save("pg-#{page.number}-sm.png")
    page.as_image(height: 500).save("pg-#{page.number}-md.png")
    page.as_image(width: 1000).save("pg-#{page.number}-lg.png")

 If the above page's #dimensions were 1000x1500, then the following images would be generated:
    pg-1-sm.png -> 100x75
    pg-1-md.png -> 500x750
    pg-1-lg.png -> 750x1000
*/
static VALUE
page_as_image(int argc, VALUE *argv, VALUE self)
{
    CPDF_Page *page = RB2PG(self);

    VALUE rb_options;
    rb_scan_args(argc,argv,":", &rb_options);
    if (NIL_P(rb_options)){
        rb_options=rb_hash_new();
    }
    if ( TYPE(rb_options) != T_HASH ){
        rb_raise(rb_eTypeError, "wrong argument type %s (expected Hash)", rb_obj_classname(rb_options));
    }

    VALUE width_option  = rb_hash_aref(rb_options, rb_sym_width);
    VALUE height_option = rb_hash_aref(rb_options, rb_sym_height);

    int width  = NIL_P(width_option)  ? 0 : FIX2INT(width_option);
    int height = NIL_P(height_option) ? 0 : FIX2INT(height_option);
    if (!width && !height){
        width = FPDF_GetPageWidth(page) * 2;
    }

    if (!width)
        width = FPDF_GetPageWidth(page) * ( (double)height / FPDF_GetPageHeight(page) );
    if (!height)
        height = FPDF_GetPageHeight(page) * ( (double)width / FPDF_GetPageWidth(page) );

    VALUE args[2];
    args[0] = self;
    VALUE img_options = args[1] = rb_hash_new();
    rb_hash_aset(img_options, rb_sym_width,  INT2FIX(width));
    rb_hash_aset(img_options, rb_sym_height, INT2FIX(height));

    VALUE bounds_args[4];
    bounds_args[0] = rb_float_new( 0 );
    bounds_args[1] = rb_float_new( FPDF_GetPageWidth(page) );
    bounds_args[2] = rb_float_new( 0 );
    bounds_args[3] = rb_float_new( FPDF_GetPageHeight(page) );
    VALUE bounds = rb_class_new_instance( 4, bounds_args, RB::BoundingBox() );
    rb_hash_aset(img_options, ID2SYM(rb_intern("bounds")), bounds);

    return rb_class_new_instance( 2, args, RB::Image() );
}

/*
 * call-seq:
 *   unload -> Page
 *
 * Frees a large portion of the internal memory allocated to the page.
 * When a page is parsed by the PDFIum engine, various elements are cached in memory
 * While Ruby will eventually garbage collect the Page instance once it's no longer
 * in use, this method will free the memory immediatly. Page#unload is safe to use
 * since the Page will re-load itself as needed, but calling it  while the page
 * is still in use will cause additional work by the engine since it will have to
 * repeatedly re-parse the page when it re-loads itself.
 *
 * PageList#each will call this method on each page after it yields.
 */
static VALUE
page_unload(VALUE self)
{
    PageWrapper *pw;
    Data_Get_Struct(self, PageWrapper, pw);
    pw->unload();
    return self;
}

// creates and yeilds an image.  Not documented since all access
// should got through the ImageList interface via the Page#images method
/* :nodoc: */
static VALUE
page_each_image(VALUE self)
{
    PageWrapper *pw;
    Data_Get_Struct(self, PageWrapper, pw);

    auto count = pw->page()->CountObjects();
    int image_index=0;
    for (int index=0; index < count; index++){
        CPDF_PageObject *object = pw->page()->GetObjectByIndex(index);
        if ( PDFPAGE_IMAGE == object->m_Type ){
            VALUE args[2];
            args[0] = self;
            VALUE img_options = args[1] = rb_hash_new();

            rb_hash_aset(img_options, ID2SYM(rb_intern("object_index")), INT2FIX(index));

            rb_hash_aset(img_options, ID2SYM(rb_intern("index")), INT2FIX(image_index));

            VALUE img = rb_class_new_instance( 2, args, RB::Image() );
            rb_yield( img );
            image_index++;
        }
    }
    return self;
}


VALUE
define_page_class()
{
    rb_sym_width  = ID2SYM(rb_intern("width"));
    rb_sym_height = ID2SYM(rb_intern("height"));

    VALUE RB_PDFium = RB::PDFium();

    // The Page class definition and methods
    VALUE RB_Page = rb_define_class_under(RB_PDFium, "Page", rb_cObject);
    //rb_define_alloc_func     (RB_Page, page_allocate);
    //rb_define_private_method (RB_Page, "initialize",   RUBY_METHOD_FUNC(page_initialize), -1);

    rb_define_singleton_method(RB_Page, "new",    RUBY_METHOD_FUNC(page_new),    0);
    rb_define_singleton_method(RB_Page, "open",   RUBY_METHOD_FUNC(page_open),   2);
    rb_define_singleton_method(RB_Page, "create", RUBY_METHOD_FUNC(page_create), -1);

    rb_define_method         (RB_Page, "text",       RUBY_METHOD_FUNC(page_text),      0);
    rb_define_method         (RB_Page, "width",      RUBY_METHOD_FUNC(page_width),      0);
    rb_define_method         (RB_Page, "height",     RUBY_METHOD_FUNC(page_height),     0);
    rb_define_method         (RB_Page, "as_image",   RUBY_METHOD_FUNC(page_as_image),  -1);
    rb_define_method         (RB_Page, "unload",     RUBY_METHOD_FUNC(page_unload),     0);
    rb_define_method         (RB_Page, "number",     RUBY_METHOD_FUNC(page_number),     0);

    rb_define_method         (RB_Page, "images",     RUBY_METHOD_FUNC(page_images),     0);

    rb_define_method         (RB_Page, "each_image", RUBY_METHOD_FUNC(page_each_image), 0);

    return RB_Page;
}
