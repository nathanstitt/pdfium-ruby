#include <stdlib.h>
#include <inttypes.h>
extern "C" {
#include "ruby.h"
}
#include <iostream>
#include <string>

#include "pdfium.h"

#include "fpdf_dataavail.h"
#include "fpdf_ext.h"
#include "fpdfformfill.h"
#include "fpdftext.h"
#include "fpdfview.h"
//#include "v8/v8.h"

#include <assert.h>
#include <list>
#include <string>
#include <utility>
#include <vector>

#include "document.h"
#include "page.h"


// file local variables that are set in Init_pdfium_ext function
// and are referenced elsewhere in file
static ID to_s;        // used for calling the to_s method on file paths
static VALUE rb_document;   // Ruby definition of the Document class
static VALUE rb_page;  // Ruby definition of the Page class


/////////////////////////////////////////////////////////////////////////
// The Document class                                                       //
/////////////////////////////////////////////////////////////////////////
/*
    * Document-class:  PDFium::Document
    *
    * A Document represents a PDF file.
    *
*/



// a utility method to extract the reference to the Document class from the Ruby wrapping
static Document*
getDocument(VALUE self) {
  Document* doc;
  Data_Get_Struct(self, Document, doc);
  return doc;
}

// While you might think this would free the Document object it does not
// Instead it simply marks the Document as no longer in use, and then it
// will release itself when there are no Pages in use.
// https://redmine.ruby-lang.org/issues/6292
static void
document_gc_free(Document* doc) {
    debug_printf("GC Free Doc: %p" , doc);
    // Note: we do not actually destroy the object yet.
    // instead we mark it as unused and it will remove itself
    // once all pages are finished
    doc->markUnused();
}

static VALUE
document_allocate(VALUE klass) {
    Document *pdf = new Document();
    debug_printf("Alloc PDF: %p", pdf);
    return Data_Wrap_Struct(klass, NULL, document_gc_free, pdf );
}

/*
 * call-seq:
 *   Document.new( pdf_file ) -> Document
 *
 * Initializes a document
 */
static VALUE
document_initialize(VALUE self, VALUE _path) {
    _path = rb_funcall(_path, to_s, 0); // call to_s in case it's a Pathname
    const char* path = StringValuePtr(_path);
    Document* p = getDocument(self);
    if (! p->initialize(path) ){
        rb_raise(rb_eArgError, "Unable to parse %s", path);
    }
    return Qnil;
}


/*
 * call-seq:
 *   page_count -> Fixnum
 *
 * Returns the number of pages on a Document
 */
static VALUE
document_page_count(VALUE self)
{
    return INT2NUM( getDocument(self)->pageCount() );
}


/*
 * call-seq:
 *   page_at( page_number ) -> Page
 *
 * Returns a Page instance for the given number.
 *
 * If the given page_number is not valid, an ArgumentError will be raised.
 *
 * _note_:  subsequent calls to this function with the same page_number will return different Page instance.
 */
static VALUE
document_page_at(VALUE self, VALUE rb_page_index)
{
    VALUE args[2];
    args[0] = self;
    args[1] = rb_page_index;
    return rb_class_new_instance( 2, args, rb_page );
}

/*
 * call-seq:
 *   each_page -> yields: Page
 *
 * Yields a page instance for each page on the document
 *
 */
static VALUE
document_each_page(VALUE self)
{
    Document *doc = getDocument(self);
    for (int pg=0; pg < doc->pageCount(); pg++){
        rb_yield( document_page_at(self, INT2FIX(pg) ) );
    }
    return self;
}


/////////////////////////////////////////////////////////////////////////
// The Page class                                                      //
/////////////////////////////////////////////////////////////////////////

/*
    * Document-class:  PDFium::Page
    *
    * A Page is a Page
*/

// a utility method to extract the reference to the Page class from the Ruby wrapping
static Page*
getPage(VALUE self) {
  Page* p;
  Data_Get_Struct(self, Page, p);
  return p;
}

static void
page_gc_free(Page* page) {
    debug_printf("GC Free Page: %p", page);
    // The page's destructor will remove itself from the Document, and perform all cleanup
    delete page;
}

static VALUE
page_allocate(VALUE klass)
{
    Page *page = new Page();
    debug_printf("Alloc Page: %p\n", page);
    return Data_Wrap_Struct(klass, NULL, page_gc_free, page);
}

/*
 * call-seq:
 *   Page.new(PDFIum::Document, page_index) -> Page
 *
 * initializes a page
 */
static VALUE
page_initialize(VALUE self, VALUE _doc, VALUE page_number) {
    Page *page = getPage(self);
    Document   *pdf = getDocument(_doc);
    if (!page->initialize(pdf, FIX2INT(page_number))){
        rb_raise(rb_eArgError, "Unable to load page %d", FIX2INT(page_number));
    }
    return Qnil;
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
    Page *pg = getPage(self);
    return rb_float_new(pg->width());
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
    Page *pg = getPage(self);
    return rb_float_new(pg->height());
}
/*
 * call-seq:
 *   number -> Fixnum
 *
 * Returns the page number.  Is zero indexed, i.e. a 0 (zero) is the first page.
  */
static VALUE
page_number(VALUE self)
{
    return INT2FIX( getPage(self)->number() );
}



/*
 call-seq:
    render(file_path, width, height) -> Boolean

 Render a page as an image of width and height to the given file.  The image type
 will be auto-detected from the file_path's extension, and can be any of the
 formats supported by the FreeImage library http://freeimage.sourceforge.net/features.html

 If either the height or width are set to 0, it will be calculated to retain
 approprate page scale

 Returns true for success false for failure

 === Example
    pdf = PDFium::Document.new( "test.pdf" )
    page = PDFium::Page.new(pdf,0)
    page.render("test-tn.png", 100, 180)
    page.render("test-large.png", 2000, 0)

 If the above page's #dimensions was 1000x1500, then the following image sizes would be generated:

    test-tn.png -> 100x180 and test-large.png -> 2000x3000

*/

static VALUE
page_render(VALUE self, VALUE path, VALUE width, VALUE height)
{
    Page *pg = getPage(self);
    VALUE str = rb_funcall(path, to_s, 0);
    return pg->render( StringValuePtr(str), FIX2INT(width), FIX2INT(height) ) ? Qtrue : Qfalse;
}


/*
 call-seq:
    render_sizes(file_path, Array< Array<width, height> >) -> Boolean

 Render a page into multiple images.  The image type
 will be auto-detected from the file_path's extension, and can be any of the
 formats supported by the FreeImage library http://freeimage.sourceforge.net/features.html

 The file_path must contain placeholders for the height and width, which are given as %h (height) and %w (width).

 The sizes are given as an Array with nested Arrays containing each height / width

 As in the #render method, if either the height or width are set to 0, it will be calculated to retain
 approprate page scale

 Returns true for success false for failure

 === Example
    pdf = PDFium::Document.new( "test.pdf" )
    page = PDFium::Page.new(pdf,0)
    page.render_sizes("test-%hx%w.png", [ [100,100], [300,0], [500,0]]

 If the above page's #dimensions was 1000x1500, then the following images would be generated:

 test-100x100.png, test-300x450.png and test-500x750.png

*/

static VALUE
page_render_sizes(VALUE self, VALUE path, VALUE rb_sizes)
{
    Page *pg = getPage(self);
    VALUE str = rb_funcall(path, to_s, 0);
    const char* dest = StringValuePtr(str);
    if (TYPE(rb_sizes) != T_ARRAY){
        return Qfalse;
    }
    Page::sizes_t sizes(RARRAY_LEN(rb_sizes));
    int index = 0;
    for (Page::sizes_t::iterator size = sizes.begin(); size != sizes.end(); ++size){
        VALUE rb_size = rb_ary_entry(rb_sizes, index);
        // not sure what we should do here.  Right now we'll just
        // skip the element, but perhaps we should return false?
        if (TYPE(rb_size) != T_ARRAY || RARRAY_LEN(rb_size) !=2 ){
            index++;
            continue;
        }
        size->first  = FIX2INT( rb_ary_entry(rb_size, 0) );
        size->second = FIX2INT( rb_ary_entry(rb_size, 1) );
        index++;
    }
    return pg->render( dest, sizes )  ? Qtrue : Qfalse;
}


static VALUE
page_render_resize(VALUE self, VALUE path, VALUE rb_sizes)
{
    Page *pg = getPage(self);
    VALUE str = rb_funcall(path, to_s, 0);
    const char* dest = StringValuePtr(str);
    if (TYPE(rb_sizes) != T_ARRAY){
        return Qfalse;
    }
    Page::sizes_t sizes(RARRAY_LEN(rb_sizes));
    int index = 0;
    for (Page::sizes_t::iterator size = sizes.begin(); size != sizes.end(); ++size){
        VALUE rb_size = rb_ary_entry(rb_sizes, index);
        // not sure what we should do here.  Right now we'll just
        // skip the element, but perhaps we should return false?
        if (TYPE(rb_size) != T_ARRAY || RARRAY_LEN(rb_size) !=2 ){
            index++;
            continue;
        }
        size->first  = FIX2INT( rb_ary_entry(rb_size, 0) );
        size->second = FIX2INT( rb_ary_entry(rb_size, 1) );
        index++;
    }
    return pg->render_resize( dest, sizes )  ? Qtrue : Qfalse;
}

extern "C" void Init_pdfium_ext()
{
    Document::Initialize();
    to_s = rb_intern ("to_s");

    // Get symbol for PDFium
    ID sym_pdfium = rb_intern("PDFium");
    // Get the module
    VALUE rb_pdfium_module = rb_const_get(rb_cObject, sym_pdfium);
#if RDOC_CAN_PARSE_DOCUMENTATION
    rb_pdfium_module = rb_define_module("PDFium");
#endif
    // The Document class definition and methods
    rb_document = rb_define_class_under(rb_pdfium_module,"Document",  rb_cObject);
    rb_define_alloc_func(rb_document, document_allocate);
    rb_define_private_method (rb_document, "initialize", RUBY_METHOD_FUNC(document_initialize), 1);
    rb_define_method         (rb_document, "page_count", RUBY_METHOD_FUNC(document_page_count), 0);
    rb_define_method         (rb_document, "page_at",    RUBY_METHOD_FUNC(document_page_at),    1);
    rb_define_method         (rb_document, "each_page",  RUBY_METHOD_FUNC(document_each_page),  0);

    // The Page class definition and methods
    rb_page = rb_define_class_under(rb_pdfium_module, "Page", rb_cObject);
    rb_define_alloc_func     (rb_page, page_allocate);
    rb_define_private_method (rb_page, "initialize",   RUBY_METHOD_FUNC(page_initialize), 2);
    rb_define_method         (rb_page, "width",        RUBY_METHOD_FUNC(page_width),      0);
    rb_define_method         (rb_page, "height",       RUBY_METHOD_FUNC(page_height),     0);
    rb_define_method         (rb_page, "render",       RUBY_METHOD_FUNC(page_render),     3);
    rb_define_method         (rb_page, "number",       RUBY_METHOD_FUNC(page_number),     0);

    rb_define_method         (rb_page, "render_sizes",  RUBY_METHOD_FUNC(page_render_sizes), 2);
    rb_define_method         (rb_page, "render_resize", RUBY_METHOD_FUNC(page_render_resize), 2);


}
