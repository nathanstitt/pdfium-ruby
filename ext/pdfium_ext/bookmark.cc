#include "pdfium.h"

/////////////////////////////////////////////////////////////////////////
// The Bookmark class
/////////////////////////////////////////////////////////////////////////
/*
    * Document-class:  PDFium::Bookmark
    *
    * Bookmarks on a Document form a tree structure.
    * Each can have siblings and children
    *
*/


class Bookmark {
  public:
    Bookmark():
        doc_wrapper(0),
        bookmark(0){}
    ~Bookmark(){
        if (doc_wrapper)
            doc_wrapper->release(this);
        if (bookmark)
            delete bookmark;
    }
    DocumentWrapper *doc_wrapper;
    CPDF_Bookmark *bookmark;
};

// a utility method to extract the reference to the FPDF_DOCUMENT from the Ruby/C++ wrapping
CPDF_Bookmark *
RB2BM(VALUE self) {
    Bookmark *bm;
    Data_Get_Struct(self, Bookmark, bm);
    return bm->bookmark;
}

static void
bookmark_gc_free(Bookmark *bm) {
    delete bm;
}

static VALUE
bookmark_allocate(VALUE klass) {
    auto bm = new Bookmark;
    return Data_Wrap_Struct(klass, NULL, bookmark_gc_free, bm );
}


/*
 * call-seq:
 *   Bookmark.new
 *
 * Initializes a bookmark.  Not intended for direct use, but called
 * internally from Document#bookmarks
 */
VALUE
bookmark_initialize(VALUE self, VALUE options){
    Bookmark *bm;
    Data_Get_Struct(self, Bookmark, bm);
    DocumentWrapper *doc;
    CPDF_Bookmark bookmark;

    if (TYPE(options) != T_HASH){
        rb_raise(rb_eArgError, "no options given");
        return Qnil;
    }

    VALUE  reference;
    if ( !NIL_P(reference = RB::get_option(options,"document")) ){
        // we're the first bookmark on a document
        Data_Get_Struct(reference, DocumentWrapper, doc);
        bookmark = CPDF_Bookmark(NULL);
        CPDF_BookmarkTree tree(doc->document);
        bm->bookmark = new CPDF_Bookmark( tree.GetFirstChild(bookmark).GetDict() );
    } else if ( !NIL_P(reference = RB::get_option(options,"parent")) ){
        // we're the first sibling on a parent bookmark
        Bookmark *reference_bm;
        Data_Get_Struct(reference, Bookmark, reference_bm);
        doc = reference_bm->doc_wrapper;
        bm->bookmark = new CPDF_Bookmark(reference_bm->bookmark->GetDict());
    } else if ( !NIL_P(reference = RB::get_option(options,"sibling")) ){
        // we're the next bookmark after a sibling bookmark
        Bookmark *reference_bm;
        Data_Get_Struct(reference, Bookmark, reference_bm);
        doc = reference_bm->doc_wrapper;
        CPDF_BookmarkTree tree(doc->document);
        bm->bookmark = new CPDF_Bookmark( tree.GetNextSibling(*reference_bm->bookmark) );
    } else {
        rb_raise(rb_eArgError, "options must contain either :document, :parent or :sibling");
        return Qnil;
    }

    bm->doc_wrapper = doc;
    doc->retain(bm);

    return Qnil;
}

/*
 * call-seq:
 *   children -> BookmarkList
 *
 * All Bookmarks that are children.  If the Bookmark has no children, an empty list is returned
 */
static VALUE
bookmark_children(VALUE self)
{
    Bookmark *bm;
    Data_Get_Struct(self, Bookmark, bm);

    CPDF_BookmarkTree tree(bm->doc_wrapper->document);
    CPDF_Bookmark child( tree.GetFirstChild(*bm->bookmark) );

    VALUE args[1];

    if (child.GetDict()){
        args[0] = rb_hash_new();
        rb_hash_aset(args[0], ID2SYM(rb_intern("parent")), self);
        args[0] = rb_class_new_instance( 1, args, RB::Bookmark() );
    } else {
        args[0] = Qnil; //rb_class_new_instance( 1, args, T_NIL );
    }
    return rb_class_new_instance( 1, args, RB::BookmarkList() );

}

/*
 * call-seq:
 *   next_sibling -> Bookmark
 *
 * Returns the Bookmark that comes after this one
 */
static VALUE
bookmark_next_sibling(VALUE self)
{
    Bookmark *bm;
    Data_Get_Struct(self, Bookmark, bm);
    CPDF_BookmarkTree tree(bm->doc_wrapper->document);
    CPDF_Bookmark next = tree.GetNextSibling(*bm->bookmark);

    if (next.GetDict()){
        VALUE args[1];
        args[0] = rb_hash_new();
        rb_hash_aset(args[0], ID2SYM(rb_intern("sibling")), self);
        return rb_class_new_instance( 1, args, RB::Bookmark() );
    } else {
        return Qnil;
    }
}


/*
 * call-seq:
 *   title -> String encoded as UTF-16LE
 *
 * Returns the title of the bookmark in UTF-16LE format.
 * This means that the string cannot be directly compared to a ASCII string, and must be converted.
 *
 *    bookmark.title.encode!("ASCII-8BIT")
 *
 */
static VALUE
bookmark_title(VALUE self)
{
    return RB::to_string( RB2BM(self)->GetTitle() );
}

/*
 * call-seq:
 *   destination -> Hash
 *
 * Returns the destination data of the bookmark.
 * Only the destination type is tested.
 * Bug reports and confirmation on the action type is appreciated.
 */
static VALUE
bookmark_destination(VALUE self)
{
    Bookmark *bm;
    Data_Get_Struct(self, Bookmark, bm);
    auto doc =  bm->doc_wrapper->document;
    VALUE hash=rb_hash_new();
    CPDF_Dest dest = bm->bookmark->GetDest( doc );
    if (dest){
        rb_hash_aset(hash, ID2SYM( rb_intern("type") ), ID2SYM(rb_intern("destination")));
        rb_hash_aset(hash, ID2SYM( rb_intern("page_number") ), INT2NUM(dest.GetPageIndex(doc)));
    } else {
        CPDF_Action action = bm->bookmark->GetAction();
        if (action){
            rb_hash_aset(hash, ID2SYM( rb_intern("type") ), ID2SYM(rb_intern("action")));
            rb_hash_aset(hash, ID2SYM( rb_intern("action") ),
                         rb_str_new2( action.GetTypeName().c_str() ) );
            rb_hash_aset(hash, ID2SYM( rb_intern("uri") ),
                         rb_str_new2( action.GetURI(doc).c_str() ) );
        } else {
            rb_hash_aset(hash, ID2SYM( rb_intern("type") ), ID2SYM(rb_intern("unknown")));
        }
    }
    return hash;
}

VALUE
define_bookmark_class(){

#if RDOC_IS_STUPID_AND_CANNOT_PARSE_DOCUMENTATION
    VALUE RB_PDFium = rb_define_module("PDFium");
#endif
    VALUE RB_PDFium = RB::PDFium();

    VALUE RB_Bookmark = rb_define_class_under(RB_PDFium, "Bookmark",  rb_cObject);
    rb_define_alloc_func(RB_Bookmark, bookmark_allocate);

    rb_define_private_method (RB_Bookmark, "initialize",   RUBY_METHOD_FUNC(bookmark_initialize),  1);
    rb_define_method         (RB_Bookmark, "title",        RUBY_METHOD_FUNC(bookmark_title),       0);
    rb_define_method         (RB_Bookmark, "next_sibling", RUBY_METHOD_FUNC(bookmark_next_sibling),0);
    rb_define_method         (RB_Bookmark, "children",     RUBY_METHOD_FUNC(bookmark_children),    0);
    rb_define_method         (RB_Bookmark, "destination",  RUBY_METHOD_FUNC(bookmark_destination), 0);

    return RB_Bookmark;
}
