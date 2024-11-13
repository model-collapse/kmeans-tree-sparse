#ifndef VECTOR_BASE_HPP
#define VECTOR_BASE_HPP

#include "sparse.hpp"
#include <map>

class VectorBase {
public:
    const SPVEC& at(int32_t id) const;
    void insert(int32_t id, const SPVEC& v);
    size_t size() const;
    std::vector<SPVEC> get_vectors(const std::vector<int32_t>& ids) const;

    VectorBase();
    VectorBase(std::string filename, int32_t dim, STR_HASH_FUNC(f) = NULL);

private:
    std::map<int32_t, SPVEC> _storage;
};

#endif