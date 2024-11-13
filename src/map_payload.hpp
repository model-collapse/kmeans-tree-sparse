#include "payload.hpp"
#include "sparse.hpp"
#include "vector_base.hpp"
#include <map>

class MapPayLoad : public LeafPayLoad {
public:
    size_t size();
    int32_t insert(int32_t id, TSVAL weight, const SPVEC& v);
    std::vector<SPVEC> get_all_vectors();
    std::set<int32_t> get_all_ids();

    LeafPayLoad* new_payload();
    void dispose(LeafPayLoad** t);

    MapPayLoad(VectorBase* base, size_t max_size);
private:
    size_t _max_size;
    SPVEC _scores;
    VectorBase* _vec_base;
};