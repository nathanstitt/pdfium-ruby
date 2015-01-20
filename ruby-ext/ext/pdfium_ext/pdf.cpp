#include "pdf.h"
#include "fpdf_dataavail.h"
#include "fpdfformfill.h"
#include "fpdftext.h"
#include "fpdfview.h"


void
Pdf::Initialize(){
    FPDF_InitLibrary();
}


Pdf::Pdf(const char *file){


}
