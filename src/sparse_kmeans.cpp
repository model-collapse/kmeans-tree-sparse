#include "sparse_kmeans.hpp"
#include "topk.hpp"

#include <stdlib.h>
#include <string>
#include <set>
#include <omp.h>
#include <algorithm>


SparseKMeansModel::SparseKMeansModel(size_t k, size_t iterations, 
                                     bool exclusive, 
                                     const char* initiator, 
                                     DENSE_SPARSE_DIST_FUNC(dist_func),
                                     SAMPLE_DEGREE_FUNC(degree_func),
                                     float cut_rate) {
    this->_k = k;
    this->_iters = iterations;
    this->_exclusive = exclusive;
    this->_init_mode = initiator;   
    this->_dist_func = dist_func;
    this->_sample_degree_func = degree_func;
    if (this->_dist_func == NULL) {
        //std::cerr << "Distance function is NULL, the clustering algorith will crash!" << std::endl;
    }
    this->_cut_rate = cut_rate;
}

SparseKMeansModel::SparseKMeansModel(const SparseKMeansModel& t) {
    this->_k = t._k;
    this->_iters = t._iters;
    this->_exclusive = t._exclusive;
    this->_init_mode = t._init_mode;
    this->_dist_func = t._dist_func;
    if (this->_dist_func == NULL) {
        //std::cerr << "Distance function is NULL, the clustering algorith will crash!" << std::endl;
    }
    this->_cut_rate = t._cut_rate;
}
    
int32_t SparseKMeansModel::fit(const std::vector<SPVEC>& samples) {
    this->_samples = &samples;
    //std::cerr << "Initializing" << std::endl;
    if (EXK_FAIL == initialize_centers()) {
        return EXK_FAIL;
    }

    if (this->_exclusive) {
        this->_assignment.clear();
        this->_hist.resize(this->_k);
    } else {
        for (auto iter = this->_samples->begin(); iter != this->_samples->end(); iter++) {
            this->_degrees.push_back(this->_sample_degree_func(*iter));
        }

        this->_hist.resize(this->_k);
        this->_u.resize(this->_samples->size());
    }
    
    for (int32_t i = 0; i < this->_iters; i++) {
        //std::cerr << "Iter@" << i << std::endl;
        int32_t res = this->iterate();
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

int32_t SparseKMeansModel::predict(const SPVEC& x, TSVAL* dist) {
    // predict should be single thread
    TSVAL m = std::numeric_limits<TSVAL>::max();
    int32_t ret = 0;
    for (auto iter = this->_centers.begin(); iter != this->_centers.end(); iter++) {
        TSVAL s = this->_dist_func(*iter, x);
        if (m > s) {
            m = s;
            ret = iter - this->_centers.begin();
        }
    }

    if (dist != NULL){
        *dist = m;
    }
    
    return ret;
} 

std::vector<std::pair<int32_t, TSVAL>> SparseKMeansModel::predict(const SPVEC& x, int32_t k) {
    Topk<int32_t, TSVAL> topk(k);
    for (auto iter = this->_centers.begin(); iter != this->_centers.end(); iter++) {
        TSVAL s = this->_dist_func(*iter, x);
        topk.insert(iter - this->_centers.begin(), s);
    }

    std::vector<std::pair<int32_t, TSVAL>> res;
    topk.finalize(res);

    if (res.size() != k) {
        std::cerr << "The topk list size is not k: " << res.size() << " | " << k << std::endl;
    } 

    TSVAL thres = res.begin()->second * this->_cut_rate;
    for (auto iter = res.begin() + 1; iter != res.end(); iter++) {
        if (iter->second > thres) {
            res.resize(iter - res.begin());
        }
    }

    return res;
}

int32_t SparseKMeansModel::iterate() {
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
    //std::cerr << "M Step" << std::endl;

    if (this->_exclusive) {
        //std::cerr << "M Ste reset centers" << std::endl;
        #pragma omp parallel for
        for (int32_t i = 0; i < this->_k; i++) {
            DSVEC& v = this->_centers[i];
            std::fill(v.begin(), v.end(), 0);
            this->_hist[i] = 0;
        }

        //std::cerr << "M adding centers" << std::endl;
        if (this->_samples->size() != this->_assignment.size()) {
            //std::cerr << "assignment size if not equal with sample set size" << std::endl;
            return EXK_FAIL;
        }

        omp_lock_t* locks = new omp_lock_t[this->_centers.size()];
        for (int32_t i = 0; i < this->_centers.size(); i++) {
            omp_init_lock(locks + i);
        }

        #pragma omp parallel for
        for (int32_t i = 0; i < this->_samples->size(); i++) {
            int32_t cid = this->_assignment[i];
            
            omp_set_lock(locks + cid);
            DSVEC& cnt = this->_centers[cid];
            cnt += this->_samples->at(i);
            this->_hist[cid]++;
            omp_unset_lock(locks + cid);
        }

        for (int32_t i = 0; i < this->_centers.size(); i++) {
            omp_destroy_lock(locks + i);
        }
        delete [] locks;

        if(std::find(this->_hist.begin(), this->_hist.end(), 0) != this->_hist.end()) {
            //std::cerr << "There is empty center, clustering failed" << std::endl;
            return EXK_FAIL;
        }

        //std::cerr << "M avg centers" << std::endl;
        #pragma omp parallel for
        for (int32_t i = 0; i < this->_k; i++) {
            DSVEC& v = this->_centers[i];
            v /= this->_hist[i];
        }

        for (int32_t i = 0; i < this->_k; i++) {
            DSVEC& v = this->_centers[i];
            //std::cerr << "p@" << i << "\t x=" << v(0) << "\t y=" << v(1) << std::endl;
        }
    } else {
        //std::cerr << "M Ste reset centers" << std::endl;
        #pragma omp parallel for
        for (int32_t i = 0; i < this->_k; i++) {
            DSVEC& v = this->_centers[i];
            std::fill(v.begin(), v.end(), 0);
            this->_hist[i] = 0;
        }

        omp_lock_t* locks = new omp_lock_t[this->_centers.size()];
        for (int32_t i = 0; i < this->_centers.size(); i++) {
            omp_init_lock(locks + i);
        }

        #pragma omp parallel for
        for (int32_t i = 0; i < this->_samples->size(); i++) {
            auto eus = this->_u[i];
            for (auto iter = eus.begin(); iter != eus.end(); iter++) {
                int32_t cid = iter->first;

                omp_set_lock(locks + cid);
                DSVEC& cnt = this->_centers[iter->first];
                SPVEC v = this->_samples->at(i);
                TSVAL w = 1.0 / (iter->second + 10);

                v *= w;
                cnt += v;
                this->_hist[cid] += w;
                omp_unset_lock(locks + cid);
            }   
        }

        for (int32_t i = 0; i < this->_centers.size(); i++) {
            omp_destroy_lock(locks + i);
        }
        delete [] locks;

        if(std::find(this->_hist.begin(), this->_hist.end(), 0) != this->_hist.end()) {
            std::cerr << "There is empty center, clustering failed" << std::endl;
            return EXK_FAIL;
        }

        //std::cerr << "M avg centers" << std::endl;
        #pragma omp parallel for
        for (int32_t i = 0; i < this->_k; i++) {
            DSVEC& v = this->_centers[i];
            v /= this->_hist[i];
        }

        for (int32_t i = 0; i < this->_k; i++) {
            DSVEC& v = this->_centers[i];
            //std::cerr << "p@" << i << "\t x=" << v(0) << "\t y=" << v(1) << std::endl;
        }
    }

    return EXK_SUC;
}

bool assignment_changed(const std::vector<int32_t>& assa, const std::vector<int32_t>& assb) {
    if (assa.size() != assb.size()) {
        return true;
    }
    return !std::equal(assa.begin(), assa.end(), assb.begin());
}

bool u_changed(const std::vector<std::vector<std::pair<int32_t, TSVAL>>>& ua, 
               const std::vector<std::vector<std::pair<int32_t, TSVAL>>>& ub) {
    for (auto itera = ua.begin(), iterb = ub.begin(); itera != ua.end(); itera++, iterb++) {
        if (itera->size() ==0 || iterb->size() == 0) {
            return false;
        }

        if (itera->at(0) != iterb->at(0)) {
            return false;
        }
    }

    return true;
}

// get center assignment
int32_t SparseKMeansModel::kmeans_e_step() {
    //std::cerr << "E Step" << std::endl;
    if (this->_exclusive) {
        std::vector<int32_t> new_assignment;
        new_assignment.resize(this->_samples->size());

        bool failed = false;
        #pragma omp parallel for
        for (int32_t i = 0; i < _samples->size(); i++) {
            int32_t cid = this->predict(this->_samples->at(i));
            if (EXK_FAIL == cid) {
                failed = true;
            } else {
                new_assignment[i] = cid;
            }
        }

        if (failed) {
            return EXK_FAIL;
        }

        for (auto iter = new_assignment.begin(); iter != new_assignment.end(); iter++) {
            //std::cerr << *iter << ",";
        }
        //std::cerr << std::endl;

        //std::cerr << "E comparing" << std::endl;
        if (!assignment_changed(new_assignment, this->_assignment)) {
            //std::cerr << "E comparing not changed" << std::endl;
            return EXK_END;
        }
        //std::cerr << "E comparing done" << std::endl;

        this->_assignment = new_assignment;
        return EXK_SUC;
    } else {
        std::vector<std::vector<std::pair<int32_t, TSVAL>>> nu;
        nu.resize(this->_samples->size());
       
        bool failed = false;
        #pragma omp parallel for
        for (int32_t i = 0; i < _samples->size(); i++) {
            auto top_match = this->predict(this->_samples->at(i), this->_degrees[i]);
            nu[i] = top_match;
        }

        //std::cerr << "E comparing" << std::endl;
        int32_t rett = EXK_SUC;
        if (u_changed(this->_u, nu)) {
            //std::cerr << "E comparing not changed" << std::endl;
            rett = EXK_END;
        }   
        //std::cerr << "E comparing done" << std::endl;

        this->_u = nu;
        return rett;
    }

    return EXK_SUC;
}

int32_t SparseKMeansModel::initialize_centers() {
    if (this->_init_mode == "kmeans++") {
        std::vector<TSVAL> scs;
        scs.resize(this->_samples->size());

        int32_t first = rand() % this->_samples->size();
        //std::cerr << "first = " << first << std::endl;
        this->_centers.clear();    
        DSVEC last_center = this->_samples->at(first);
        
        //std::cerr << "start selecting...." << std::endl;
        while (this->_centers.size() < this->_k) {
            this->_centers.push_back(last_center);
            
            #pragma omp parallel for
            for (int32_t i = 0; i < this->_samples->size(); i++){
                scs[i] = this->_dist_func(this->_samples->at(i), last_center);
            }

            // integral
            for (int32_t i = 1; i < this->_samples->size(); i++){
                scs[i] += scs[i-1];
            }

            float seed = (float)rand() / RAND_MAX;
            seed *= *scs.rbegin();
            //std::cerr << "seed = " << seed  << "; max = " << *scs.rbegin() << std::endl;

            auto pick = std::lower_bound(scs.begin(), scs.end(), seed);
            last_center = this->_samples->at(pick-scs.begin());
        }

        for (int32_t i = 0; i < this->_k; i++) {
            DSVEC& v = this->_centers[i];
            //std::cerr << "p@" << i << "\t x=" << v(0) << "\t y=" << v(1) << std::endl;
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

int32_t constant_degree(const DSVEC& d) {
    return 1;
}

std::string SparseKMeansModel::to_string() const {
    std::ostringstream stream;
    stream << "{\"centers\": [";
    
    stream << "@0:" << sp_vec_to_string(this->_centers[0]);
    if (this->_centers.size() > 2) {
        stream << " ... " << "@" << this->_centers.size() - 1 << ":" << sp_vec_to_string(this->_centers[this->_centers.size() - 1]);
    } else if (this->_centers.size() == 2) {
        stream << " , " << "@" << this->_centers.size() - 1 << ":" << sp_vec_to_string(this->_centers[this->_centers.size() - 1]);
    }
    
    stream << "]}";

    return stream.str();
}

TSVAL inversed_dense_sparse_dot(const DSVEC& d, const SPVEC& v) {
    return 1.0 / (boost::numeric::ublas::inner_prod(d, v) + 0.000000001);
}

TSVAL dense_sparse_l2_distance_sq(const DSVEC& d, const SPVEC& v) {
    SPVEC tmp = v;
    tmp -= d;

    return boost::numeric::ublas::inner_prod(tmp, tmp);
}

TSVAL dense_sparse_l2_distance(const DSVEC& d, const SPVEC& v) {
    return sqrt(dense_sparse_l2_distance_sq(d, v));
}

