#ifndef SPARSE_HPP
#define SPARSE_HPP
#include <stdio.h>
#include <iostream>
#include <json.h>
#include <boost/numeric/ublas/vector_sparse.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/blas.hpp>

#define TSVAL float
#define SPVEC boost::numeric::ublas::mapped_vector<TSVAL>
#define DSVEC boost::numeric::ublas::vector<TSVAL>
#define SPMTX boost::numeric::ublas::mapped_matrix<TSVAL>

#define STR_HASH_FUNC(n) int32_t (*n)(std::string)


int32_t initialize_vocabulary(const char* path);

inline TSVAL spase_dense_dot(const TSVAL* dense, int32_t dim, const SPVEC& sparse) {
    TSVAL dsum = 0;
    for (auto iter = sparse.begin(); iter != sparse.end(); iter++) {
        dsum += *iter * dense[iter.index()];
    }

    return dsum;
}

inline SPVEC sp_vec_from_string(std::string jsf, int32_t dim) {
    SPVEC ret(dim);
    nlohmann::json obj = nlohmann::json::parse(jsf);
    for (auto iter = obj.begin(); iter != obj.end(); iter++) {
        int32_t sid;
        try {
            sid = std::stol(iter.key());
        } catch (std::invalid_argument const& ex) {
            std::cerr << "json key is not a number!!" << std::endl;
            SPVEC empty;
            return empty;
        }
        
        ret(sid) = (float)iter.value();
    }

    return ret;
}

inline SPVEC sp_vec_from_string(std::string jsf, int32_t dim, STR_HASH_FUNC(f)) {
    SPVEC ret(dim);
    nlohmann::json obj = nlohmann::json::parse(jsf);
    for (auto iter = obj.begin(); iter != obj.end(); iter++) {
        int32_t sid = f(iter.key());

        ret(sid) = (float)iter.value();
    }

    return ret;
}

inline std::string sp_vec_to_string(const SPVEC& v) {
    std::ostringstream stream;
    stream << "[";

    stream << v.begin().index() << ":" << *v.begin();
    if (v.size() > 1) {
        stream << " ... " << v.rbegin().index() << ":" << *v.rbegin();
    } 

    stream << "]";
    
    return stream.str();
}

#endif 