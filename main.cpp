#include <iostream>
#include <string>
#include "HashMap.hpp"
#include <cassert>

int main(){
    HashMap<std::string, int> m;
    for (int i = 0; i < 100; ++i)
        m["k" + std::to_string(i)] = i;

    size_t count = 0;
    long sum = 0;
    for (auto &[k, v] : m)
    {
        ++count;
        sum += v;
    }
    assert(count == 100 && sum == 4950);

    for (auto &[k, v] : m)
        v *= 2;
    assert(m["k10"] == 20);

    HashMap<int, int> empty;
    assert(empty.begin() == empty.end());
}