import normalize

norm = normalize.normalize("heightdata.png", 6, 6)

class TestNormArray:
    """test_norm_array references requirement 3.0 because it shows 2x2 block area of (0,0),
        (0,1), (1,0), (1,1), this area will for sure be a 2x2 block area"""
    # \brief Ref : Req 3.0 One pixel in topographic image shall correspond to a 2x2 block area
    def test_norm_array(self):
        assert norm.getval(0, 0) == 66
        assert norm.getval(0, 1) == 66
        assert norm.getval(1, 0) == 66
        assert norm.getval(1, 1) == 66

    """test_array_dimensions reference requirement 3.0 because if each pixel corresponds
     to a 2x2 area, then the height and width will be doubled from 6x6 to 12x12 as an example"""
    # \brief Ref : Req 3.0 One pixel in topographic image shall correspond to a 2x2 block area
    def test_array_dimensions(self):
        assert norm.height == 12
        assert norm.width == 12

    # \brief Ref : Req 1.2 Pixel values shall be normalized to represent a realistic range of block heights
    # \brief Ref : Req 1.3 The realistic range of block heights shall be a minimum of 20 blocks and a maximum of 100 blocks
    def test_max(self):
        assert norm.get_max() == [100, 8, 8]
    # \brief Ref : Req 1.2 Pixel values shall be normalized to represent a realistic range of block heights
    # \brief Ref : Req 1.3 The realistic range of block heights shall be a minimum of 20 blocks and a maximum of 100 blocks
    def test_min(self):
        assert norm.get_min() == [20, 2, 0]

class TestProcessImage:
    # \brief Ref : Req Req 1.1 Topographic image shall be a .png image
    def test_file_ending(self):
        ## test file not ending in .png
        assert normalize.process_image("heightdata.jpg", 10, 10) == 'File must be a .png \n'

    def test_file_not_found(self):
        ## test file that doesn't exist
        assert normalize.process_image("heightfile.png", 10, 10) == None


