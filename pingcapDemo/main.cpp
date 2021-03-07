#include "HashTable.hpp"
#include <vector>
#include <iostream>
#include <chrono>

static const size_t BatchSize = 1 << 10;
bool isGenerate;

int main()
{
    FILE* filep = fopen("D:\\pcdemo\\dat", "rt");
    if (!filep) {
        fprintf(stderr, "Data file open failed\n");
        exit(0);
    }
    FILE* hstbp = fopen("D:\\pcdemo\\index", "rb");

    if (!hstbp) {
        fprintf(stderr, "First use\n");
        hstbp = fopen("D:\\pcdemo\\index", "wb+");
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
        std::vector<KeyNode> vec;
        vec.reserve(BatchSize);
        auto offset = 0l;
        while (reader.GetEndStatus() == 0) {
            size_t batchCounter = 0;
            while (reader.GetEndStatus() == 0) {
                auto [k, v] = reader.GetKeyValuePair();
                vec.push_back({ BKDRHash(k.c_str()), offset, k });
                offset = reader.GetOffset();
                ++batchCounter;
            }
            sort(vec.begin(), vec.begin() + batchCounter);
            hashTable.SetBatchKey(vec);
        }
    }

    std::cout << "res: " << hashTable.GetValue(std::string("a")) << std::endl;
    std::cout << "res: " << hashTable.GetValue(std::string("b")) << std::endl;
    std::cout << "res: " << hashTable.GetValue(std::string("c")) << std::endl;
    std::cout << "res: " << hashTable.GetValue(std::string("dd")) << std::endl;

    return 0;
}