#ifndef __PAGE_WRAPPER_H__
#define __PAGE_WRAPPER_H__

#include "pdfium.h"
#include <unordered_set>

class DocumentWrapper;

class PageWrapper {

  public:

    PageWrapper(DocumentWrapper* doc, int page);
    void wrap(CPDF_Page *page, DocumentWrapper *doc_wrapper);
    ~PageWrapper();

    void markUnused();

    void retain(void *obj);
    void release(void *obj);

    CPDF_Page *page();
    void unload();
    void setPage(CPDF_Page *pg);

    DocumentWrapper *document_wrapper;
    int _page_number;
  private:

    bool _in_use;
    CPDF_Page *_page;
    std::unordered_set<void*> _children;
    void maybeKillSelf();
};


#endif // __PAGE_WRAPPER_H__
