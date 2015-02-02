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

    // same as above, except render the page to multiple sizes
    // "file" must have %w and %h placeholders
    // For best quality, the largest size should be give first
    bool render(const std::string &file, const sizes_t &sizes);

    // render to the first size, then use freeimage to scale the page down
    // to the remaining sizes
    bool render_resize(const std::string &file, const sizes_t &sizes);

    ~Page();

  private:
    Document *_doc;
    int _page_number;
    // Render the page to a FreeImage bitmap.
    // The caller is responsible for calling FreeImage_Unload on the bitmap when it's no
    // longer in use.
    // Does not draw forms or annotations on bitmap
    // will return NULL if render fails
    FIBITMAP *renderToBitmap(int width, int height);

    bool saveAndUnloadBitmap(FIBITMAP *bmp, const std::string &file);

    // these check if value is 0 and if so scale the
    // missing sizes to match.
    // Returns false if both width & height are missing
    bool calculate_default_sizes(int &width, int &height);


    FPDF_PAGE _page;
};


#endif // __PAGE_H__
