#include <stdlib.h>
#include <inttypes.h>
extern "C" {
#include "ruby.h"
}
#include <iostream>
#include <string>

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
    printf("GC Free Doc: %p\n" , doc);
    // Note: we do not actually destroy the object yet.
    // instead we mark it as unused and it will remove itself
    // once all pages are finished
    doc->markUnused();
}

static VALUE
document_allocate(VALUE klass) {
    Document *pdf = new Document();
    printf("Alloc PDF: %p\n", pdf);
    return Data_Wrap_Struct(klass, NULL, document_gc_free, pdf );
}

static VALUE
document_initialize(VALUE self, VALUE _path) {
    _path = rb_funcall(_path, to_s, 0); // call to_s in case it's a Pathname
    const char* path = StringValuePtr(_path);

    Document* p = getDocument(self);
    if (! p->initialize(path) ){
        rb_raise(rb_eRuntimeError, "Unable to parse %s", path);
    }
    return Qnil;
}


static VALUE
document_page_count(VALUE klass){
    return INT2NUM( getDocument(klass)->pageCount() );
}



/////////////////////////////////////////////////////////////////////////
// The Page class                                                      //
/////////////////////////////////////////////////////////////////////////

// a utility method to extract the reference to the Page class from the Ruby wrapping
static Page*
getPage(VALUE self) {
  Page* p;
  Data_Get_Struct(self, Page, p);
  return p;
}

static void
page_gc_free(Page* page) {
    printf("GC Free Page: %p\n", page);
    // The page's destructor will remove itself from the Document, and perform all cleanup
    delete page;
}

static VALUE
page_allocate(VALUE klass){
    Page *page = new Page();
    printf("Alloc Page: %p\n", page);
    return Data_Wrap_Struct(klass, NULL, page_gc_free, page);
}

static VALUE
page_initialize(VALUE self, VALUE _doc, VALUE page_number) {
    Page *page = getPage(self);
    Document   *pdf = getDocument(_doc);
    printf("INit page: %p pdf: %p\n", page, pdf);
    if (!page->initialize(pdf, FIX2INT(page_number))){
        rb_raise(rb_eRuntimeError, "Unable to load page %d", FIX2INT(page_number));
    }
    return Qnil;
}

static VALUE
page_dimensions(VALUE self){
    Page *pg = getPage(self);
    return rb_ary_new3(2, rb_float_new(pg->width()), rb_float_new(pg->height()) );
}


static VALUE
page_render(VALUE self, VALUE path, VALUE width, VALUE height){
    Page *pg = getPage(self);
    VALUE str = rb_funcall(path, to_s, 0);
    return pg->render( StringValuePtr(str), FIX2INT(width), FIX2INT(height) ) ? Qtrue : Qfalse;
}


extern "C"
void Init_pdfium_ext()
{
    Document::Initialize();
    to_s = rb_intern ("to_s");

    // Get symbol for PDFium
    ID sym_pdfium = rb_intern("PDFium");
    // Get the module
    VALUE rb_pdfium_module = rb_const_get(rb_cObject, sym_pdfium);

    // The Document class definition and methods
    rb_document = rb_define_class_under(rb_pdfium_module,"Document",  rb_cObject);
    rb_define_alloc_func(rb_document, document_allocate);
    rb_define_private_method (rb_document, "initialize", RUBY_METHOD_FUNC(document_initialize), 1);
    rb_define_method         (rb_document, "page_count", RUBY_METHOD_FUNC(document_page_count), 0);
    rb_define_method         (rb_document, "page_at",    RUBY_METHOD_FUNC(document_page_count), 1);

    // The Page class definition and methods
    rb_page = rb_define_class_under(rb_pdfium_module, "Page", rb_cObject);
    rb_define_alloc_func     (rb_page, page_allocate);
    rb_define_private_method (rb_page, "initialize", RUBY_METHOD_FUNC(page_initialize), 2);
    rb_define_method         (rb_page, "dimensions", RUBY_METHOD_FUNC(page_dimensions), 0);
    rb_define_method         (rb_page, "render",     RUBY_METHOD_FUNC(page_render),     3);

}
