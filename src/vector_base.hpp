#include "sparse.hpp"
#include <map>

class VectorBase {
public:
    const SPVEC& vec_at(int32_t id) const;
    void insert(int32_t id, const SPVEC& v);
    size_t size() const;

    VectorBase();

private:
    std::map<int32_t, SPVEC> storage;
};