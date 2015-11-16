#ifndef __BLOCK_H__
#define __BLOCK_H__

#define VOID_BLOCK 0
#define SOLID_BLOCK 100

namespace konstructs {
    struct BlockData {
        int blocks[256][6];
        char is_plant[256];
        char is_obstacle[256];
        char is_transparent[256];
    };
};

#endif
