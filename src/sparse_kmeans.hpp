#ifndef SPARSE_KMEANS_HPP
#define SPARSE_KMEANS_HPP
#include <stdint.h>
#include <stdio.h>
#include "sparse.hpp"
#include <vector>

#define EXK_FAIL -1
#define EXK_END 1
#define EXK_SUC 0

class SparseKMeansModel {
private:
    std::vector<DSVEC> _centers;
    std::vector<size_t> _hist;
    size_t _k;
    size_t _iters;
    bool _exclusive;
    std::string _init_mode;

    // training premise will be cleared after training is done
    std::vector<int32_t> _assignment;
    std::vector<SPVEC> _u;
    const std::vector<SPVEC>* _samples;

    int32_t iteration();
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

    const std::vector<SPVEC>& get_u() const {
        return this->_u;
    }

    const int32_t clean_training_outcome() {
        this->_assignment.clear();
        this->_u.clear();
        this->_samples = NULL;
        return EXK_SUC;
    }

    SparseKMeansModel(size_t k, size_t iterations = 1000, bool exclusive = true, const char* initiator = "kmeans++");
    SparseKMeansModel(const SparseKMeansModel& t);
    int32_t fit(const std::vector<SPVEC>& samples);
    int32_t predict(const SPVEC& x); 

    ~SparseKMeansModel() {
        // Just do nothing
    }
};

#endif