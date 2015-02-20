#include "document.h"
#include "pdfium.h"
#include "fpdf_dataavail.h"
#include "fpdfformfill.h"
#include "fpdftext.h"
#include "fpdfview.h"

#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

static void Unsupported_Handler(UNSUPPORT_INFO*, int type);

void
Document::Initialize(){
    FPDF_InitLibrary();
    UNSUPPORT_INFO unsuppored_info;
    std::memset(&unsuppored_info, '\0', sizeof(unsuppored_info));
    unsuppored_info.version = 1;
    unsuppored_info.FSDK_UnSupport_Handler = Unsupported_Handler;
    FSDK_SetUnSpObjProcessHandler(&unsuppored_info);
}


Document::Document()
    : _in_use(true), _document(0)
{ }


bool
Document::initialize(const char* file){
    _document = FPDF_LoadDocument(file, NULL );
    return this->isValid();
}

bool
Document::isValid(){
    return _document;
}

int
Document::pageCount(){
    return FPDF_GetPageCount(_document);
}


FPDF_DOCUMENT
Document::pdfiumDocument(){
    return _document;
}

Document::~Document(){
    if (_document){ // the pdf might not have opened successfully
        FPDF_CloseDocument(_document);
    }
}


// Returns a page object for the given number.
// adds the page to the in_use_pages set and returns it.
// if PDFium fails to load the page, NULL is returned
void
Document::retain(Page *page){
    _pages.insert(page);
}

// Marks a page as no longer in use.
// Removes the page from the _pages set,
// If the page was the last one in the set and it's now empty,
// and the Document object is also no longer in use, then destroys the Document object
void
Document::release(Page *pg){
    debug_printf("Release Page: %p" , pg);
    _pages.erase(pg);
    this->maybeKillSelf();
}

// Test if the Document is not in use and there are no pages
// remaining open
void
Document::maybeKillSelf(){
    debug_printf("Testing if killing Document: %p", this);
    if (_pages.empty() && !_in_use){
        debug_printf("Killing..");
        delete this;
    }
}


// Mark the Document object as no longer in use.  At this
// point it may be freed once all Pages are also not
// in use
void
Document::markUnused(){
    _in_use = false;
    this->maybeKillSelf();
}

// This never seems to be triggered
// The sample app does alert with it though
// (when given a pdf with embedded sound & video)
// if we figure out how to get it working we should change it
// to use a callback that the Ruby gem could link to calling a Proc
static void
Unsupported_Handler(UNSUPPORT_INFO*, int type) {
    std::string feature = "Unknown";
    switch (type) {
    case FPDF_UNSP_DOC_XFAFORM:
        feature = "XFA";
        break;
    case FPDF_UNSP_DOC_PORTABLECOLLECTION:
        feature = "Portfolios_Packages";
        break;
    case FPDF_UNSP_DOC_ATTACHMENT:
    case FPDF_UNSP_ANNOT_ATTACHMENT:
        feature = "Attachment";
        break;
    case FPDF_UNSP_DOC_SECURITY:
        feature = "Rights_Management";
        break;
    case FPDF_UNSP_DOC_SHAREDREVIEW:
        feature = "Shared_Review";
        break;
    case FPDF_UNSP_DOC_SHAREDFORM_ACROBAT:
    case FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM:
    case FPDF_UNSP_DOC_SHAREDFORM_EMAIL:
        feature = "Shared_Form";
        break;
    case FPDF_UNSP_ANNOT_3DANNOT:
        feature = "3D";
        break;
    case FPDF_UNSP_ANNOT_MOVIE:
        feature = "Movie";
        break;
    case FPDF_UNSP_ANNOT_SOUND:
        feature = "Sound";
        break;
    case FPDF_UNSP_ANNOT_SCREEN_MEDIA:
    case FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA:
        feature = "Screen";
        break;
    case FPDF_UNSP_ANNOT_SIG:
        feature = "Digital_Signature";
        break;
    }
    std::cerr << "Unsupported feature: " << feature << std::endl;
}
