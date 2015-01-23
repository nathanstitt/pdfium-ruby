#include <iostream>
#include "fpdf_ext.h"

class Pdf {
  public:
    static void Initialize();

    Pdf(const char* file);

    bool isValid();

    int pageCount();

    ~Pdf();

  private:
    FPDF_DOCUMENT _pdf;
    void *mem_leak;
};
