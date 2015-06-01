#include "pdfium.h"
#include <cstdint>
#include <cstring>

/////////////////////////////////////////////////////////////////////////
// The Image class
/////////////////////////////////////////////////////////////////////////
/*
    * Document-class:  PDFium::Image
    *
    * A Image can represent either a Page that
    * has been rendered to a Image via {PDFium::Page#as_image}
    *
    * Or an embedded image on a {PDFium::Page}, obtained via {PDFium::Page#images}
*/


static void
image_gc_free(ImageWrapper *img) {
    delete img;
}

static VALUE
image_allocate(VALUE klass) {
    auto img = new ImageWrapper;
    return Data_Wrap_Struct(klass, NULL, image_gc_free, img );
}

/*
 * call-seq:
 *   Image.new -> Image
 *
 * Initializes an image
 */
VALUE
image_initialize(int argc, VALUE *argv, VALUE self){

    VALUE rb_page, rb_options;
    rb_scan_args(argc,argv,"1:", &rb_page, &rb_options);

    ImageWrapper *img;
    Data_Get_Struct(self, ImageWrapper, img);

    PageWrapper *pg;
    Data_Get_Struct(rb_page, PageWrapper, pg);
    img->wrap(pg);

    if (NIL_P(rb_options)){
        rb_options=rb_hash_new();
    }
    VALUE rb_width = RB::get_option(rb_options, "width");
    if (!NIL_P(rb_width)){
        rb_iv_set(self, "@width", rb_width);
    }
    VALUE rb_height = RB::get_option(rb_options, "height");
    if (!NIL_P(rb_height)){
        rb_iv_set(self, "@height", rb_height);
    }

    VALUE rb_bounds = RB::get_option(rb_options, "bounds");
    VALUE rb_page_index = RB::get_option(rb_options, "index");
    if (!NIL_P(rb_page_index)){
        rb_iv_set(self, "@index", rb_page_index);
    }
    VALUE pg_object_index = RB::get_option(rb_options, "object_index");
    if (NIL_P(rb_bounds) && NIL_P(pg_object_index)){
        rb_raise(rb_eArgError, ":bounds or :object_index must be given");
    }
    if (!NIL_P(pg_object_index)){
        img->page_object_index = FIX2INT(pg_object_index);
    }
    if (NIL_P(rb_bounds)){
        CPDF_ImageObject *image = (CPDF_ImageObject*)pg->
            page()->GetObjectByIndex(img->page_object_index);


        VALUE bounds_args[4];
        bounds_args[0] = rb_float_new( 0 );
        bounds_args[1] = rb_float_new( image->m_pImage->GetPixelWidth() );
        bounds_args[2] = rb_float_new( 0 );
        bounds_args[3] = rb_float_new( image->m_pImage->GetPixelHeight() );
        rb_bounds = rb_class_new_instance( 4, bounds_args, RB::BoundingBox() );
        if (NIL_P(rb_height)){
            rb_iv_set(self, "@height", INT2FIX(image->m_pImage->GetPixelHeight()) );
        }
        if (NIL_P(rb_width)){
            rb_iv_set(self, "@width", INT2FIX(image->m_pImage->GetPixelWidth()) );
        }

    }
    rb_iv_set(self, "@bounds", rb_bounds);

    return Qnil;
}

FIBITMAP*
render_page(CPDF_Page *page, FREE_IMAGE_FORMAT format, int width, int height){
    // Create bitmap.  width, height, alpha 1=enabled,0=disabled
    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(width, height, FPDFBitmap_BGR, NULL, 0);

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

    // Safety checks to make sure that the bitmap
    // is properly sized and can be safely manipulated
    if (stride < 0){
        FPDFBitmap_Destroy(bitmap);
        return NULL;
    }
    if (width > INT_MAX / height){
        FPDFBitmap_Destroy(bitmap);
        return NULL;
    }
    int out_len = stride * height;
    if (out_len > INT_MAX / 3){
        FPDFBitmap_Destroy(bitmap);
        return NULL;
    }

    FIBITMAP *image = FreeImage_ConvertFromRawBits((BYTE*)FPDFBitmap_GetBuffer(bitmap),
                                                   width, height, stride, 24,
                                                   0xFF0000, 0x00FF00, 0x0000FF, true);
    if ( FIF_GIF == format ){
        FIBITMAP *gif = FreeImage_ColorQuantize(image, FIQ_WUQUANT);
        FreeImage_Unload(image);
        return gif;
    } else {
        return image;
    }
}


FIBITMAP*
render_image(ImageWrapper *image_wrapper, int index, FREE_IMAGE_FORMAT format, int width, int height){
    CPDF_Page *page = image_wrapper->page_wrapper->page();
    CPDF_ImageObject *image_obj = static_cast<CPDF_ImageObject*>(page->GetObjectByIndex(index));
    CFX_DIBSource *dib = image_obj->m_pImage->LoadDIBSource();
    FIBITMAP *bmp = FreeImage_Allocate(dib->GetWidth(), dib->GetHeight(), dib->GetBPP());
    unsigned int byte_width = dib->GetWidth() * (dib->GetBPP()/8);
    for (int row=0; row < dib->GetHeight(); row++ ){
        auto dest_row = FreeImage_GetScanLine(bmp,dib->GetHeight()-row-1);
        std::memcpy(dest_row, dib->GetScanline(row), byte_width );
    }
    return bmp;
}

FIBITMAP*
render_to_bitmap(VALUE self, FREE_IMAGE_FORMAT format){
    int width  = 0;
    int height = 0;
    VALUE rb_width = rb_iv_get(self, "@width");
    if (T_FIXNUM == TYPE(rb_width)){
        width = FIX2INT(rb_width);
    }
    VALUE rb_height = rb_iv_get(self, "@height");
    if (T_FIXNUM == TYPE(rb_height)){
        height = FIX2INT(rb_height);
    }
    // we must have at least one of width or height
    if (!width && !height){
        rb_raise(rb_eRuntimeError, "Both height and width must be set to a number");
    }
    ImageWrapper *img;
    Data_Get_Struct(self, ImageWrapper, img);

    if (-1 == img->page_object_index){
        CPDF_Page *page = img->page_wrapper->page();
        return render_page(page, format, width, height);
    } else {
        return render_image(img, img->page_object_index, format, width, height);
    }
}


/*
 * call-seq:
 *   as_science -> ImageScience instance
 *
 * Converts to an ImageScience bitmap and returns it.
 * The ImageScience (https://github.com/seattlerb/image_science) library
 * must be installed and required before calling this method or
 * a NameError: uninitialized constant ImageScience exception will be raised.

 === Example
    pdf = PDFium::Document.new( "test.pdf" )
    page = pdf.pages.first
    page.images.each do | image |
        image.as_science.cropped_thumbnail 100 do |thumb|
            thumb.save "image-#{image.index}-cropped.png"
        end
    end

 */
VALUE
image_as_science(VALUE self){
    VALUE RBImageScience = rb_const_get(rb_cObject, rb_intern("ImageScience"));
    FIBITMAP *image = render_to_bitmap(self, FIF_BMP);

    VALUE instance = Data_Wrap_Struct(RBImageScience, NULL, NULL, image);
    rb_iv_set(instance, "@file_type", INT2FIX(FIF_BMP));
    return instance;
}

/*
 call-seq:
   save( file )

 Save image to a file

 @param file [String, Pathname] path to file to save image to
 @return [Boolean] indicating success or failure

 */
VALUE
image_save(VALUE self, VALUE rb_file){
    // figure out the desired format from the file extension
    const char* file = StringValuePtr(rb_file);

    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(file);
    if((format == FIF_UNKNOWN) || !FreeImage_FIFSupportsWriting(format)) {
        rb_raise(rb_eArgError, "Unable to write to a image of that type");
    }

    FIBITMAP *image = render_to_bitmap(self, format);

    bool success = FreeImage_Save(format, image, file, 0);

    // unload the image
    FreeImage_Unload(image);

    return success ? Qtrue : Qfalse;
}

/*
 call-seq:
   data(:format)

 @param format [symbol] any file extension recogized by FreeImage.
 @return String containing binary data for the image in the specified format.

 Used in conjuction with Document.from_memory this can render be used to
 render a PDF's pages completely in memory.

 === Example rendering a PDF to AWS without hitting disk
    # Assuming AWS::S3 is already authorized elsewhere
    bucket = AWS::S3.new.buckets['my-pdfs']
    pdf = PDFium::Document.from_memory bucket.objects['secrets.pdf'].read
    pdf.pages.each do | page |
      path = "secrets/page-#{page.number}.jpg"
      bucket.objects[path].write page.as_image(height: 1000).data(:jpg)
      page.images.each do | image |
        path = "secrets/page-#{page.number}-image-#{image.index}.png"
        bucket.objects[path].write image.data(:png)
      end
    end

 */
static VALUE
image_data(VALUE self, VALUE rb_format)
{
    VALUE path = RB::to_s(rb_format);
    const char* type = StringValuePtr(path);

    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(type);
    if((format == FIF_UNKNOWN) || !FreeImage_FIFSupportsWriting(format)) {
        rb_raise(rb_eArgError, "Unable to write to a image of that type");
    }

    FIBITMAP *image = render_to_bitmap(self, format);

    FIMEMORY *mem = FreeImage_OpenMemory(); //mem_buffer, buf.st_size);

    bool success = FreeImage_SaveToMemory(format, image, mem, 0);
    if (!success){
        FreeImage_Unload(image);
        rb_raise(rb_eArgError, "Unable to save image to memory buffer");
    }

    long size = FreeImage_TellMemory(mem);

    char *buffer = ALLOC_N(char, size);

    FreeImage_SeekMemory(mem, 0L, SEEK_SET);

    FreeImage_ReadMemory(buffer, size, 1, mem);

    FreeImage_Unload(image);

    FreeImage_CloseMemory(mem);
    VALUE ret = rb_str_new(buffer, size);

    xfree(buffer);
    return ret;
}

void
define_image_class(){
    VALUE PDFium = RB::PDFium();
    VALUE RB_Image = rb_define_class_under(PDFium, "Image",  rb_cObject);

    rb_define_alloc_func(RB_Image, image_allocate);
    rb_define_private_method (RB_Image, "initialize",   RUBY_METHOD_FUNC(image_initialize),  -1);

    /* Returns the bouding box of the image as a {PDFium::BoundingBox} */
    rb_define_attr( RB_Image, "bounds", 1, 0 );

    /* Returns the index of the image on the page.
     * Note: The index method is only provided as a convience method.
     * It has no relation to the position of images on the page.
     * Do not depend on the top-left image being index 0, even if it often is. */
    rb_define_attr( RB_Image, "index",  1, 0 );

    /* Height of the image in pixels (Fixnum) */
    rb_define_attr( RB_Image, "height", 1, 1 );

    /* Width of the image in pixels (Fixnum) */
    rb_define_attr( RB_Image, "width",  1, 1 );

    rb_define_method( RB_Image, "save", RUBY_METHOD_FUNC(image_save), 1);
    rb_define_method( RB_Image, "data", RUBY_METHOD_FUNC(image_data), 1);
    rb_define_method( RB_Image, "as_science", RUBY_METHOD_FUNC(image_as_science),0);

}
