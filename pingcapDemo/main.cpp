#include "HashTable.hpp"
#include <vector>
#include <iostream>
#include <chrono>

static const size_t BatchSize = 1 << 16;
bool isGenerate;

int main()
{
    FILE* filep = fopen("dat", "rb");
    if (!filep) {
        fprintf(stderr, "Data file open failed\n");
        exit(0);
    }
    FILE* hstbp = fopen("index", "rb");

    if (!hstbp) {
        fprintf(stderr, "First use\n");
        hstbp = fopen("index", "wb+");
        if (!hstbp) {
            fprintf(stderr, "Index file open failed\n");
            exit(0);
        }
    } else {
        isGenerate = 1;
    }

    Reader reader(filep);
    HashTable hashTable(reader, hstbp);

    if (!isGenerate) {
        auto st = std::chrono::steady_clock::now();

        std::vector<KeyNode> vec;
        vec.reserve(BatchSize);
        auto offset = 0l;
        while (reader.GetEndStatus() == 0) {
            size_t batchCounter = 0;
            while (reader.GetEndStatus() == 0 && batchCounter < BatchSize) {
                auto [k, v] = reader.GetKeyValuePair();
                vec.push_back({ BKDRHash(k.c_str()), offset, k });
                offset = reader.GetOffset();
                ++batchCounter;
            }
            sort(vec.begin(), vec.begin() + batchCounter);
            hashTable.SetBatchKey(vec);
            vec.clear();
        }

        auto ed = std::chrono::steady_clock::now();
        printf("generate index use: %lld ms\nkv count: %ld\n",
            std::chrono::duration_cast<std::chrono::milliseconds>(ed - st).count(),
            reader.GetKVCount());
    }

    auto st = std::chrono::steady_clock::now();

    printf("res: %s\n", hashTable.GetValue(std::string("cw")).c_str());

    auto ed = std::chrono::steady_clock::now();
    printf("per query use: %lld ¦Ìs\n",
        std::chrono::duration_cast<std::chrono::microseconds>(ed - st).count());

    return 0;
}