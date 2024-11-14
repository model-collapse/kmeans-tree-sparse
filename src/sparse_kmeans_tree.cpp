#include "sparse_kmeans_tree.hpp"
#include <vector>
#include <iostream>
#include <boost/algorithm/string/join.hpp>

SparseKMeansTree::SparseKMeansTree(
    LeafPayLoad* sample_payload,
    const std::vector<SPVEC>& training_samples, 
    int32_t max_node_size,
    int32_t k,
    int32_t iterations, 
    bool exclusive, 
    const char* initiator,
    DENSE_SPARSE_DIST_FUNC(func),
    SAMPLE_DEGREE_FUNC(deg_func),
    float cut_rate) {
    this->_max_node_size = max_node_size;
    this->_root = new KMeansNode;
    this->_root->model = new SparseKMeansModel(k, iterations, exclusive, initiator, func, deg_func, cut_rate);
    this->_root->count = 0;
    this->_root->children.clear();
    this->_sample_payload = sample_payload;
    this->_func = func;

    //std::cerr << "Start fitting..." << std::endl;
    this->fit(training_samples);
}

int32_t SparseKMeansTree::fit(const std::vector<SPVEC>& training_samples) {
    return fit_node(this->_root, training_samples);
}

int32_t SparseKMeansTree::fit_node(KMeansNode* n, const std::vector<SPVEC>& training_samples) {
    //std::cerr << "Fitting..." << std::endl;
    if (training_samples.size() <= this->_max_node_size) {
        // This is leaf node, initialize payload
        n->storage = this->_sample_payload->new_payload();

        return EXK_END;
    }
    
    if (this->_root->model == NULL) {
        return EXK_FAIL;
    }
    
    if (n->model == NULL) {
        n->model = new SparseKMeansModel(*this->_root->model);
    }

    //std::cerr << "Model Fitting..." << std::endl;
    n->model->fit(training_samples);
    //std::cerr << "Model Fitted..." << std::endl;
    if (n->model->is_exclusive()) {
        const std::vector<int32_t>& assignment = n->model->get_assignment();
        for (int32_t i = 0; i < n->model->get_k(); i++) {
            std::vector<SPVEC> segment;
            for (auto iter = assignment.begin(); iter != assignment.end(); iter ++) {
                if (*iter == i) {
                    segment.push_back(training_samples.at(iter - assignment.begin()));
                }
            }

            KMeansNode* nnd = new KMeansNode{NULL, std::vector<KMeansNode*>(), 0, NULL};
            this->fit_node(nnd, segment);
            n->children.push_back(nnd);
        }
    } else {
        // Not implemented
    }
    n->model->clean_training_outcome();

    return EXK_SUC;
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
        //std::cerr << "Child node is NULL in the children list." << std::endl;
        return EXK_FAIL;
    }
    
    res.push_back(entry);
    if (this->is_leaf(entry)) {
        return EXK_END;
    }

    if (entry->model == NULL) {
        //std::cerr << "Non leaf node have NULL model-ptr!" << std::endl;
        return EXK_FAIL;
    }

    int32_t cid = entry->model->predict(v);
    if (cid == EXK_FAIL) {
        //std::cerr << "Modle prediction failed!" << std::endl;
        return EXK_FAIL;
    } else if (cid < 0 || cid >= entry->children.size()) {
        //std::cerr << "Cluster ID is out of the scope of children list." << std::endl;
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

std::string SparseKMeansTree::node_to_string(KMeansNode* n) {
    if (n == NULL) {
        std::string empty;
        return empty;
    }

    std::stringstream stream;
    std::string model_string = n->model != NULL ? n->model->to_string() : "NULL";
    stream << "{\"model\":" << model_string << ", size: " << n->count << "}";
    return stream.str();
}

std::string SparseKMeansTree::to_string() {
    std::vector<std::string> lines;
    this->node_string_traverse(0, true, this->_root, lines);

    return boost::algorithm::join(lines, "\n");
}

void SparseKMeansTree::node_string_traverse(int32_t depth, bool last_child, KMeansNode* n, std::vector<std::string>& lines) {
    std::stringstream stream;
    if (depth == 0) {
        stream << "--";
    } else {
        for (int32_t i = 0; i < depth; i++) {
            stream << "    ";
        }

        if (last_child) {
            stream << "└─";
        } else {
            stream << "├─";
        }
    }

    if (this->is_leaf(n)) {
        stream << "------" << this->node_to_string(n);
    } else {
        stream << "--┬---" << this->node_to_string(n);
    }
    lines.push_back(stream.str());

    for (auto iter = n->children.begin(); iter != n->children.end(); iter++) {
        this->node_string_traverse(depth + 1, n->children.end() - iter == 1, *iter, lines);
    }
}