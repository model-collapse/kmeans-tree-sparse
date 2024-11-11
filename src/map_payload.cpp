#include "map_payload.hpp"

MapPayLoad::MapPayLoad(VectorBase* base): _vec_base(base) {

}

size_t MapPayLoad::size() {
    return this->_scores.size();
}

int32_t MapPayLoad::insert(int32_t id, TSVAL weight, const SPVEC& v) {
    this->_scores.insert_element(id, weight);
}

std::vector<SPVEC> MapPayLoad::get_all_vectors() {
    std::vector<SPVEC> ret;
    for (auto iter = this->_scores.begin(); iter != this->_scores.end(); iter++) {
        ret.push_back(this->_vec_base->vec_at(iter.index()));
    }
}

std::set<int32_t> MapPayLoad::get_all_ids() {
    std::set<int32_t> ids;
    for (auto iter = this->_scores.begin(); iter != this->_scores.end(); iter++) {
        ids.insert(iter.index());
    }

    return ids;
}

LeafPayLoad* MapPayLoad::new_payload() {

}

void MapPayLoad::dispose(LeafPayLoad** t) {

}