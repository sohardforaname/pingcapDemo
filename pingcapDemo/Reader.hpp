#pragma once

#include <cstdio>
#include <string>
#include <memory>
#include <bits/stdc++.h>

typedef long long ll;
static const size_t bufferSize = 1 << 23;
static const size_t elementSize = 1;

class Reader {

private:
    off_t count, kvCount;
    std::shared_ptr<char[]> buffer;
    char *fileHeadPointer, *fileTailPointer;
    FILE* inputStream;
    bool isEnd, nextKV;

private:
    void ReadBuf()
    {
        fileHeadPointer = buffer.get();
        fileTailPointer = fileHeadPointer + fread(fileHeadPointer, elementSize, bufferSize, inputStream);
        *fileTailPointer = 0;
        if (fileHeadPointer == fileTailPointer) {
            isEnd = 1;
            return;
        }
        if (nextKV)
            ++count;
    }

    inline bool IsGraph(const int ch)
    {
        return !(ch < 33 || ch > 126);
    }

    char Seek()
    {
        if (!isEnd && fileHeadPointer == fileTailPointer) {
            ReadBuf();
        }
        return *fileHeadPointer;
    }

    char Take() 
    {
        if (!isEnd)
            return *(fileHeadPointer++);
        return 0;
    }

    void SkipSpace()
    {
        while (!IsGraph(Seek()) && !isEnd) {
            Take();
        }
    }

    void MoveBufferPointer(off_t offset) 
    {
        fseek(inputStream, offset & (~((1 << 23) - 1)), SEEK_SET);
        ReadBuf();
        fileHeadPointer = buffer.get() + (offset & ((1 << 23) - 1));
    }

    std::string GetRawData() 
    {
        std::string dat;

        size_t datSize, i = 0;
        SkipSpace();
        while (IsGraph(Seek())) {
            dat.push_back(Take());
        }

        datSize = std::stoi(dat);
        dat.resize(datSize);
        SkipSpace();
        while (IsGraph(Seek()) && i != datSize) {
            dat[i++] = Take();
        }

        return std::move(dat);
    }

    std::pair<std::string, std::string> GetKV() 
    {
        std::string &&key = GetRawData(), &&value = GetRawData();
        return std::move(
            std::pair<std::string, std::string>(
                std::move(key), std::move(value)));
    }

public:

    off_t GetOffset() const
    {
        return (count - 1) * bufferSize + fileHeadPointer - buffer.get();
    }

    off_t GetKVCount() const
    {
        return kvCount;
    }

    bool GetEndStatus() const 
    {
        return isEnd;
    }

    std::pair<std::string, std::string> GetKeyValuePair()
    {
        if (isEnd)
            return std::move(std::pair<std::string, std::string>());
        ++kvCount;
        return std::move(GetKV());
    }

    std::pair<std::string, std::string> GetKeyValuePairByOffset(const off_t offset) 
    {
        bool endStatus = isEnd;
        bool nextKVStatus = nextKV;
        off_t curFilePtr = ftell(inputStream);

        isEnd = 0;
        nextKV = 0;

        MoveBufferPointer(offset);
        auto pair = std::move(GetKV());

        isEnd = endStatus;
        nextKV = nextKVStatus;
        fseek(inputStream, curFilePtr, SEEK_SET);

        return std::move(pair);
    }

    Reader(FILE* filePointer)
        : count(0)
        , kvCount(0)
        , inputStream(filePointer)
        , fileHeadPointer(nullptr)
        , fileTailPointer(nullptr)
        , isEnd(0)
        , buffer(new char[bufferSize])
        , nextKV(1) {}

    ~Reader() {}

};