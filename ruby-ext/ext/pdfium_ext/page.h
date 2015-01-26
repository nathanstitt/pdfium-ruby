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

    void initialize(Pdf *pdf, FPDF_PAGE page);

    double width();

    double height();

    bool render(const std::string &file, int width, int height);

    ~Page();

  private:
    Pdf *_pdf;
    FPDF_PAGE _page;
};


#endif // __PAGE_H__
