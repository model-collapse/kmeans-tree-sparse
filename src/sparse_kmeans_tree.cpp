#include "sparse_kmeans_tree.hpp"
#include <vector>
#include <iostream>

SparseKMeansTree::SparseKMeansTree(
    LeafPayLoad* sample_payload,
    const std::vector<SPVEC>& training_samples, 
    int32_t max_node_size,
    int32_t k,
    int32_t iterations, 
    bool exclusive, 
    const char* initiator) {
    this->_max_node_size = max_node_size;
    this->_root = new KMeansNode;
    this->_root->model = new SparseKMeansModel(k, iterations, exclusive, initiator);
    this->_root->count = 0;
    this->_root->children.clear();
    this->_sample_payload = sample_payload;

    this->fit(training_samples);
}

int32_t SparseKMeansTree::fit(const std::vector<SPVEC>& training_samples) {
    fit_node(this->_root, training_samples);
}

int32_t SparseKMeansTree::fit_node(KMeansNode* n, const std::vector<SPVEC>& training_samples) {
    if (training_samples.size() < this->_max_node_size) {
        return EXK_END;
    }
    
    if (this->_root->model == NULL) {
        return EXK_FAIL;
    }
    
    if (n->model == NULL) {
        n->model = new SparseKMeansModel(*this->_root->model);
    }

    n->model->fit(training_samples);
    if (n->model->is_exclusive()) {
        const std::vector<int32_t>& assignment = n->model->get_assignment();
        for (int32_t i = 0; i < n->model->get_k(); i++) {
            std::vector<SPVEC> segment;
            for (auto iter = assignment.begin(); iter != assignment.end(); iter ++) {
                if (*iter == i) {
                    segment.push_back(training_samples.at(iter - assignment.begin()));
                }
            }

            KMeansNode* nnd = new KMeansNode;
            this->fit_node(nnd, segment);
            n->children.push_back(nnd);
        }
    } else {
        // Not implemented
    }
    n->model->clean_training_outcome();
}

void SparseKMeansTree::dispose_sub_tree(KMeansNode* n) {
    if (!this->is_leaf(n)) {
        for (auto iter = n->children.begin(); iter != n->children.end(); iter++) {
            dispose_sub_tree(*iter);
        }
    } 

    if (n->storage != NULL) {
        this->_sample_payload->dispose(&n->storage);
    }

    delete n->model;
}

const LeafPayLoad* SparseKMeansTree::search_for_leaf(const SPVEC& v) {
    auto path = this->search_for_path(v);
    return (*path.rbegin())->storage;
}

int32_t SparseKMeansTree::_search_for_path(const SPVEC& v, KMeansNode* entry, std::vector<KMeansNode*>& res) const {
    if (entry == NULL) {
        std::cerr << "Child node is NULL in the children list." << std::endl;
        return EXK_FAIL;
    }
    
    res.push_back(entry);
    if (this->is_leaf(entry)) {
        return EXK_END;
    }

    if (entry->model == NULL) {
        std::cerr << "Non leaf node have NULL model-ptr!" << std::endl;
        return EXK_FAIL;
    }

    int32_t cid = entry->model->predict(v);
    if (cid == EXK_FAIL) {
        std::cerr << "Modle prediction failed!" << std::endl;
        return EXK_FAIL;
    } else if (cid < 0 || cid >= entry->children.size()) {
        std::cerr << "Cluster ID is out of the scope of children list." << std::endl;
        return EXK_FAIL;
    }   

    return _search_for_path(v, entry->children.at(cid), res);
}

std::vector<KMeansNode*> SparseKMeansTree::_search_for_path(const SPVEC& v) const {
    std::vector<KMeansNode*> ret;
    this->_search_for_path(v, this->_root, ret);
    return ret;
}

std::vector<const KMeansNode*> SparseKMeansTree::search_for_path(const SPVEC& v) const {
    std::vector<const KMeansNode*> ret;
    std::vector<KMeansNode*> path = this->_search_for_path(v);
    for (auto iter = path.begin(); iter != path.end(); iter++) {
        ret.push_back(*iter);
    }

    return ret;
}

int32_t SparseKMeansTree::insert(int32_t id, const SPVEC& v) {
    auto path = this->_search_for_path(v);
    for (auto iter = path.begin(); iter != path.end(); iter++) {
        (*iter)->count += 1;
    }

    return EXK_SUC;
}

SparseKMeansTree::~SparseKMeansTree() {
    if (this->_root != NULL){
        this->dispose_sub_tree(this->_root);
    }
}