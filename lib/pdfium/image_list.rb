module PDFium

    # A list of Image instances for a given Page.  Is returned by Page#images
    # Images are lazily loaded upon request.
    class ImageList

        include Enumerable

        # Load list for a given page.  Not normally called directly, but from Page#images
        def initialize(page)
            @page=page
        end

        # Calls block once for each object on the document
        def each(&block)
            @page.each_image(&block)
        end

    end

end
