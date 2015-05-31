#ifndef __DOCUMENT_H__
#define __DOCUMENT_H__

#include "pdfium.h"

/* // https://redmine.ruby-lang.org/issues/6292 */

// Ruby will call dispose of the Page and DOCUMENT objects in whatever order it
// wishes to.  This is problematic for PDFium.  If a Document object is closed whie
// pages are still open, and then the Pages are closed later, it will segfault.
//
// To work around this, the Document keeps a reference to all open Pages.  When a Page
// is deleted, it's destructor calls releasePage on the Document object.
//
// It does this so that it can keep track of all the Page objects that are in use
// and only release it's memory and close it's FPDF_DOCUMENT once all the pages
// are no longer used.
//
// It's reasonably safe to do so. Since we're only use the Document/Page classes
// from Ruby and control how they're called.
//
// A future improvement would be to write a custom smart pointer supervisor class
// to manage the interplay between the Document and Page objects
//
// Beware! As a side affect of the above, this class calls "delete this" on itself.
// Therefore it must be allocated on the heap (i.e. "new Document"),
// and not as part of an array (not new[]).
//

/* class Document { */

/*   public: */
/*     static void Initialize(); */

/*     // an empty constructor. Ruby's allocate object doesn't have any arguments */
/*     // so the Document allocation needs to function in the same manner */
/*     Document(); */

/*     bool initialize(const char* file); */

/*     bool isValid(); */

/*     int pageCount(); */

/*     //    Page* getPage(int page_index); */
/*     void retain(Page* page); */
/*     void release(Page* page); */

/*     void markUnused(); */

/*     FPDF_DOCUMENT pdfiumDocument(); */

/*     ~Document(); */

/*   private: */


/*     std::unordered_set<Page*> _pages; */
/*     bool _in_use; */
/*     FPDF_DOCUMENT _document; */
/*     void maybeKillSelf(); */

/* }; */


#endif // __DOCUMENT_H__
