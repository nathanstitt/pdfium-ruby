# Ruby bindings for Google's PDFium project

This allows Ruby efficiently to extract information from PDF files.

It currently has only very rudimantary PDF editing capabilities.

RDoc documentation is also available and the test directory has examples of usage.

## Open and saveing

```ruby
pdf = PDFium::Document.new("test.pdf")
pdf.save
```

## Document information

Page count:
```ruby
pdf.page_count
```

PDF Metadata:
```ruby
pdf.metadata
```

Returns a hash with keys = :title, :author :subject, :keywords, :creator, :producer, :creation_date, :mod_date



## Bookmarks

```ruby
def print_bookmarks(list, indent=0)
    list.bookmarks.each do | bm |
        print ' ' * indent
        puts bm.title
        print_marks( bm.children )
    end
end
print_bookmarks( pdf.bookmarks )
```

## Render page as an image

```ruby
pdf.each_page | page |
    page.as_image(width: 800).save("test-{page.number}.png")
end
```

## Extract embedded images from page
```ruby
doc = PDFium::Document.new("test.pdf")
page = doc.page_at(0)
page.images do |image|
    img.save("page-0-image-#{image.index}.png")
end
```

## Text access

Text is returned as a UTF-16LE encoded string.  Future version may return position information as well

```ruby
pdf.page_at(0).text.encode!("ASCII-8BIT")
```
