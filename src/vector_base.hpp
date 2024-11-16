#ifndef VECTOR_BASE_HPP
#define VECTOR_BASE_HPP

#include "sparse.hpp"
#include <map>

class VectorBase {
public:
    const SPVEC& at(int32_t id) const;
    void insert(int32_t id, const SPVEC& v);
    size_t size() const;
    std::vector<SPVEC> export_vectors(const std::vector<int32_t>& ids) const;
    std::vector<const SPVEC*> get_vectors(const std::vector<int32_t>& ids) const;
    const std::map<int32_t, SPVEC> get_map() const;

    VectorBase();
    VectorBase(std::string filename, int32_t dim, STR_HASH_FUNC(f) = NULL);

private:
    std::map<int32_t, SPVEC> _storage;
};



#endif