#ifndef PAYLOAD_HPP
#define PAYLOAD_HPP
#include <stdlib.h>
#include <vector>
#include <set>
#include "sparse.hpp"

class LeafPayLoad {
public:
    virtual size_t size() = 0;
    virtual int32_t insert(int32_t id, const SPVEC& v) = 0;
    virtual std::vector<SPVEC> get_all_vectors() = 0;
    virtual std::set<int32_t> get_all_ids() = 0;

    virtual LeafPayLoad* new_payload() = 0;
    virtual void dispose(LeafPayLoad** t) = 0;
};

#endif