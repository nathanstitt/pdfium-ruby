#include "pdf.h"
#include "fpdf_dataavail.h"
#include "fpdfformfill.h"
#include "fpdftext.h"
#include "fpdfview.h"

#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <assert.h>

static void Unsupported_Handler(UNSUPPORT_INFO*, int type);

void
Pdf::Initialize(){
    FPDF_InitLibrary();
    UNSUPPORT_INFO unsuppored_info;
    std::memset(&unsuppored_info, '\0', sizeof(unsuppored_info));
    unsuppored_info.version = 1;
    unsuppored_info.FSDK_UnSupport_Handler = Unsupported_Handler;
    FSDK_SetUnSpObjProcessHandler(&unsuppored_info);
}


Pdf::Pdf()
    : _pdf(0)
{ }


bool
Pdf::initialize(const char* file){
    _pdf = FPDF_LoadDocument(file, NULL );
    return this->isValid();
}

bool
Pdf::isValid(){
    return _pdf;
}


int
Pdf::pageCount(){
    return FPDF_GetPageCount(_pdf);
}


Pdf::~Pdf(){
    if (_pdf){ // the pdf might not have opened successfully
        FPDF_CloseDocument(_pdf);
    }
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
