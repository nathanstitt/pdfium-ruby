#include <iostream>
#include "fpdf_ext.h"
#include "pdf.h"

class Page {
  public:
    Page(Pdf *pdf, int page_index);

    bool isValid();

    double width();

    double height();

    bool render(FPDF_BITMAP bitmap);

    ~Page();

  private:
    FPDF_PAGE _page;
};
