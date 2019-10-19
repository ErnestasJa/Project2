#ifndef MNODE_H
#define MNODE_H

#include "MNodeUtil.h"

struct MNode
{
public:
    uint32_t start;
    uint8_t r,g,b;
    uint8_t size;

    MNode(const MNode & node) = default;
    MNode(MNode&& n) noexcept;
    MNode(uint32_t x, uint32_t y, uint32_t z, uint8_t nodeSize=1);
    MNode(uint32_t morton, uint8_t nodeSize, uint8_t red, uint8_t green, uint8_t blue);
    MNode(uint32_t morton, uint8_t nodeSize=1);
    MNode();

    MNode& operator=(MNode&& x) noexcept = default;
    bool operator<(const MNode &other) const;
    bool operator==(const MNode &other) const;
};

#endif
