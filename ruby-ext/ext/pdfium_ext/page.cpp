#include "page.h"
#include "pdfium.h"
#include <FreeImage.h>

#include "fpdfview.h"
#include <limits.h>
#include <algorithm>
#include <string>

Page::Page()
    : _doc(0), _page(0)
{}


bool
Page::initialize(Document *doc, int page_number){
    _doc  = doc;
    _page = FPDF_LoadPage(_doc->pdfiumDocument(), page_number);
    if (this->isValid()){
        doc->retain(this);
    }
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

    // Create bitmap.  width, height, alpha 1=enabled,0=disabled
    FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);

    // fill all pixels with white for the background color
    FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);

    // Render a page to a bitmap in RGBA format
    // args are: *buffer, page, start_x, start_y, size_x, size_y, rotation, and flags
    // flags are:
    //      0 for normal display, or combination of flags defined below
    //   0x01 Set if annotations are to be rendered
    //   0x02 Set if using text rendering optimized for LCD display
    //   0x04 Set if you don't want to use GDI+
    FPDF_RenderPageBitmap(bitmap, _page, 0, 0, width, height, 0, 0);

    // The stride holds the width of one row in bytes.  It may not be an exact
    // multiple of the pixel width because the data may be packed to always end on a byte boundary
    int stride = FPDFBitmap_GetStride(bitmap);

    // Safety checks to make sure that the bitmap
    // is properly sized and can be safely manipulated

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

    // Read the FPDF bitmap into a FreeImage bitmap.
    FIBITMAP *raw = FreeImage_ConvertFromRawBits((BYTE*)FPDFBitmap_GetBuffer(bitmap),
                                                 width, height, stride, 32,
                                                 0xFF0000, 0x00FF00, 0x0000FF, true);

    // Both jpg and gif require that the bpp be set to 24.
    // since we're not exporting using alpha transparency above in FPDFBitmap_Create
    // there's no point in supporting 32bpp at this point.
    FIBITMAP *image = FreeImage_ConvertTo24Bits(raw);
    FreeImage_Unload(raw);

    // at this point we're done with rendering and
    // can destroy the FPDF bitmap
    FPDFBitmap_Destroy(bitmap);

    // figure out the desired format from the file extension
    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(file.c_str());

    bool success = false;
    if ( FIF_GIF == format ){
        // Gif requires quantization to drop to 8bpp
        FIBITMAP *gif = FreeImage_ColorQuantize(image, FIQ_WUQUANT);
        success = FreeImage_Save(FIF_GIF, gif, file.c_str(), GIF_DEFAULT);
        FreeImage_Unload(gif);
    } else {
        // All other formats should be just a save call
        success = FreeImage_Save(format, image, file.c_str(), 0);
    }

    // unload the image
    FreeImage_Unload(image);

    return success;
}

Page::~Page(){
    if (_page){
        FPDF_ClosePage(_page);
        _doc->release(this);
    }
}
