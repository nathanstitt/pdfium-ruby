module PDFium

    # A list of Page objects associated with a Document.
    class PageList

        include Enumerable

        # Create a new listing from the given document.
        # Not normally called directly, is called internally by Document#pages
        def initialize(document)
            @document=document
        end

        # Calls block once for each page on the document, yielding the current page
        # After the page is yielded, Page#unload will be automatically called.
        #
        # _note_ Subsequent calls to this function will return different Page instances.
        def each(&block)
            @document.each_page(&block)
        end

        # Returns the number of pages on the document
        def count
            @document.page_count
        end

        # Returns a Page instance for the given number.
        # If the given page_number is not valid, an ArgumentError will be raised.
        #
        # _note_ Subsequent calls to this function will return different Page instances.
        def [](index)
            @document.page_at(index)
        end
    end

end
