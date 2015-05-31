#ifndef __PAGE_OBJECT_WRAPPER_H__
#define __PAGE_OBJECT_WRAPPER_H__

class PageWrapper;

class PageObjectWrapper {
  public:
    PageObjectWrapper();
    ~PageObjectWrapper();

    void wrap(CPDF_PageObject *object, PageWrapper *page_wrapper);

    PageWrapper *page_wrapper;
    CPDF_PageObject *object;
    int page_object_index;
};


class ImageWrapper : public PageObjectWrapper {
  public:

    void wrap(PageWrapper *page_wrapper);

};


#endif // __PAGE_OBJECT_WRAPPER_H__
