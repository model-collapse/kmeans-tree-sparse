#include "doctest.h"
#include "vector_base.hpp"
#include "sparse_kmeans_tree.hpp"
#include "map_payload.hpp"
#include <iostream>

int32_t parse_xy_3(std::string v) {
    if (v == "x") {
        return 0;
    } else if (v == "y") {
        return 1;
    } 
    
    return -1;
}

TEST_CASE("A simple K Means Tree in L2") {
    VectorBase base("../data/kmeans_3.jsonl", 2, parse_xy_3);
    std::vector<int32_t> ids;
    for (int32_t i = 0; i < 60; i++) {
        ids.push_back(i);
    }

    std::vector<const SPVEC*> vecs = base.get_vectors(ids);

    MapPayLoad sbrk(&base, 10);
    SparseKMeansTree kmst(&sbrk, vecs, 10, 2, 100, true, "kmeans++", dense_sparse_l2_distance);

    for (auto iter = ids.begin(); iter != ids.end(); iter++) {
        kmst.insert(*iter, base.at(*iter), 1.0);
    }

    std::cout << kmst.to_string() << std::endl;
}