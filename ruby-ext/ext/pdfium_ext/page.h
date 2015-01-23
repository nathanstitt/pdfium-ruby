#include <iostream>
#include "fpdf_ext.h"
#include "pdf.h"

#ifndef __PAGE_H__
#define __PAGE_H__

class Page {
  public:
    Page();

    bool initialize(Pdf *pdf, int page_index);

    bool isValid();

    double width();

    double height();

    bool render(const std::string &file, int width, int height);

    ~Page();

  private:
    FPDF_PAGE _page;
};


#endif // __PAGE_H__
