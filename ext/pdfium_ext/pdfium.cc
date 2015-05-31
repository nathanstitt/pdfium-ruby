#include <iostream>
#include <string>

#include "pdfium.h"
extern "C" {
#include "ruby/encoding.h"
}

#include <assert.h>
#include <list>
#include <string>
#include <utility>
#include <vector>

#include "document.h"
#include "page.h"

// // file local variables that are set in Init_pdfium_ext function
// // and are referenced elsewhere in file
// static VALUE rb_page;  // Ruby definition of the Page class

const char*
PDFiumLastErrorString() {
    switch(FPDF_GetLastError()){
    case 0:
        return "No Error";
    case 1:
        return "unknown error";
    case 2:
        return "file not found or could not be opened";
    case 3:
        return "file not in PDF format or corrupted";
    case 4:
        return "password required or incorrect password";
    case 5:
        return "unsupported security scheme";
    case 6:
        return "page not found or content error";
    default:
        return "error code unknown";
    }
}


VALUE _get(const char *name){
    return rb_const_get(RB::PDFium(), rb_intern(name));
}
VALUE RB::PDFium(){
    static VALUE val = rb_const_get(rb_cObject, rb_intern("PDFium"));
    return val;
}
VALUE RB::Page(){
    static VALUE val = _get("Page");
    return val;
}
VALUE RB::Image(){
    static VALUE val = _get("Image");
    return val;
}
VALUE RB::BoundingBox(){
    static VALUE val = _get("BoundingBox");
    return val;
}
VALUE RB::Bookmark(){
    static VALUE val = _get("Bookmark");
    return val;
}
VALUE RB::Document(){
    static VALUE val = _get("Document");
    return val;
}
VALUE RB::BookmarkList(){
    static VALUE val = _get("BookmarkList");
    return val;
}
VALUE RB::PageList(){
    static VALUE val = _get("PageList");
    return val;
}
VALUE RB::ImageList(){
    static VALUE val = _get("ImageList");
    return val;
}

VALUE
RB::to_string(const CFX_WideString &wstr){
    static rb_encoding *enc = rb_enc_find("UTF-16LE");
    return rb_enc_str_new(wstr.UTF16LE_Encode().c_str(),(wstr.GetLength()*2), enc);
}

ID RB::to_s(VALUE obj){
    static ID id = rb_intern("to_s");
    return rb_funcall(obj, id, 0);
}

VALUE RB::type(VALUE obj){
    static ID id = rb_intern("class");
    return rb_funcall(obj, id, 0);
}

VALUE RB::get_option(VALUE options, const std::string &key){
    return rb_hash_aref(options, ID2SYM(rb_intern(key.c_str())));
}

extern "C" void
Init_pdfium_ext()
{
    // Initialize the PDFium library
    FPDF_InitLibrary();

    define_document_class();
    define_page_class();
    define_bookmark_class();
    define_image_class();
}
