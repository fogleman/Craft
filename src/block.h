#ifndef __BLOCK_H__
#define __BLOCK_H__

namespace konstructs {
    struct BlockData {
        int blocks[256][6];
        char is_plant[256];
        char is_obstacle[256];
        char is_transparent[256];
    };
};

#endif
