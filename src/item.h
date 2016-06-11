#ifndef _ITEM_H_
#define _ITEM_H_

#include <cstdint>

namespace konstructs {
    struct ItemStack {
        int amount;
        uint16_t type;
    };
};

#endif
