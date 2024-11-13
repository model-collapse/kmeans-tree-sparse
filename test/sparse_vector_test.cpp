#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <string>
#include "sparse.hpp"

TEST_CASE( "[sp_vec_from_string] Simple parse => success") {
    std::string test_json = "{\"1\": 0.798, \"513\": 776.09}";

    SPVEC vec = sp_vec_from_string(test_json, 30000);
    REQUIRE(fabs(vec[1] - 0.798) < 0.0001);
    REQUIRE(fabs(vec[513] - 776.09) < 0.0001);
    REQUIRE(vec[5134] == 0);
    REQUIRE(vec.size() == 30000);
}

TEST_CASE( "[sp_vec_from_string] non-numeric index => fail") {
    std::string test_json = "{\"1\": 0.798, \"sfs\": 776.09}";

    SPVEC vec = sp_vec_from_string(test_json, 30000);
    REQUIRE(vec.size() == 0);
}

int32_t parse_xy(std::string v) {
    if (v == "x") {
        return 0;
    } else if (v == "y") {
        return 1;
    } 
    
    return -1;
}

TEST_CASE( "[sp_vec_from_string] x y hash ==> success") {
    std::string test_json = "{\"x\": 0.798, \"y\": 776.09}";
    SPVEC vec = sp_vec_from_string(test_json, 2, parse_xy);

    REQUIRE(fabs(vec[0] - 0.798) < 0.0001);
    REQUIRE(fabs(vec[1] - 776.09) < 0.0001);
}