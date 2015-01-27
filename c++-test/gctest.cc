#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <string>
#include <utility>
#include <iostream>
#include <iostream>

#include "fpdf_dataavail.h"
#include "fpdf_ext.h"
#include "fpdfformfill.h"
#include "fpdftext.h"
#include "fpdfview.h"
#include <FreeImage.h>

void renderPage(FPDF_PAGE page, int width = 180, int height = 250 ){
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
    FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);

    // The stride holds the width of one row in bytes.  It may not be an exact
    // multiple of the pixel width because the data may be packed to always end on a byte boundary
    int stride = FPDFBitmap_GetStride(bitmap);

    // Read the FPDF bitmap into a FreeImage bitmap.
    FIBITMAP *raw = FreeImage_ConvertFromRawBits((BYTE*)FPDFBitmap_GetBuffer(bitmap),
                                                 width, height, stride, 32,
                                                 0xFF0000, 0x00FF00, 0x0000FF, true);

    // saving de-activated because I don't want to figure have
    // to calculate or specify a save file
    // FreeImage_Save(FIF_PNG, image, file.c_str(), 0);

    FreeImage_Unload(raw);

    FPDFBitmap_Destroy(bitmap);
}

int main(int argc, const char* argv[]) {
    FPDF_InitLibrary();
    if (argc != 2){
        std::cerr << "usage: " << argv[0] << " <input pdf>" << std::endl;
    }
    std::cout << "press enter to start run" << std::endl;
    std::cin.ignore();

    FPDF_DOCUMENT doc = FPDF_LoadDocument(argv[1], NULL );
    if (!doc){
        std::cerr << "failed to parse document: " << argv[1] << std::endl;
        return 1;
    }

    std::list<FPDF_PAGE> pages;
    // limit pages to 50 just 'cuz I'm impatient
    int max_pages = std::min(FPDF_GetPageCount(doc), 50 );
    for (int pg=0; pg < max_pages; pg++){
        FPDF_PAGE page = FPDF_LoadPage(doc, pg );
        if (!page){
            std::cerr << "failed to open page: " << pg << std::endl;
            return 1;
        }
        std::cout << "Rendered page " << pg << std::endl;
        renderPage(page);
        pages.push_back(page);
    }


    // https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man3/libgmalloc.3.html
    // http://www.cimgf.com/2008/04/02/cocoa-tutorial-fixing-memory-leaks-with-instruments/

    // in this order, all is well
    // for (auto page : pages)
    //     FPDF_ClosePage(page);
    // FPDF_CloseDocument(doc);

    // This doesn't always segfault, but it drives valgrind
    // and XCode's "instruments" memory tool crazy
    FPDF_CloseDocument(doc);
    for (auto page : pages)
        FPDF_ClosePage(page);


}
