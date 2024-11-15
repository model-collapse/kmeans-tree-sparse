#include "doctest.h"
#include "vector_base.hpp"
#include "sparse_kmeans.hpp"
#include <iostream>

int32_t parse_xy_2(std::string v) {
    if (v == "x") {
        return 0;
    } else if (v == "y") {
        return 1;
    } 
    
    return -1;
}

TEST_CASE("A simple K Means") {
    VectorBase base("kmeans.jsonl", 2, parse_xy_2);
    std::vector<int32_t> ids = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    std::vector<const SPVEC*> vecs = base.get_vectors(ids);

    std::cerr << "init model..." << std::endl;
    SparseKMeansModel model(2, 100, true, "kmeans++");

    std::cerr << "start fitting..." << std::endl;
    int32_t st = model.fit(vecs);
    REQUIRE(st != EXK_FAIL);

    auto assignment = model.get_assignment();
    for (int32_t i = 0; i < 9; i++) {
        REQUIRE(assignment[i] == assignment[i+1]);
        REQUIRE(assignment[i] != assignment[i+10]);
    }
}

TEST_CASE("A simple K Means using L2") {
    VectorBase base("kmeans_2.jsonl", 2, parse_xy_2);
    std::vector<int32_t> ids = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    std::vector<const SPVEC*> vecs = base.get_vectors(ids);

    std::cerr << "init model..." << std::endl;
    SparseKMeansModel model(2, 100, true, "kmeans++", dense_sparse_l2_distance);

    std::cerr << "start fitting..." << std::endl;
    int32_t st = model.fit(vecs);
    REQUIRE(st != EXK_FAIL);

    auto assignment = model.get_assignment();
    for (int32_t i = 0; i < 9; i++) {
        REQUIRE(assignment[i] == assignment[i+1]);
        REQUIRE(assignment[i] != assignment[i+10]);
    }
}

TEST_CASE("A simple K Means when empty center ==> fail") {
    VectorBase base("kmeans_2.jsonl", 2, parse_xy_2);
    std::vector<int32_t> ids = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    std::vector<const SPVEC*> vecs = base.get_vectors(ids);

    std::cerr << "init model..." << std::endl;
    SparseKMeansModel model(2, 100, true, "kmeans++", inversed_dense_sparse_dot);

    std::cerr << "start fitting..." << std::endl;
    int32_t st = model.fit(vecs);
    REQUIRE(st == EXK_FAIL);
}

TEST_CASE("Non exclusive K Means degenerates to exclusive if the K == 1") {
    VectorBase base("kmeans_2.jsonl", 2, parse_xy_2);
    std::vector<int32_t> ids = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    std::vector<const SPVEC*> vecs = base.get_vectors(ids);

    std::cerr << "init model..." << std::endl;
    SparseKMeansModel model(2, 100, false, "kmeans++", dense_sparse_l2_distance);

    std::cerr << "start fitting..." << std::endl;
    int32_t st = model.fit(vecs);
    REQUIRE(st != EXK_FAIL);

    auto u = model.get_u();
    for (int32_t i = 0; i < 9; i++) {
        REQUIRE(u[i][0].first == u[i+1][0].first);
        REQUIRE(u[i][0].first != u[i+10][0].first);
    }
}