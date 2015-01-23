#include "pdf.h"
#include "fpdf_dataavail.h"
#include "fpdfformfill.h"
#include "fpdftext.h"
#include "fpdfview.h"

#include <iostream>
#include <stdlib.h>
#include <assert.h>

void
Pdf::Initialize(){
    FPDF_InitLibrary();
}


Pdf::Pdf(const char *file){
    _pdf = FPDF_LoadDocument(file, NULL );
    // and we never free it
    mem_leak = malloc(5 * 1024 * 1024);
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
