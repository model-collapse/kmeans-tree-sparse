#include "sparse_kmeans.hpp"

#include <stdlib.h>
#include <string>
#include <set>
#include <omp.h>
#include <algorithm>

SparseKMeansModel::SparseKMeansModel(size_t k, size_t iterations, bool exclusive, const char* initiator) {
    this->_k = k;
    this->_iters = iterations;
    this->_exclusive = exclusive;
    this->_init_mode = initiator;   
}

SparseKMeansModel::SparseKMeansModel(const SparseKMeansModel& t) {
    this->_k = t._k;
    this->_iters = t._iters;
    this->_exclusive = t._exclusive;
    this->_init_mode = t._init_mode;
}
    
int32_t SparseKMeansModel::fit(const std::vector<SPVEC>& samples) {
    initialize_centers();
    this->_samples = &samples;

    if (this->_exclusive) {
        this->_assignment.resize(samples.size());
        this->_hist.resize(this->_k);
    } else {

    }
    
    for (int32_t i = 0; i < this->_iters; i++) {
        int32_t res = this->iteration();
        if (EXK_END == res) {
            break;
        } else if (EXK_FAIL == res) {
            return EXK_FAIL;
        }
    }

    // defer
    this->_samples = NULL;

    return EXK_SUC;
}

int32_t SparseKMeansModel::predict(const SPVEC& x) {
    // predict should be single thread
    TSVAL m = std::numeric_limits<TSVAL>::lowest();
    int32_t ret = 0;
    for (auto iter = this->_centers.begin(); iter != this->_centers.end(); iter++) {
        TSVAL s = boost::numeric::ublas::inner_prod(x, *iter);
        if (m < s) {
            m = s;
            ret = iter - this->_centers.begin();
        }
    }

    return ret;
} 

int32_t SparseKMeansModel::iteration() {
    int32_t res = this->kmeans_e_step();
    if (EXK_FAIL == res) {
        return EXK_FAIL;
    } else if (EXK_END == res) {
        return EXK_END;
    }

    if (EXK_FAIL == this->kmeans_m_step()) {
        return EXK_FAIL;
    }

    return EXK_SUC;
}

// calculate centers
int32_t SparseKMeansModel::kmeans_m_step() {
    if (this->_exclusive) { 
        #pragma omp parallel
        for (int32_t i = 0; i < this->_k; i++) {
            DSVEC& v = this->_centers[i];
            std::fill(v.begin(), v.end(), 0);
            this->_hist[i] = 0;
        }

        #pragma omp parallel
        for (int32_t i = 0; i < this->_samples->size(); i++) {
            int32_t cid = this->_assignment[i];
            DSVEC& cnt = this->_centers[cid];
            cnt += this->_samples->at(i);
            this->_hist[cid]++;
        }

        for (int32_t i = 0; i < this->_k; i++) {
            DSVEC& v = this->_centers[i];
            v /= this->_hist[i];
        }
    } else {
        // not implememted yet
    }

    return EXK_SUC;
}

bool assignment_changed(const std::vector<int32_t>& assa, const std::vector<int32_t>& assb) {
    return assa == assb;
}

// get center assignment
int32_t SparseKMeansModel::kmeans_e_step() {
    if (this->_exclusive) {
        std::vector<int32_t> new_assignment;
        new_assignment.resize(this->_samples->size());

        bool failed = false;
        #pragma omp parallel
        for (int32_t i = 0; i < _samples->size(); i++) {
            int32_t cid = this->predict(this->_samples->at(i));
            if (EXK_FAIL == cid) {
                failed = true;
                break;
            } else {
                new_assignment[i] = cid;
            }
        }

        if (failed) {
            return EXK_FAIL;
        }

        if (!assignment_changed(new_assignment, this->_assignment)) {
            return EXK_END;
        }

        return EXK_SUC;
    } else {
        // not implememted yet
    }

    return EXK_SUC;
}

int32_t SparseKMeansModel::initialize_centers() {
    if (this->_init_mode == "kmeans++") {
        std::vector<TSVAL> scs;
        scs.resize(this->_samples->size());

        int32_t first = rand() % this->_samples->size();
        this->_centers.clear();
        
        SPVEC last_center = this->_samples->at(first);
        
        while (this->_centers.size() < this->_k) {
            this->_centers.push_back((DSVEC)last_center);
            
            #pragma omp parallel for
            for (int32_t i = 0; i < this->_samples->size(); i++){
                scs[i] = 1.0f / boost::numeric::ublas::inner_prod(this->_samples->at(i), last_center);
            }

            // integral
            for (int32_t i = 1; i < this->_samples->size(); i++){
                scs[i] += scs[i-1];
            }

            float seed = (float)rand() / RAND_MAX;
            seed *= *scs.rbegin();

            auto pick = std::lower_bound(scs.begin(), scs.end(), seed);
            last_center = this->_samples->at(pick-scs.begin());
        }
    } else if (this->_init_mode == "random") {
        std::set<int32_t> cids;
        while (cids.size() < this->_k) {
            cids.insert(rand() % this->_samples->size());
        }

        this->_centers.resize(this->_k);
        int32_t t = 0;
        for (auto iter = cids.begin(); iter != cids.end(); iter++) {
            this->_centers[t] = this->_samples->at(*iter);
            t++;
        }
    } else {
        return EXK_FAIL;
    }

    return EXK_SUC;
}



