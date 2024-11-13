#ifndef SPARSE_KMEANS_HPP
#define SPARSE_KMEANS_HPP
#include <stdint.h>
#include <stdio.h>
#include "sparse.hpp"
#include <vector>

#define EXK_FAIL -1
#define EXK_END 1
#define EXK_SUC 0

#define DENSE_SPARSE_DIST_FUNC(x) TSVAL(*x)(const DSVEC& d, const SPVEC& v)
#define SAMPLE_DEGREE_FUNC(x) int32_t(*x)(const DSVEC& d)

TSVAL inversed_dense_sparse_dot(const DSVEC& d, const SPVEC& v);
TSVAL dense_sparse_l2_distance_sq(const DSVEC& d, const SPVEC& v);
TSVAL dense_sparse_l2_distance(const DSVEC& d, const SPVEC& v);

int32_t constant_degree(const DSVEC& d);

class SparseKMeansModel {
private:
    std::vector<DSVEC> _centers;
    std::vector<TSVAL> _hist;
    size_t _k;
    size_t _iters;
    bool _exclusive;
    std::string _init_mode;
    DENSE_SPARSE_DIST_FUNC(_dist_func);
    SAMPLE_DEGREE_FUNC(_sample_degree_func);
    float _cut_rate;

    // training premise will be cleared after training is done
    std::vector<int32_t> _assignment;
    std::vector<std::vector<std::pair<int32_t, TSVAL>>> _u;
    std::vector<int32_t> _degrees;
    const std::vector<SPVEC>* _samples;

    int32_t iterate();
    int32_t kmeans_m_step();
    int32_t kmeans_e_step();

    int32_t initialize_centers();

public:
    size_t get_k() const {
        return this->_k;
    }

    bool is_exclusive() const {
        return this->_exclusive;
    }

    const std::vector<DSVEC>& get_centers() const {
        return this->_centers;
    }

    const std::vector<int32_t>& get_assignment() const {
        return this->_assignment;
    }

    const std::vector<std::vector<std::pair<int32_t, TSVAL>>>& get_u() const {
        return this->_u;
    }

    const int32_t clean_training_outcome() {
        this->_assignment.clear();
        this->_u.clear();
        this->_samples = NULL;
        return EXK_SUC;
    }

    SparseKMeansModel(size_t k, size_t iterations = 1000, 
                      bool exclusive = true, const char* initiator = "kmeans++", 
                      DENSE_SPARSE_DIST_FUNC(dist_func) = inversed_dense_sparse_dot,
                      SAMPLE_DEGREE_FUNC(degree_func) = constant_degree, 
                      float cut_rate = 2);
    SparseKMeansModel(const SparseKMeansModel& t);
    int32_t fit(const std::vector<SPVEC>& samples);
    int32_t predict(const SPVEC& x); 
    std::vector<std::pair<int32_t, TSVAL>> predict(const SPVEC& x, int32_t k); 

    ~SparseKMeansModel() {
        // Just do nothing
    }

    std::string to_string() const;
};

#endif