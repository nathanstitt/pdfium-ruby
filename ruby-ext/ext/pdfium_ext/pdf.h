#ifndef __PDF_H__
#define __PDF_H__

#include <iostream>
#include "fpdf_ext.h"
#include "pdf.h"
#include "page.h"
#include <unordered_set>


// https://redmine.ruby-lang.org/issues/6292

// Ruby will call dispose of the Page and PDF objects in whatever order it
// wishes to.  This is problematic for PDFium.  If a Pdf object is closed whie
// pages are still open, and then the Pages are closed later, it will segfault.
//
// To work around this, the Pdf keeps a reference to all open Pages.  When a Page
// is deleted, it's destructor calls releasePage on the Pdf object.
//
// It does this so that it can keep track of all the Page objects that are in use
// and only release it's memory and close it's FPDF_DOCUMENT once all the pages
// are no longer used.
//
// It's reasonably safe to do so. Since we're only use the Pdf/Page classes
// from Ruby and control how they're called.
//
// A future improvement would be to write a custom smart pointer supervisor class
// to manage the interplay between the Pdf and Page objects
//
// Beware! As a side affect of the above, this class calls "delete this" on itself.
// Therefore it must be allocated on the heap (i.e. "new Pdf"),
// and not as part of an array (not new[]).
//

class Pdf {
  public:
    static void Initialize();

    Pdf();

    bool initialize(const char* file);

    bool initializePage(Page *page, int page_index);

    bool isValid();

    int pageCount();

    Page* getPage(int page_index);
    void releasePage(Page*);

    void markUnused();

    ~Pdf();

  private:
    std::unordered_set<Page*> _in_use_pages;
    bool _in_use;
    FPDF_DOCUMENT _pdf;
    void maybeKillSelf();

};


#endif // __PDF_H__
