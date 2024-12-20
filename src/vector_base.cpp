#include "vector_base.hpp"
#include <iostream>
#include <fstream>

const SPVEC& VectorBase::at(int32_t id) const {
    return this->_storage.at(id);
}

void VectorBase::insert(int32_t id, const SPVEC& v) {
    this->_storage[id] = v;
}

size_t VectorBase::size() const {
    return this->_storage.size();
}

std::vector<SPVEC> VectorBase::export_vectors(const std::vector<int32_t>& ids) const {
    std::vector<SPVEC> ret;
    for (auto iter = ids.begin(); iter != ids.end(); iter++) {
        ret.push_back(this->_storage.at(*iter));
    }

    return ret;
}

std::vector<const SPVEC*> VectorBase::get_vectors(const std::vector<int32_t>& ids) const {
    std::vector<const SPVEC*> ret;
    for (auto iter = ids.begin(); iter != ids.end(); iter++) {
        auto vptr = &this->_storage.at(*iter);
        ret.push_back(vptr);
    }

    return ret;
}

const std::map<int32_t, SPVEC> VectorBase::get_map() const {
    return this->_storage;
}

VectorBase::VectorBase() {

}

VectorBase::VectorBase(std::string filename, int32_t dim, STR_HASH_FUNC(f), bool self_inc_id) {
    std::ifstream stream;
    stream.open(filename);
    if (!stream.is_open()) {
        std::cerr << "invalid file" << std::endl;
        return;
    }

    int32_t inc = 0;
    bool sstop = false;
    #pragma omp parallel
    while (!sstop)  {
        int32_t id;
        std::string json_content;
        std::string buf;

        #pragma omp critical
        {
            if (self_inc_id) {        
                std::getline(stream, buf, '\t');
                if (buf.size() == 0) {
                    sstop = true;
                } else {
                    id = std::stol(buf);
                }
            } else {
                id = inc;
                inc++;
            }
      
            std::getline(stream, buf, '\n');
            json_content = buf;
            if (buf.size() == 0) {
                sstop = true;
            }
        }

        if (!sstop) {
            SPVEC vec;
            if (f == NULL) {
                //std::cerr << "here" << std::endl;
                vec = sp_vec_from_string(json_content, dim);
            } else {
                //std::cerr << "there" << std::endl;
                vec = sp_vec_from_string(json_content, dim, f);
            }

            //std::cerr.flush();
            #pragma omp critical
            {
                this->insert(id, vec);
            }
        }
    }

    //std::cerr << "done" << std::endl;
    //std::cerr.flush();
}
