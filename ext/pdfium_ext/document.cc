#include "document.h"
#include "pdfium.h"
#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <fstream>
#include <map>
#include "buffer_file_write.hpp"

/////////////////////////////////////////////////////////////////////////
// The Document class                                                  //
/////////////////////////////////////////////////////////////////////////


// While you might think this would free the Document object it does not
// Instead it simply marks the Document as no longer in use, and then it
// will release itself when there are no Pages in use.
// https://redmine.ruby-lang.org/issues/6292
static void
document_gc_free(DocumentWrapper* doc)
{
    DEBUG_MSG("GC Free Doc: " << doc);
    // Note: we do not actually destroy the object yet.
    // instead we mark it as unused and it will remove itself
    // once all pages are finished
    doc->markUnused();
}

static VALUE
document_allocate(VALUE klass)
{
    DocumentWrapper *doc = new DocumentWrapper();
    DEBUG_MSG("Alloc PDF: " << doc);
    return Data_Wrap_Struct(klass, NULL, document_gc_free, doc );
}


/*
 * call-seq:
 *   Document.new( path_to_pdf_file ) -> Document
 *   Document.new() -> An empty PDF Document with no pages
 *
 * Initializes a document either from a PDF file or creates a blank document
 */
VALUE
document_initialize(int argc, VALUE *argv, VALUE self)
{
    DocumentWrapper* d;
    Data_Get_Struct(self, DocumentWrapper, d);
    if (argc){
        VALUE path = RB::to_s(argv[0]); // call to_s in case it's a Pathname
        d->document = (CPDF_Document*)FPDF_LoadDocument(StringValuePtr(path), NULL);
    } else {
        d->document = (CPDF_Document*)FPDF_CreateNewDocument();
    }
    if (! d->document ){
        rb_raise(rb_eArgError, "Unable to create document: %s", PDFiumLastErrorString());
    }
    return Qnil;
}


/*
 * call-seq:
 *   Document.from_memory( pdf_data ) -> Document
 *
 * Initializes a document from a binary string.
 *
 * See {PDFium::Image#data} for an example of reading a PDF directly from Amazon S3
 * and writing it's images completely in memory.
 */
static VALUE
document_from_memory(VALUE klass, VALUE data){
    DocumentWrapper *doc = new DocumentWrapper();
    VALUE instance = Data_Wrap_Struct(klass, NULL, document_gc_free, doc );
    doc->document = (CPDF_Document*)FPDF_LoadMemDocument(RSTRING_PTR(data), RSTRING_LEN(data),NULL);
    return instance;
}

/*
 * @overload page_count
 * page_count -> Fixnum
 *
 * Returns the number of pages on a Document
 */
static VALUE
document_page_count(VALUE self)
{
    return INT2NUM( RB2DOC(self)->GetPageCount() );
}

// Not documented in favor of the Document#pages[] access
/*
  @private
 */
static VALUE
document_page_at(VALUE self, VALUE rb_page_index)
{
    return rb_funcall(RB::Page(), rb_intern("open"), 2, self, rb_page_index);
}

/*
 * call-seq:
 *   pages
 * @return {PDFium::PageList}
 *
 * Returns a collection of all the pages on the document as a PDFium::PageList. Pages
 * are lazily loaded.
 *
 */
static VALUE
document_pages(VALUE self)
{
    VALUE args[1];
    args[0] = self;
    return rb_class_new_instance( 1, args, RB::PageList() );
}

// creates and yields a page.  Not documented since all access
// should got through the Pageist interface via the Document#pages method
/* @private */
static VALUE
document_each_page(VALUE self)
{
    auto doc = RB2DOC(self);
    auto count = doc->GetPageCount();
    for (int pg=0; pg < count; pg++){
        VALUE page = document_page_at(self, INT2FIX(pg));
        rb_yield(page);
        PageWrapper *pw;
        Data_Get_Struct(page, PageWrapper, pw);
        pw->unload();
    }
    return self;
}



/*
 * call-seq:
 *   bookmarks
 *
 * @return {PDFium::BookmarkList}
 *
 * Retrieves the first Bookmark for a document
 */
static VALUE
document_bookmarks(VALUE self)
{
    VALUE args[1];
    args[0] = rb_hash_new();
    rb_hash_aset(args[0], ID2SYM(rb_intern("document")), self);
    VALUE bm = rb_class_new_instance( 1, args, RB::Bookmark() );
    args[0] = bm;
    return rb_class_new_instance( 1, args, RB::BookmarkList() );
}



/*
 * call-seq:
 *   save(file)
 *
 * @param file [String, Pathname] path to save the file to
 * @return [Boolean] indicating success or failure
 *
 * Retrieves the first Bookmark for a document
 */
static VALUE
document_save(VALUE self, VALUE _path)
{
    auto doc = RB2DOC(self);
    VALUE path = RB::to_s(_path); // call to_s in case it's a Pathname
    BufferFileWrite output_file_write(StringValuePtr(path));
    FPDF_SaveAsCopy(doc, &output_file_write, FPDF_REMOVE_SECURITY);
    return self;
}


/*
  call-seq:
    metadata -> Hash

 Retrieves and optionally sets the metadata on a document. Returns a hash with the following keys:

   :title, :author :subject, :keywords, :creator, :producer, :creation_date, :mod_date

 An empty Hash will be returned if the metadata cannot be read

 All values in the hash are encoded as UTF-16LE strings.

 If caled with a block, the values will be passed to it and updates written back to the Document

 === Example
    pdf = PDFium::Document.new( "test.pdf" )
    pdf.metadata do | md |
        md[:title]  = "My Awesome PDF"
        md[:author] = "Nathan Stitt"
    end
    pdf.metadata[:author] # => "Nathan Stitt"

 */
VALUE
document_metadata(int argc, VALUE *argv, VALUE self)
{
    auto doc = RB2DOC(self);
    VALUE metadata = rb_hash_new();
    CPDF_Dictionary* info = doc->GetInfo();
    if (!info)
        return metadata;

    VALUE block;
    rb_scan_args(argc, argv, "0&", &block);

    std::map<std::string, std::string> keys = {
        { "Title",        "title"  },
        { "Author",       "author" },
        { "Subject",      "subject" },
        { "Keywords",     "keywords"},
        { "Creator",      "creator" },
        { "Producer",     "producer"},
        { "CreationDate", "creation_date" },
        { "ModDate",      "mod_date" }
    };

    for (auto& kv : keys) {
        rb_hash_aset(metadata,
                     ID2SYM( rb_intern( kv.second.c_str() ) ),
                     RB::to_string( info->GetUnicodeText( kv.first.c_str() ) )
                     );
    }

    if (RTEST(block)){
        rb_yield( metadata );
        for (auto& kv : keys) {
            VALUE value = RB::get_option(metadata, kv.second);
            auto bs = CFX_ByteString( RSTRING_PTR(value), RSTRING_LEN(value) );
            info->SetAtString(kv.first.c_str(), bs);
        }
    }

    return metadata;
}

void
define_document_class()
{
    VALUE PDFium = RB::PDFium();

    // The Document class definition and methods
    VALUE RB_Document = rb_define_class_under(PDFium, "Document",  rb_cObject);

    rb_define_alloc_func(RB_Document, document_allocate);

    rb_define_singleton_method(RB_Document, "from_memory", RUBY_METHOD_FUNC(document_from_memory), 1);

    rb_define_private_method (RB_Document, "initialize", RUBY_METHOD_FUNC(document_initialize), -1);
    rb_define_method         (RB_Document, "page_count", RUBY_METHOD_FUNC(document_page_count),  0);
    rb_define_method         (RB_Document, "page_at",    RUBY_METHOD_FUNC(document_page_at),     1);
    rb_define_method         (RB_Document, "each_page",  RUBY_METHOD_FUNC(document_each_page),   0);
    rb_define_method         (RB_Document, "pages",      RUBY_METHOD_FUNC(document_pages),       0);
    rb_define_method         (RB_Document, "metadata",   RUBY_METHOD_FUNC(document_metadata),   -1);
    rb_define_method         (RB_Document, "bookmarks",  RUBY_METHOD_FUNC(document_bookmarks),   0);
    rb_define_method         (RB_Document, "save",       RUBY_METHOD_FUNC(document_save),        1);
}
