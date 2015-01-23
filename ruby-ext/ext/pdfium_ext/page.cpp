#include "page.h"
#include "fpdfview.h"
#include "bitmap.hpp"
#include <limits.h>
Page::Page()
    : _page(0)
{}

bool
Page::initialize(Pdf *pdf, int page_index){
    _page = FPDF_LoadPage(pdf->_pdf, page_index);

    return this->isValid();
}

bool
Page::isValid(){
    return _page;
}

double
Page::width(){
    return FPDF_GetPageHeight(_page);
}

double
Page::height(){
    return FPDF_GetPageWidth(_page);
}


// N.B.  Does not draw forms or annotations on bitmap
bool
Page::render(const std::string &file, int width, int height){
    FPDF_BITMAP bitmap;
    // Create bitmap.  width, height, alpha 1=enabled,0=disabled
    bitmap = FPDFBitmap_Create(width, height, 0);

    // fill all pixles with white for the background color
    FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);

    FPDF_RenderPageBitmap(bitmap, _page, 0, 0, width, height, 0, 0);
    int stride = FPDFBitmap_GetStride(bitmap);

    if (stride < 0 || width < 0 || height < 0){
        FPDFBitmap_Destroy(bitmap);
        return false;
    }
    if (height > 0 && width > INT_MAX / height){
        FPDFBitmap_Destroy(bitmap);
        return false;
    }
    int out_len = stride * height;
    if (out_len > INT_MAX / 3){
        FPDFBitmap_Destroy(bitmap);
        return false;
    }

    const char* buffer = reinterpret_cast<const char*>(FPDFBitmap_GetBuffer(bitmap));

    bitmap_image bmp;

    bmp.setwidth_height(width, height,true);
    for (int h = 0; h < height; ++h) {
        const char* src_line = buffer + (stride * h);
        for (int w = 0; w < width; ++w) {
            bmp.set_pixel(w, h,
                          src_line[(w * 4) + 2],
                          src_line[(w * 4) + 1],
                          src_line[w * 4]);
        }
    }
    FPDFBitmap_Destroy(bitmap);

    std::ofstream stream(file.c_str(),std::ios::binary);
    if (!stream){
        return false;
    }
    bmp.save_image( stream );
    stream.close();

    return true;
}

Page::~Page(){
    if (_page){ // the page might not have opened successfully
        FPDF_ClosePage(_page);
    }
}
