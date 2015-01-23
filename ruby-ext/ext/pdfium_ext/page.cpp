#include "page.h"

Page::Page(Pdf *pdf, int page_index){
    _page = FPDF_LoadPage(pdf, page_index);
}

bool
Page::isValid(){
    return _page;
}

double
Page::width(){
    return FPDF_GetPageHeight(_page);
}

double
Page::height(){
    return FPDF_GetPageWidth(_page);
}

bool
Page::render(FPDF_BITMAP bitmap){
    return false;
}

Page::~Page(){
    if (_page){ // the page might not have opened successfully
        FPDF_ClosePage(_page);
    }
}
