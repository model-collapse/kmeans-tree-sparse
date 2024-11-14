#ifndef TOPK_HPP
#define TOPK_HPP
#include <vector>
#include <queue>

template <typename TID, typename TVAL>
struct CompareByValue {
    bool operator() (const std::pair<TID, TVAL>& a, const std::pair<TID, TVAL>& b) {
        return a.second < b.second;
    }
};

template <class TID, class TVAL>
class Topk {
public:
    Topk(size_t k) : _k(k) {}
    void insert(TID id, TVAL value) {
        if (_queue.size() < _k) _queue.push(std::make_pair(id, value));
        else if (value < _queue.top().second) {
            _queue.pop(); _queue.push(std::make_pair(id, value)); 
        }
    }
    
    void finalize(std::vector<std::pair<TID, TVAL>>& result) {
        result.resize(_queue.size());
        while (_queue.size()) {
            result[_queue.size() - 1] = _queue.top();
            _queue.pop();
        }
    }
  
private:
    size_t _k;
    std::priority_queue<std::pair<TID, TVAL>, std::vector<std::pair<TID, TVAL>>, CompareByValue<TID, TVAL>> _queue;
};

#endif