#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

#include "fpdf_dataavail.h"
#include "fpdf_ext.h"
#include "fpdfformfill.h"
#include "fpdftext.h"
#include "fpdfview.h"

// This may be needed
//#v8/include/v8.h"

#define PATH_SEPARATOR '/'

enum OutputFormat {
  OUTPUT_NONE,
  OUTPUT_PPM,
};

struct Options {
    Options() : output_format(OUTPUT_NONE) { }

    OutputFormat output_format;
    std::string exe_path;
    std::string bin_directory;
};

// Reads the entire contents of a file into a newly malloc'd buffer.
static char* GetFileContents(const char* filename, size_t* retlen) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open: %s\n", filename);
        return NULL;
    }
    (void) fseek(file, 0, SEEK_END);
    size_t file_length = ftell(file);
    if (!file_length) {
        return NULL;
    }
    (void) fseek(file, 0, SEEK_SET);
    char* buffer = (char*) malloc(file_length);
    if (!buffer) {
        return NULL;
    }
    size_t bytes_read = fread(buffer, 1, file_length, file);
    (void) fclose(file);
    if (bytes_read != file_length) {
        fprintf(stderr, "Failed to read: %s\n", filename);
        free(buffer);
        return NULL;
    }
    *retlen = bytes_read;
    return buffer;
}


static void WritePpm(const char* pdf_name, int num, const void* buffer_void,
                     int stride, int width, int height) {
    const char* buffer = reinterpret_cast<const char*>(buffer_void);

    if (stride < 0 || width < 0 || height < 0)
        return;
    if (height > 0 && width > INT_MAX / height)
        return;
    int out_len = width * height;
    if (out_len > INT_MAX / 3)
        return;
    out_len *= 3;

    char filename[256];
    snprintf(filename, sizeof(filename), "%s.%d.ppm", pdf_name, num);
    FILE* fp = fopen(filename, "wb");
    if (!fp)
        return;
    fprintf(fp, "P6\n# PDF test render\n%d %d\n255\n", width, height);
    // Source data is B, G, R, unused.
    // Dest data is R, G, B.
    char* result = new char[out_len];
    if (result) {
        for (int h = 0; h < height; ++h) {
            const char* src_line = buffer + (stride * h);
            char* dest_line = result + (width * h * 3);
            for (int w = 0; w < width; ++w) {
                // R
                dest_line[w * 3] = src_line[(w * 4) + 2];
                // G
                dest_line[(w * 3) + 1] = src_line[(w * 4) + 1];
                // B
                dest_line[(w * 3) + 2] = src_line[w * 4];
            }
        }
        fwrite(result, out_len, 1, fp);
        delete [] result;
    }
    fclose(fp);
}


int Form_Alert(IPDF_JSPLATFORM*, FPDF_WIDESTRING, FPDF_WIDESTRING, int, int) {
    printf("Form_Alert called.\n");
    return 0;
}

void Unsupported_Handler(UNSUPPORT_INFO*, int type) {
    std::string feature = "Unknown";
    switch (type) {
    case FPDF_UNSP_DOC_XFAFORM:
        feature = "XFA";
        break;
    case FPDF_UNSP_DOC_PORTABLECOLLECTION:
        feature = "Portfolios_Packages";
        break;
    case FPDF_UNSP_DOC_ATTACHMENT:
    case FPDF_UNSP_ANNOT_ATTACHMENT:
        feature = "Attachment";
        break;
    case FPDF_UNSP_DOC_SECURITY:
        feature = "Rights_Management";
        break;
    case FPDF_UNSP_DOC_SHAREDREVIEW:
        feature = "Shared_Review";
        break;
    case FPDF_UNSP_DOC_SHAREDFORM_ACROBAT:
    case FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM:
    case FPDF_UNSP_DOC_SHAREDFORM_EMAIL:
        feature = "Shared_Form";
        break;
    case FPDF_UNSP_ANNOT_3DANNOT:
        feature = "3D";
        break;
    case FPDF_UNSP_ANNOT_MOVIE:
        feature = "Movie";
        break;
    case FPDF_UNSP_ANNOT_SOUND:
        feature = "Sound";
        break;
    case FPDF_UNSP_ANNOT_SCREEN_MEDIA:
    case FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA:
        feature = "Screen";
        break;
    case FPDF_UNSP_ANNOT_SIG:
        feature = "Digital_Signature";
        break;
    }
    printf("Unsupported feature: %s.\n", feature.c_str());
}

bool ParseCommandLine(const std::vector<std::string>& args,
                      Options* options, std::list<std::string>* files) {
    if (args.empty()) {
        return false;
    }
    options->exe_path = args[0];
    size_t cur_idx = 1;
    for (; cur_idx < args.size(); ++cur_idx) {
        const std::string& cur_arg = args[cur_idx];
        if (cur_arg == "--ppm") {
            if (options->output_format != OUTPUT_NONE) {
                fprintf(stderr, "Duplicate or conflicting --ppm argument\n");
                return false;
            }
            options->output_format = OUTPUT_PPM;
        }
        else
            break;
    }
    if (cur_idx >= args.size()) {
        fprintf(stderr, "No input files.\n");
        return false;
    }
    for (size_t i = cur_idx; i < args.size(); i++) {
        files->push_back(args[i]);
    }
    return true;
}

class TestLoader {
public:
    TestLoader(const char* pBuf, size_t len);

    const char* m_pBuf;
    size_t m_Len;
};

TestLoader::TestLoader(const char* pBuf, size_t len)
    : m_pBuf(pBuf), m_Len(len) {
}

int Get_Block(void* param, unsigned long pos, unsigned char* pBuf,
              unsigned long size) {
    TestLoader* pLoader = (TestLoader*) param;
    if (pos + size < pos || pos + size > pLoader->m_Len) return 0;
    memcpy(pBuf, pLoader->m_pBuf + pos, size);
    return 1;
}

bool Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size) {
    return true;
}

void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {
}

void RenderPdf(const std::string& name, const char* pBuf, size_t len,
               OutputFormat format) {
    printf("Rendering PDF file %s.\n", name.c_str());

    IPDF_JSPLATFORM platform_callbacks;
    memset(&platform_callbacks, '\0', sizeof(platform_callbacks));
    platform_callbacks.version = 1;
    platform_callbacks.app_alert = Form_Alert;

    FPDF_FORMFILLINFO form_callbacks;
    memset(&form_callbacks, '\0', sizeof(form_callbacks));
    form_callbacks.version = 1;
    form_callbacks.m_pJsPlatform = &platform_callbacks;

    TestLoader loader(pBuf, len);

    FPDF_FILEACCESS file_access;
    memset(&file_access, '\0', sizeof(file_access));
    file_access.m_FileLen = static_cast<unsigned long>(len);
    file_access.m_GetBlock = Get_Block;
    file_access.m_Param = &loader;

    FX_FILEAVAIL file_avail;
    memset(&file_avail, '\0', sizeof(file_avail));
    file_avail.version = 1;
    file_avail.IsDataAvail = Is_Data_Avail;

    FX_DOWNLOADHINTS hints;
    memset(&hints, '\0', sizeof(hints));
    hints.version = 1;
    hints.AddSegment = Add_Segment;

    FPDF_DOCUMENT doc;
    FPDF_AVAIL pdf_avail = FPDFAvail_Create(&file_avail, &file_access);

    (void) FPDFAvail_IsDocAvail(pdf_avail, &hints);

    if (!FPDFAvail_IsLinearized(pdf_avail)) {
        printf("Non-linearized path...\n");
        doc = FPDF_LoadCustomDocument(&file_access, NULL);
    } else {
        printf("Linearized path...\n");
        doc = FPDFAvail_GetDocument(pdf_avail, NULL);
    }

    (void) FPDF_GetDocPermissions(doc);
    (void) FPDFAvail_IsFormAvail(pdf_avail, &hints);

    FPDF_FORMHANDLE form = FPDFDOC_InitFormFillEnvironment(doc, &form_callbacks);
    FPDF_SetFormFieldHighlightColor(form, 0, 0xFFE4DD);
    FPDF_SetFormFieldHighlightAlpha(form, 100);

    int first_page = FPDFAvail_GetFirstPageNum(doc);
    (void) FPDFAvail_IsPageAvail(pdf_avail, first_page, &hints);

    int page_count = FPDF_GetPageCount(doc);
    for (int i = 0; i < page_count; ++i) {
        (void) FPDFAvail_IsPageAvail(pdf_avail, i, &hints);
    }

    FORM_DoDocumentJSAction(form);
    FORM_DoDocumentOpenAction(form);

    size_t rendered_pages = 0;
    size_t bad_pages = 0;
    for (int i = 0; i < page_count; ++i) {
        FPDF_PAGE page = FPDF_LoadPage(doc, i);
        if (!page) {
            bad_pages ++;
            continue;
        }
        FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
        FORM_OnAfterLoadPage(page, form);
        FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_OPEN);

        int width = static_cast<int>(FPDF_GetPageWidth(page));
        int height = static_cast<int>(FPDF_GetPageHeight(page));
        FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
        FPDFBitmap_FillRect(bitmap, 0, 0, width, height, 0xFFFFFFFF);

        FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
        rendered_pages ++;

        FPDF_FFLDraw(form, bitmap, page, 0, 0, width, height, 0, 0);
        int stride = FPDFBitmap_GetStride(bitmap);
        const char* buffer =
            reinterpret_cast<const char*>(FPDFBitmap_GetBuffer(bitmap));

        switch (format) {
        case OUTPUT_PPM:
            WritePpm(name.c_str(), i, buffer, stride, width, height);
            break;
        default:
            break;
        }

        FPDFBitmap_Destroy(bitmap);

        FORM_DoPageAAction(page, form, FPDFPAGE_AACTION_CLOSE);
        FORM_OnBeforeClosePage(page, form);
        FPDFText_ClosePage(text_page);
        FPDF_ClosePage(page);
    }

    FORM_DoDocumentAAction(form, FPDFDOC_AACTION_WC);
    FPDFDOC_ExitFormFillEnvironment(form);
    FPDF_CloseDocument(doc);
    FPDFAvail_Destroy(pdf_avail);

    std::cout << "Loaded, parsed and rendered " << rendered_pages << " pages." << std::endl;
    std::cout << "Skipped " << bad_pages << " pages." << std::endl;
}

int main(int argc, const char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);
    Options options;
    std::list<std::string> files;
    if (!ParseCommandLine(args, &options, &files)) {
        printf("Usage: pdfium_test [OPTION] [FILE]...\n");
        printf("--bin-dir=<path> - override path to v8 external data\n");
        printf("--ppm - write page images <pdf-name>.<page-number>.ppm\n");
        return 1;
    }

    //    v8::V8::InitializeICU();


    FPDF_InitLibrary();

    UNSUPPORT_INFO unsuppored_info;
    memset(&unsuppored_info, '\0', sizeof(unsuppored_info));
    unsuppored_info.version = 1;
    unsuppored_info.FSDK_UnSupport_Handler = Unsupported_Handler;

    FSDK_SetUnSpObjProcessHandler(&unsuppored_info);

    while (!files.empty()) {
        std::string filename = files.front();
        files.pop_front();
        size_t file_length = 0;
        char* file_contents = GetFileContents(filename.c_str(), &file_length);
        if (!file_contents)
            continue;
        RenderPdf(filename, file_contents, file_length, options.output_format);
        free(file_contents);
    }

    FPDF_DestroyLibrary();

    return 0;
}
