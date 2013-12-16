# This file allows you to programmatically create blocks in Craft.
# Please use this wisely. Test on your own server first. Do not abuse it.

import socket

HOST = '127.0.0.1'
PORT = 4080

GRASS = 1
SAND = 2
STONE = 3
BRICK = 4
WOOD = 5
CEMENT = 6
DIRT = 7
PLANK = 8
SNOW = 9
GLASS = 10
COBBLE = 11

OFFSETS = [
    (-0.5, -0.5, -0.5),
    (-0.5, -0.5, 0.5),
    (-0.5, 0.5, -0.5),
    (-0.5, 0.5, 0.5),
    (0.5, -0.5, -0.5),
    (0.5, -0.5, 0.5),
    (0.5, 0.5, -0.5),
    (0.5, 0.5, 0.5),
]

class Client(object):
    def __init__(self, host=HOST, port=PORT):
        self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.conn.connect((host, port))
    def set(self, x, y, z, w):
        self.conn.sendall('B,%d,%d,%d,%d\n' % (x, y, z, w))
    def circle_x(self, cx, cy, cz, r, w, fill=False):
        self.sphere(cx, cy, cz, r, w, fill, fx=True)
    def circle_y(self, cx, cy, cz, r, w, fill=False):
        self.sphere(cx, cy, cz, r, w, fill, fy=True)
    def circle_z(self, cx, cy, cz, r, w, fill=False):
        self.sphere(cx, cy, cz, r, w, fill, fz=True)
    def sphere(self, cx, cy, cz, r, w, fill=False, fx=False, fy=False, fz = False):
        for x in range(cx - r, cx + r + 1):
            if fx and x != cx:
                continue
            for y in range(cy - r, cy + r + 1):
                # if y < cy:
                #     continue # top hemisphere only
                if fy and y != cy:
                    continue
                for z in range(cz - r, cz + r + 1):
                    if fz and z != cz:
                        continue
                    inside = False
                    outside = fill
                    for dx, dy, dz in OFFSETS:
                        ox, oy, oz = x + dx, y + dy, z + dz
                        d2 = (ox - cx) ** 2 + (oy - cy) ** 2 + (oz - cz) ** 2
                        d = d2 ** 0.5
                        if d < r:
                            inside = True
                        else:
                            outside = True
                    if inside and outside:
                        self.set(x, y, z, w)
    def cuboid(self, x1, x2, y1, y2, z1, z2, w, fill=True):
        a = 0
        a += x1 == x2
        a += y1 == y2
        a += z1 == z2
        for x in range(x1, x2 + 1):
            for y in range(y1, y2 + 1):
                for z in range(z1, z2 + 1):
                    n = 0
                    n += x in (x1, x2)
                    n += y in (y1, y2)
                    n += z in (z1, z2)
                    if not fill and n <= a:
                        continue
                    self.set(x, y, z, w)
    def pyramid(self, y, x1, x2, z1, z2, w, fill=False):
        while x2 >= x1 and z2 >= z2:
            self.cuboid(x1, x2, y, y, z1, z2, w, fill)
            y, x1, x2, z1, z2 = y + 1, x1 + 1, x2 - 1, z1 + 1, z2 - 1

def main():
    client = Client()
    # client.circle_y(0, 32, 0, 16, STONE, True)
    # client.circle_y(0, 33, 0, 16, BRICK)
    # client.cuboid(-1, 1, 1, 31, -1, 1, CEMENT)
    # client.cuboid(-1024, 1024, 32, 32, -3, 3, STONE)
    # client.cuboid(-3, 3, 32, 32, -1024, 1024, STONE)
    # client.cuboid(-1024, 1024, 33, 33, -3, -3, BRICK)
    # client.cuboid(-1024, 1024, 33, 33, 3, 3, BRICK)
    # client.cuboid(-3, -3, 33, 33, -1024, 1024, BRICK)
    # client.cuboid(3, 3, 33, 33, -1024, 1024, BRICK)
    # client.sphere(0, 32, 0, 16, GLASS)
    # for y in range(1, 32):
    #     client.circle_y(0, y, 0, 4, CEMENT, True)
    # client.circle_x(16, 33, 0, 3, BRICK)
    # client.circle_x(-16, 33, 0, 3, BRICK)
    # client.circle_z(0, 33, 16, 3, BRICK)
    # client.circle_z(0, 33, -16, 3, BRICK)
    # for x in range(0, 1024, 32):
    #     client.cuboid(x - 1, x + 1, 31, 32, -1, 1, CEMENT)
    #     client.cuboid(-x - 1, -x + 1, 31, 32, -1, 1, CEMENT)
    #     client.cuboid(x, x, 1, 32, -1, 1, CEMENT)
    #     client.cuboid(-x, -x, 1, 32, -1, 1, CEMENT)
    # for z in range(0, 1024, 32):
    #     client.cuboid(-1, 1, 31, 32, z - 1, z + 1, CEMENT)
    #     client.cuboid(-1, 1, 31, 32, -z - 1, -z + 1, CEMENT)
    #     client.cuboid(-1, 1, 1, 32, z, z, CEMENT)
    #     client.cuboid(-1, 1, 1, 32, -z, -z, CEMENT)
    # for x in range(0, 1024, 8):
    #     client.set(x, 32, 0, CEMENT)
    #     client.set(-x, 32, 0, CEMENT)
    # for z in range(0, 1024, 8):
    #     client.set(0, 32, z, CEMENT)
    #     client.set(0, 32, -z, CEMENT)
    # client.pyramid(12, 32, 32+64-1, 32, 32+64-1, COBBLE)

if __name__ == '__main__':
    main()
