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
    _page_number = page_number;
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
    return FPDF_GetPageWidth(_page);
}


double
Page::height(){
    return FPDF_GetPageHeight(_page);
}

int
Page::number(){
    return _page_number;
}

// Render the page to a FreeImage bitmap
// the caller is responsible for calling
// FreeImage_Unload on the bitmap when it's no
// longer in use.
// Does not draw forms or annotations on bitmap

FIBITMAP *
Page::renderToBitmap(int width, int height){
    if (! this->calculate_default_sizes(width, height)){
        return NULL;
    }
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
        return NULL;
    }
    if (height > 0 && width > INT_MAX / height){
        FPDFBitmap_Destroy(bitmap);
        return NULL;
    }
    int out_len = stride * height;
    if (out_len > INT_MAX / 3){
        FPDFBitmap_Destroy(bitmap);
        return NULL;
    }

    // Read the FPDF bitmap into a FreeImage bitmap.
    FIBITMAP *img32 = FreeImage_ConvertFromRawBits((BYTE*)FPDFBitmap_GetBuffer(bitmap),
                                        width, height, stride, 32,
                                        0xFF0000, 0x00FF00, 0x0000FF, true);

    // Both jpg and gif require that the bpp be set to 24.
    // since we're not exporting using alpha transparency above in FPDFBitmap_Create
    // there's no point in supporting 32bpp at this point.
    FIBITMAP *image = FreeImage_ConvertTo24Bits(img32);
    FreeImage_Unload(img32);

    // at this point we're done with rendering and
    // can destroy the FPDF bitmap
    FPDFBitmap_Destroy(bitmap);

    return image;

}


bool
Page::saveAndUnloadBitmap(FIBITMAP *bmp, const std::string &file){
    // figure out the desired format from the file extension
    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(file.c_str());

    bool success = false;
    if ( FIF_GIF == format ){
        // Gif requires quantization to drop to 8bpp
        FIBITMAP *gif = FreeImage_ColorQuantize(bmp, FIQ_WUQUANT);
        success = FreeImage_Save(FIF_GIF, gif, file.c_str(), GIF_DEFAULT);
        FreeImage_Unload(gif);
    } else {
        // All other formats should be just a save call
        success = FreeImage_Save(format, bmp, file.c_str(), 0);
    }

    // unload the image
    FreeImage_Unload(bmp);

    return success;
}


bool
Page::render(const std::string &file, const sizes_t &sizes){
    if (sizes.empty()){
        return false;
    }
    std::string::size_type width_pos  = file.find("%w", 0, 2);
    std::string::size_type height_pos = file.find("%h", 0, 2);
    if (width_pos == std::string::npos || height_pos == std::string::npos ){
        return false;
    }
    for (sizes_t::const_iterator size = sizes.begin(); size != sizes.end(); ++size){
        int width = size->first;
        int height = size->second;
        if (!this->calculate_default_sizes(width, height)){
            return false;
        }

        std::string dest(file); // copy the file in prep for replacing sizes
        // replace the width format "%w"
        dest.replace(width_pos, 2, std::to_string(width) );

        // and then the height "%h".  We need to re-find the formatter position
        // since the above replacement has altered the string and it's position
        height_pos = dest.find("%h", 0, 2);
        dest.replace(height_pos, 2, std::to_string(height) );
        FIBITMAP *image = this->renderToBitmap(width,height);
        this->saveAndUnloadBitmap(image, dest);
    }
    return true;
}


bool
Page::render_resize(const std::string &file, const sizes_t &sizes){
    if (sizes.empty()){
        return false;
    }
    sizes_t::value_type lg_size = sizes.front();
    FIBITMAP *tmpl_image = this->renderToBitmap(lg_size.first, lg_size.second);
    if (!tmpl_image){
        return false;
    }
    std::string::size_type width_pos  = file.find("%w", 0, 2);
    std::string::size_type height_pos = file.find("%h", 0, 2);
    if (width_pos == std::string::npos || height_pos == std::string::npos ){
        FreeImage_Unload(tmpl_image);
        return false;
    }
    for (sizes_t::const_iterator size = sizes.begin(); size != sizes.end(); ++size){
        int width = size->first;
        int height = size->second;
        if (!this->calculate_default_sizes(width, height)){
            return false;
        }

        // The section in the FreeImage manual titled "Choosing the right resampling filter"
        // recommends Catmull-Rom filter.
        // http://www.imagemagick.org/Usage/filter/ also has good info

        FIBITMAP *smaller = FreeImage_Rescale(tmpl_image, width, height, FILTER_CATMULLROM);
        if (!smaller){
            FreeImage_Unload(tmpl_image);
            return false;
        }
        std::string dest(file); // copy the file in prep for replacing sizes
        // replace the width format "%w"
        dest.replace(width_pos, 2, std::to_string(width) );

        // and then the height "%h".  We need to re-find the formatter position
        // since the above replacement has altered the string and it's position
        int height_pos = dest.find("%h", 0, 2);
        dest.replace(height_pos, 2, std::to_string(height) );

        // and then export the image
        if (!this->saveAndUnloadBitmap(smaller, dest)){
            FreeImage_Unload(tmpl_image);
            return false;
        }
    }
    FreeImage_Unload(tmpl_image);
    return true;
}

// render the page to a file
bool
Page::render(const std::string &file, int width, int height){
    FIBITMAP *image = this->renderToBitmap(width, height);
    if (!image){
        return false;
    }
    return this->saveAndUnloadBitmap(image, file);
}

bool
Page::calculate_default_sizes(int &width, int &height){
    if (!width && !height){
        return false;
    }
    if (!width){
        width = ((double)this->width()) * ( (double)height / this->height() );
    }
    if (!height){
        height = ((double)this->height()) * ( (double)width / this->width() );
    }
    return true;
}


Page::~Page(){
    if (_page){
        FPDF_ClosePage(_page);
        _doc->release(this);
    }
}
