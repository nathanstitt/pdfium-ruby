#ifndef __PDF_H__
#define __PDF_H__

#include <iostream>
#include "fpdf_ext.h"
#include "pdf.h"

class Pdf {
    friend class Page;
  public:
    static void Initialize();

    Pdf();

    bool initialize(const char* file);

    bool isValid();

    int pageCount();

    ~Pdf();

  private:
    FPDF_DOCUMENT _pdf;

};


#endif // __PDF_H__
