#include "security.h"

extern void tfs_demo(uint64_t ts);

int unittest_tfs(uint64_t timestamp)
{
    tfs_demo(timestamp);
    return 0;
}
