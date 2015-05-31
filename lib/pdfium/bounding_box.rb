module PDFium

    # The size of an object.  Used with both Page and Image
    class BoundingBox

        # dimensions for the BoundingBox.  Fixnum given in terms of points
        attr_reader :left, :right, :top, :bottom

        # Left, Right, Top, Bottom
        def initialize(l,r,t,b)
            @left=l; @right=r; @top=t; @bottom=b
        end

  end

end
