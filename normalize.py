from mpl_toolkits import mplot3d
import numpy as np
import matplotlib.pyplot as plt
import cv2


# Takes height and width as parameters
# sets and gets values in array
# returns and array
class NormArray:
    def __init__(self, height, width):
        self.height = height
        self.width = width
        self.array = np.arange(self.height * self.width).reshape(self.width, self.height)

    def setval(self, x, y, val):
        self.array[x][y] = val

    def getval(self, x, y):
        val = self.array[int(x)][int(y)]
        return val

    def getarray(self):
        return self.array


# Takes image as parameter and normalizes it
# Normalize means taking an array of value and making min 0 and max 100 (or whatever value you want to the max)
# Returns normalized array#
#
## \package normalize
# \brief PURPOSE : Creates a file containing pixel values of topographic image
#
# \brief Ref : Req 1.0 Craft implementation shall create a terrain from a topographic image
# \brief Ref : Req 1.3 The realistic range of block heights shall be a minimum of 20 blocks and a maximum of 100 blocks
# \brief Ref : Req 3.0 One pixel in topographic image shall correspond to a 2x2 block area
# \brief Ref : Req 4.0 The minimum terrain generated block height shall be 20 blocks
# \brief Ref : Req 4.1 The maximum terrain generated block height shall be 100 blocks
#
# \param image : the topographic image to be processed
# \return norm_arr : file containing the data of pixel values for the topographic image
##
def normalize(image):
    # scan image, resize to equal dimensions, convert to array of rgb values.
    height = 1000
    width = 1000
    max_block_height = 100
    img = plt.imread(image)
    img = cv2.resize(img, (height, width))
    img_arr = np.asarray(img)
    norm_arr = NormArray(height * 2, width * 2)

    # get max and min values from img_arr
    max = img_arr.max()
    min = img_arr.min()

    p = 0
    q = 0
    # copy img_arr to norm_arr
    # Ref : Req 1.3 The realistic range of block heights shall be a minimum of 20 blocks and a maximum of 100 blocks
    # Ref : Req 3.0 One pixel in topographic image shall correspond to a 2x2 block area
    for x in range(norm_arr.height):
        if (x % 2) == 0 and x != 0 and p < height:
            p += 1
        for y in range(norm_arr.width):
            if (y % 2) == 0 and y != 0 and q < height:
                q += 1
            val = img_arr[p][q].min()
            # depth of blocks is from 0 to 256 blocks, but max height will be 100 to give room to build higher
            # and have a more terrain that's easier to traverse

            # Ref Req 4.0 The maximum terrain generated block height shall be 100
            val = ((val - min) / (max - min)) * max_block_height
            # added 20 b/c the lowest point in the image should still be able to be mined past
            #Ref : Req 4.0 The minimum terrain generated block height shall be 20 blocks
            val = int(val + 20)
            norm_arr.setval(x, y, val)
        q = 0

    return norm_arr

## \package normalize
# \brief PURPOSE : Creates a file containing pixel values of topographic image
# \brief Ref : Req 1.1 Topographic image shall be a .png image.
#
# \param image_name : the topographic image file to be processed
# \return file : file containing the data of pixel values for the topographic image
##
def process_image(image_name):

    if not image.endswith(".png"):
        return print("File must be a .png")

    pixel_arr = normalize(image)
    with open("src/data_file.txt", 'w') as file:
        for x in range(pixel_arr.height):
            for y in range(pixel_arr.width):
                file.write(str(pixel_arr.getval(x, y)) + '\n')

    file.close()
    return file
