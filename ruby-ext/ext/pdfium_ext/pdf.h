#include <iostream>
#include "fpdf_ext.h"

class Pdf {
 public:
    static void Initialize();

    Pdf(const char* file);


  private:
    FPDF_DOCUMENT _pdf;

};
