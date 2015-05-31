#include "document_wrapper.h"


DocumentWrapper::DocumentWrapper()
    : document(0), _in_use(true)
{ }

// Mark the Document object as no longer in use.  At this
// point it may be freed once all Pages are also not
// in use
void
DocumentWrapper::markUnused(){
    _in_use = false;
    this->maybeKillSelf();
}


// a utility method to extract the reference to the FPDF_DOCUMENT from the Ruby/C++ wrapping
CPDF_Document*
RB2DOC(VALUE self) {
  DocumentWrapper* doc;
  Data_Get_Struct(self, DocumentWrapper, doc);
  return doc->document;
}


// Retains a copy of the page, which will prevent
// the Document from being destroyed until the release()
// is called for the page
void
DocumentWrapper::retain(void *child){
    _children.insert(child);
}

// Marks a page as no longer in use.
// Removes the page from the _pages set,
// If the page was the last one in the set and it's now empty,
// and the Document object is also no longer in use, then destroys the Document object
void
DocumentWrapper::release(void *child){
    DEBUG_MSG("Release Doc Child: " << child);
    _children.erase(child);
    this->maybeKillSelf();
}


// Test if the Document is not in use and there are no pages
// that are still retained
void
DocumentWrapper::maybeKillSelf(){
    DEBUG_MSG("Testing if killing Document: " << this);
    if (_children.empty() && !_in_use){
        DEBUG_MSG("Killing..");
        delete this;
    }
}


DocumentWrapper::~DocumentWrapper(){
    if (document){ // the pdf might not have opened successfully
        FPDF_CloseDocument(document);
    }
}
