#ifndef SPARSE_HPP
#define SPARSE_HPP
#include <stdio.h>
#include <boost/numeric/ublas/vector_sparse.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/blas.hpp>

#define TSVAL float
#define SPVEC boost::numeric::ublas::mapped_vector<TSVAL>
#define DSVEC boost::numeric::ublas::vector<TSVAL>

int32_t initialize_vocabulary(const char* path);

inline TSVAL spase_dense_dot(const TSVAL* dense, int32_t dim, const SPVEC& sparse) {
    TSVAL dsum = 0;
    for (auto iter = sparse.begin(); iter != sparse.end(); iter++) {
        dsum += *iter * dense[iter.index()];
    }

    return dsum;
}

#endif 