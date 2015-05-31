#ifndef __DOCUMENT_WRAPPER_H__
#define __DOCUMENT_WRAPPER_H__
extern "C" {
#include "ruby.h"
}

#include "pdfium.h"
#include "fpdf_ext.h"
#include <unordered_set>
/*
  +---------------------------------------------------------------------------------------------+
  |                                                                                             |
  | This is a lightweight wrapper that mediates between                                         |
  | CPDF_Document and all the other types that depend on it                                     |
  |                                                                                             |
  | Ruby will dispose of the Document and it's child objects objects in whatever order it       |
  | wishes to.  This is problematic for PDFium.  For instance, if a Document object             |
  | is closed whie pages are still open, and then the Pages are closed later, it will segfault. |
  |                                                                                             |
  | To work around this, when a dependent object is created, it calls retain on                 |
  | the DocumentWrapper. Then when Ruby garbage collects a dependent object,                    |
  | it's destructor calls release on the Document object.                                       |
  |                                                                                             |
  | When Ruby GC's the DocumentWrapper itself, it checks to see if any objects are still        |
  | retained.  If there are, it does not delete itself until they are all removed.              |
  |                                                                                             |
  | Beware! As a side affect of the above, this class calls "delete this" on itself.            |
  | Therefore it must be allocated on the heap.  (i.e. "new DocumentWrapper"),                  |
  | and not as part of an array (not new[]).                                                    |
  +---------------------------------------------------------------------------------------------+
*/

class DocumentWrapper {

  public:
    DocumentWrapper();

    void retain(void *child);
    void release(void *child);

    void markUnused();

    ~DocumentWrapper();

    CPDF_Document *document;

  private:

    bool _in_use;
    void maybeKillSelf();

    std::unordered_set<void*> _children;

};

#endif // __DOCUMENT_WRAPPER_H__
