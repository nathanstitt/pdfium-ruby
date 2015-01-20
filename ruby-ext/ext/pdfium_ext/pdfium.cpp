#include <stdlib.h>
#include <inttypes.h>
extern "C" {
#include "ruby.h"
}
#include "pdf.h"

#include <string>

#include "fpdf_dataavail.h"
#include "fpdf_ext.h"
#include "fpdfformfill.h"
#include "fpdftext.h"
#include "fpdfview.h"

static ID to_s;
static VALUE rb_pdf;

void Unsupported_Handler(UNSUPPORT_INFO*, int type) {
  std::string feature = "Unknown";
  switch (type) {
    case FPDF_UNSP_DOC_XFAFORM:
      feature = "XFA";
      break;
    case FPDF_UNSP_DOC_PORTABLECOLLECTION:
      feature = "Portfolios_Packages";
      break;
    case FPDF_UNSP_DOC_ATTACHMENT:
    case FPDF_UNSP_ANNOT_ATTACHMENT:
      feature = "Attachment";
      break;
    case FPDF_UNSP_DOC_SECURITY:
      feature = "Rights_Management";
      break;
    case FPDF_UNSP_DOC_SHAREDREVIEW:
      feature = "Shared_Review";
      break;
    case FPDF_UNSP_DOC_SHAREDFORM_ACROBAT:
    case FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM:
    case FPDF_UNSP_DOC_SHAREDFORM_EMAIL:
      feature = "Shared_Form";
      break;
    case FPDF_UNSP_ANNOT_3DANNOT:
      feature = "3D";
      break;
    case FPDF_UNSP_ANNOT_MOVIE:
      feature = "Movie";
      break;
    case FPDF_UNSP_ANNOT_SOUND:
      feature = "Sound";
      break;
    case FPDF_UNSP_ANNOT_SCREEN_MEDIA:
    case FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA:
      feature = "Screen";
      break;
    case FPDF_UNSP_ANNOT_SIG:
      feature = "Digital_Signature";
      break;
  }
  printf("Unsupported feature: %s.\n", feature.c_str());
}



void
pdf_gc_free(Pdf *pdf) {
    printf("GC Free PDF: %016" PRIxPTR "\n", (uintptr_t)pdf);

}

void
pdf_gc_mark(Pdf *pdf){
    printf("GC Mark PDF: %016" PRIxPTR "\n", (uintptr_t)pdf);
}


extern "C"
VALUE
pdf_open(VALUE klass, VALUE path){
    path = rb_funcall(path, to_s, 0); // call to_s in case the path is a Pathname or something

    Pdf *pdf = new Pdf(StringValuePtr(path));
    printf("Created PDF: %016" PRIxPTR "\n", (uintptr_t)pdf);

    VALUE self = Data_Wrap_Struct( rb_pdf, &pdf_gc_mark, &pdf_gc_free, pdf );
    return self;
}

extern "C"
void Init_pdfium_ext()
{
    Pdf::Initialize();
    to_s = rb_intern ("to_s");

    UNSUPPORT_INFO unsuppored_info;
    memset(&unsuppored_info, '\0', sizeof(unsuppored_info));
    unsuppored_info.version = 1;
    unsuppored_info.FSDK_UnSupport_Handler = Unsupported_Handler;
    FSDK_SetUnSpObjProcessHandler(&unsuppored_info);

    // Get symbol for PDFium
    ID sym_pdfium = rb_intern("PDFium");
    // Get the module
    VALUE rb_pdfium_module = rb_const_get(rb_cObject, sym_pdfium);

    VALUE rb_pdf = rb_define_class_under(rb_pdfium_module,"Pdf",  rb_cObject);

    rb_define_singleton_method(rb_pdf, "open",
                               reinterpret_cast<VALUE(*)(...)>(pdf_open),1);
}
