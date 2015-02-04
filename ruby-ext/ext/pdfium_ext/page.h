#include <iostream>
#include "fpdf_ext.h"
#include "document.h"
#include <list>
#ifndef __PAGE_H__
#define __PAGE_H__

class Document;
class FIBITMAP;

class Page {

  public:
    typedef std::list< std::pair<int,int> > sizes_t;

    Page();

    bool initialize(Document *doc, int page_index);

    // did the page load successfully?
    bool isValid();

    // return the "native" height/width of the page
    // the dimensions are in points, which are usually 72 per inch
    double width();
    double height();

    int number();

    // render page to file with width/height.  If either width or height is 0
    // the page will be scaled to match the missing size
    bool render(const std::string &file, int width, int height);

    ~Page();

  private:
    Document *_doc;
    int _page_number;

    FPDF_PAGE _page;
};


#endif // __PAGE_H__
