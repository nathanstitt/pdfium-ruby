#include <iostream>
#include "fpdf_ext.h"
#include "document.h"

#ifndef __PAGE_H__
#define __PAGE_H__

class Document;

class Page {

  public:
    Page();

    bool initialize(Document *doc, int page_index);

    bool isValid();

    double width();

    double height();

    bool render(const std::string &file, int width, int height);

    ~Page();

  private:
    Document *_doc;
    FPDF_PAGE _page;
};


#endif // __PAGE_H__
