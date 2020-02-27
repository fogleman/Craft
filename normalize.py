from mpl_toolkits import mplot3d
import numpy as np
import matplotlib.pyplot as plt
import cv2


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

    def getarray (self):
        return self.array

def normalize(image):
    #scan image, resize to equal dimensions, convert to array of rgb values.
    img = plt.imread(image)
    img = cv2.resize(img, (1000, 1000))
    img_arr = np.asarray(img)
    norm_arr = NormArray(1000, 1000)

    #get max and min values from img_arr
    max = img_arr.max(); min = img_arr.min()

    #copy img_arr to norm_arr
    for x in range(norm_arr.height):
        for y in range(norm_arr.width):
            val = img_arr[x][y].min()
            val = ((val - min)/(max - min)) * 256   #depth of blocks is from 0 to 256 blocks.
            val = int(val)
            norm_arr.setval(x, y, val)

    return norm_arr

def plot(arr):
    fig = plt.figure()
    ax = plt.axes(projection='3d')
    x = np.linspace(0, int(arr.height), int(arr.width)) # z-value set to 1 because arr values are from 1 to 0
    y = np.linspace(0, int(arr.height), int(arr.width))

    X, Y = np.meshgrid(x, y)
    Z = arr.getarray()

    ax.plot_surface(X, Y, Z, rstride=1, cstride=1, cmap='binary')
    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_zlabel('z')

    ax.view_init(1000, 1000)
    plt.show()

arr = normalize("heightdata.png")
plot(arr)