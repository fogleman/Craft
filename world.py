# gcc -std=c99 -O3 -shared -o world \
#   -I src -I deps/noise deps/noise/noise.c src/world.c

from ctypes import CDLL, CFUNCTYPE, c_int, c_void_p
from collections import OrderedDict

dll = CDLL('./world')

WORLD_FUNC = CFUNCTYPE(None, c_int, c_int, c_int, c_int, c_void_p)

def dll_seed(x):
    dll.seed(x)

def dll_create_world(p, q):
    result = {}
    def world_func(x, y, z, w, arg):
        result[(x, y, z)] = w
    dll.create_world(p, q, WORLD_FUNC(world_func), None)
    return result

class World(object):
    def __init__(self, seed=None, cache_size=64):
        self.seed = seed
        self.cache = OrderedDict()
        self.cache_size = cache_size
    def create_chunk(self, p, q):
        if self.seed is not None:
            dll_seed(self.seed)
        return dll_create_world(p, q)
    def get_chunk(self, p, q):
        try:
            chunk = self.cache.pop((p, q))
        except KeyError:
            chunk = self.create_chunk(p, q)
        self.cache[(p, q)] = chunk
        if len(self.cache) > self.cache_size:
            self.cache.popitem(False)
        return chunk
