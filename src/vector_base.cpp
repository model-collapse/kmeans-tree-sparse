#include "vector_base.hpp"

const SPVEC& VectorBase::vec_at(int32_t id) const {
    return this->_storage.at(id);
}

void VectorBase::insert(int32_t id, const SPVEC& v) {
    this->_storage[id] = v;
}

size_t VectorBase::size() const {
    return this->_storage.size();
}

VectorBase::VectorBase() {

}