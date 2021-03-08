#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include "Reader.hpp"

typedef unsigned long long ull;

static const ull seed = 131;

size_t BKDRHash(const char* str) 
{
    size_t hash = 0;
    for (size_t i = 0; str[i]; ++i)
        hash = hash * seed + str[i];
    return hash;
}

struct KeyNode {
    size_t hashValue;
    off_t fileOffset;
    std::string key;
    KeyNode(const size_t hashValue_, off_t fileOffset_, std::string key_)
        : hashValue(hashValue_)
        , fileOffset(fileOffset_)
        , key(key_) {}
    bool operator<(const KeyNode& node)
    {
        return hashValue < node.hashValue;
    }
};

static const size_t bucketSize = 1 << 23;
static const size_t blockSize = 1 << 21;

static char pathBuffer[20];

class HashTable {

private:
    Reader& reader;

    struct HashTableLinkListNode {
        off_t kvOffset;
        off_t nextNodeOffset;
    };

    std::unique_ptr<off_t[]> bucket;
    size_t Size;

    FILE* hashTableFilePtr;

    std::unique_ptr<char[]> buffer;

    void LoadFileBlock(const off_t fileOffset)
    {
        fseek(hashTableFilePtr, fileOffset & (~(blockSize - 1)), SEEK_SET);

        fread(buffer.get(), elementSize, blockSize, hashTableFilePtr);
    }

    void WriteFileBlock(const off_t fileOffset)
    {
        fseek(hashTableFilePtr, fileOffset & (~(blockSize - 1)), SEEK_SET);

        fwrite(buffer.get(), elementSize, blockSize, hashTableFilePtr);
        fflush(hashTableFilePtr);
    }

    void GetHashNodeFromFile(const off_t fileOffset, HashTableLinkListNode& node)
    {
        LoadFileBlock(fileOffset);
        off_t offset = fileOffset & (blockSize - 1);
        memcpy(&node, buffer.get() + offset, sizeof(HashTableLinkListNode));
    }

    void SetHashNodeToFile(const off_t fileOffset, HashTableLinkListNode& node)
    {
        LoadFileBlock(fileOffset);
        off_t offset = fileOffset & (blockSize - 1);
        memcpy(buffer.get() + offset, &node, sizeof(HashTableLinkListNode));
        WriteFileBlock(fileOffset);
    }

    std::pair<HashTableLinkListNode, off_t> FindNode(const std::string& key)
    {
        size_t hashValue = BKDRHash(key.c_str()) % bucketSize;
        off_t curPtr = (bucket.get())[hashValue];
        while (curPtr != -1) {

            HashTableLinkListNode node;
            GetHashNodeFromFile(curPtr, node);

            auto [key_, value_] = std::move(reader.GetKeyValuePairByOffset(node.kvOffset));

            if (key == key_) {
                return std::pair(node, curPtr);
            }

            curPtr = node.nextNodeOffset;
        }
        return std::pair(HashTableLinkListNode({ -1, -1 }), curPtr);
    }

public:
    std::string GetValue(const std::string& key)
    {
        HashTableLinkListNode node = FindNode(key).first;
        if (node.kvOffset == -1)
            return std::move(std::string(""));
        return std::move(reader.GetKeyValuePairByOffset(node.kvOffset).second);
    }

    void SetBatchKey(std::vector<KeyNode>& keySet)
    {
        for (auto& keyNode : keySet) {
            auto Iter = FindNode(keyNode.key);
            if (Iter.second != -1) {
                Iter.first.kvOffset = keyNode.fileOffset;
                SetHashNodeToFile(Iter.second, Iter.first);
            } else {
                HashTableLinkListNode node = { keyNode.fileOffset, bucket.get()[keyNode.hashValue % bucketSize] };
                off_t offset = bucketSize * sizeof(off_t) + Size * sizeof(HashTableLinkListNode);
                SetHashNodeToFile(offset, node);

                bucket.get()[keyNode.hashValue % bucketSize] = offset;
                ++Size;
            }
        }
        fseek(hashTableFilePtr, 0, SEEK_SET);
        fwrite(bucket.get(), sizeof(off_t), bucketSize, hashTableFilePtr);
        fflush(hashTableFilePtr);
    }

    HashTable(Reader& inReader, FILE* filePointer)
        : reader(inReader)
        , bucket(new off_t[bucketSize])
        , Size(0)
        , hashTableFilePtr(filePointer)
        , buffer(new char[blockSize]) 
    {
        fseek(hashTableFilePtr, 0, SEEK_END);
        off_t len = ftell(hashTableFilePtr);
        fseek(hashTableFilePtr, 0, SEEK_SET);
        if (len == 0) { 
            memset(bucket.get(), 0xff, sizeof(off_t) * bucketSize);
            fwrite(bucket.get(), sizeof(off_t), bucketSize, hashTableFilePtr);
            fflush(hashTableFilePtr);
        } else {
            fread(bucket.get(), sizeof(off_t), bucketSize, hashTableFilePtr);
        }
    }

    ~HashTable()
    {
        fclose(hashTableFilePtr);
    }
};