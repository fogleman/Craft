#ifndef _ITEM_H_
#define _ITEM_H_

#include <cstdint>

namespace konstructs {
    struct ItemStack {
        uint32_t amount;
        uint16_t type;
        uint16_t health;
    };
};

#endif
