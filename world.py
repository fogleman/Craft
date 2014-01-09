# gcc -std=c99 -O3 -shared -o world \
#   -I src -I deps/noise deps/noise/noise.c src/world.c

from ctypes import CDLL, CFUNCTYPE, c_int, c_void_p

dll = CDLL('./world')

WORLD_FUNC = CFUNCTYPE(None, c_int, c_int, c_int, c_int, c_void_p)

def seed(x):
    dll.seed(x)

def create_world(p, q):
    result = {}
    def world_func(x, y, z, w, arg):
        result[(x, y, z)] = w
    dll.create_world(p, q, WORLD_FUNC(world_func), None)
    return result
