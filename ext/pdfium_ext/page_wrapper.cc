#include "page_wrapper.h"
#include "pdfium.h"

CPDF_Page*
RB2PG(VALUE self){
    PageWrapper *page;
    Data_Get_Struct(self, PageWrapper, page);
    return page->page();
}

PageWrapper::PageWrapper(DocumentWrapper* doc, int page) :
    document_wrapper(doc), _page_number(page), _in_use(true), _page(NULL)
{
    this->document_wrapper->retain(this);
}

void
PageWrapper::unload(){
    FPDF_ClosePage(_page);
    _page = NULL;
}


// Mark the page object as no longer in use.  At this
// point it may be freed once all children are also not
// in use
void
PageWrapper::markUnused(){
    _in_use = false;
    this->unload();
    this->maybeKillSelf();
}

void
PageWrapper::setPage(CPDF_Page *page){
    if (_page){
        this->unload();
    }
    // unload won't work if the page has children
    if (!_page){
        this->_page=page;
    }
}

void
PageWrapper::retain(void *obj){
    _children.insert(obj);
}

CPDF_Page *
PageWrapper::page(){
    if (!_page){
        _page = static_cast<CPDF_Page *>(FPDF_LoadPage(document_wrapper->document, _page_number));
    }
    return _page;
}

void
PageWrapper::release(void *obj){
    _children.erase(obj);
    this->maybeKillSelf();

}

// Test if the Document is not in use and there are no pages
// that are still retained
void
PageWrapper::maybeKillSelf(){
    bool killable = _children.empty() && !_in_use;
    DEBUG_MSG("Testing if killing Page: " << this << " " << killable );
    if (killable){
        delete this;
    }
}

// void
// PageWrapper::wrap(CPDF_Page *pg, DocumentWrapper *doc_wrapper){
//     this->page = pg;
//     this->doc  = doc_wrapper;
//     this->doc->retain(this);
// }


PageWrapper::~PageWrapper(){
    this->unload();
}
