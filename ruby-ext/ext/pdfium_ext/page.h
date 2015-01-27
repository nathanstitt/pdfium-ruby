#include <iostream>
#include "fpdf_ext.h"
#include "pdf.h"

#ifndef __PAGE_H__
#define __PAGE_H__

class Pdf;

class Page {
    friend class Pdf;
  public:
    Page();

    bool initialize(Pdf *pdf, int page_index);

    bool isValid();

    double width();

    double height();

    bool render(const std::string &file, int width, int height);

    ~Page();

  private:
    int page_number;
    Pdf *_pdf;
    FPDF_PAGE _page;
};


#endif // __PAGE_H__
