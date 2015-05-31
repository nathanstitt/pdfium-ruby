#include "pdfium.h"
#include "page_wrapper.h"

PageObjectWrapper::PageObjectWrapper():
    page_wrapper(0),
    object(0),
    page_object_index(-1)
{}

PageObjectWrapper::~PageObjectWrapper(){
    if (page_wrapper)
        page_wrapper->release(this);
    if (object){
        //        object->Release();
    }
}

void
PageObjectWrapper::wrap(CPDF_PageObject *obj, PageWrapper *pg){
    this->object  = obj;
    this->page_wrapper = pg;
    this->page_wrapper->retain(this);
}



CPDF_ImageObject*
RB2IMG(VALUE self) {
  PageObjectWrapper* po;
  Data_Get_Struct(self, PageObjectWrapper, po);
  return static_cast<CPDF_ImageObject*>(po->object);
}

void
ImageWrapper::wrap(PageWrapper *page_wrapper){
    this->page_wrapper = page_wrapper;
    this->page_wrapper->retain(this);
}
