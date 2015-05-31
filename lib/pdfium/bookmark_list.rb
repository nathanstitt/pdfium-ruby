module PDFium

    # A list of bookmarks for a Document or a Bookmark
    class BookmarkList

        include Enumerable
        # Not used directly, but called from Document#bookmarks
        def initialize(initial)
            @first=initial
        end

        # Calls block once for each bookmark that exists at the current level
        # Since bookmarks form a tree, each bookmark may have one or more children
        def each
            bookmark = @first
            while bookmark
                yield bookmark
                bookmark = bookmark.next_sibling
            end
        end

        # True if no bookmarks exist, false if at least one is present
        def empty?
            @first.nil?
        end
    end

end
