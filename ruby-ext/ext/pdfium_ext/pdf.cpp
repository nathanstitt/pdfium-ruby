#include "pdf.h"
#include "fpdf_dataavail.h"
#include "fpdfformfill.h"
#include "fpdftext.h"
#include "fpdfview.h"

#include <iostream>
void
Pdf::Initialize(){
    FPDF_InitLibrary();
}


Pdf::Pdf(const char *file){
    _pdf =FPDF_LoadDocument(file, NULL );
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
    FPDF_CloseDocument(_pdf);
    std::cout << "closed doc" <<std::endl;
}
