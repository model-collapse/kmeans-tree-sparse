#ifndef SPARSE_KMEANS_TREE_HPP
#define SPARSE_KMEANS_TREE_HPP
#include <vector>
#include "sparse_kmeans.hpp"
#include "payload.hpp"

struct KMeansNode {
    // Payload
    LeafPayLoad* storage;
    std::vector<KMeansNode*> children;
    int32_t count;

    // Model
    SparseKMeansModel* model;
};

class SparseKMeansTree {
private:
    KMeansNode* _root;
    int32_t _max_node_size;
    
    int32_t fit(const std::vector<SPVEC>& training_samples);
    int32_t fit_node(KMeansNode* n, const std::vector<SPVEC>& training_samples);
    bool is_leaf(const KMeansNode* n) const {
        return n->children.size() == 0;
    };

    std::vector<KMeansNode*> _search_for_path(const SPVEC& v) const;
    int32_t _search_for_path(const SPVEC& v, KMeansNode* entry, std::vector<KMeansNode*>& res) const;

    LeafPayLoad* _sample_payload;
    void dispose_sub_tree(KMeansNode* n);

public:
    SparseKMeansTree(LeafPayLoad* sample_payload,
                     const std::vector<SPVEC>& training_samples, 
                     int32_t max_node_size = 1000,
                     int32_t k = 100,
                     int32_t iterations = 1000, 
                     bool exclusive = false, 
                     const char* initiator="kmeans++"
                     );

    const LeafPayLoad* search_for_leaf(const SPVEC& v);
    std::vector<const KMeansNode*> search_for_path(const SPVEC& v) const;
    int32_t insert(int32_t id, const SPVEC& v);

    ~SparseKMeansTree();
};

#endif