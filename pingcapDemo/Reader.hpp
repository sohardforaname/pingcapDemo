#pragma once

#include <cstdio>
#include <string>
#include <memory>
#include <bits/stdc++.h>

typedef long long ll;
static const size_t bufferSize = 1 << 21;
static const size_t elementSize = 1;

class Reader {

private:
    off_t count, kvCount;
    std::shared_ptr<char[]> buffer, buffer1;
    char *fileHeadPointer, *fileTailPointer;
    FILE* inputStream;
    bool isEnd, nextKV;

private:
    void ReadBuf()
    {
        fileHeadPointer = buffer.get();

        size_t readSize = fread(fileHeadPointer, elementSize, bufferSize, inputStream);
        fileTailPointer = fileHeadPointer + readSize;
        *fileTailPointer = 0;

        if (fileHeadPointer == fileTailPointer) {
            isEnd = 1;
            nextKV = 0;
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
        if (!isEnd) {
            return *(fileHeadPointer++);
        }
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
        fseek(inputStream, offset & (~(bufferSize - 1)), SEEK_SET);
        ReadBuf();
        fileHeadPointer = buffer.get() + (offset & (bufferSize - 1));
    }

    std::string GetRawData() 
    {
        std::string dat;

        size_t datSize, i = 0;

        SkipSpace();
        if (isEnd)
            return std::move(dat);
        while (IsGraph(Seek())) {
            dat.push_back(Take());
        }

        datSize = std::stoi(dat);
        dat.resize(datSize);
        SkipSpace();
        if (isEnd)
            return std::move(dat);
        while (IsGraph(Seek()) && i != datSize) {
            dat[i++] = Take();
        }
        SkipSpace();

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
        // set status value
        bool endStatus = isEnd;
        bool nextKVStatus = nextKV;
        off_t curFilePtr = ftell(inputStream);
        char* curBufferPtr = fileHeadPointer;
        char* curBUfferPtr1 = fileTailPointer;
        memcpy(buffer1.get(), buffer1.get(), bufferSize);

        isEnd = 0;
        nextKV = 0;
        MoveBufferPointer(offset);
        auto pair = std::move(GetKV());
        //auto pair = std::pair<std::string, std::string>();

        // recover status value
        memcpy(buffer.get(), buffer1.get(), bufferSize);
        fseek(inputStream, curFilePtr, SEEK_SET);
        fileHeadPointer = curBufferPtr;
        fileTailPointer = curBUfferPtr1;

        isEnd = endStatus;
        nextKV = nextKVStatus;

        return std::move(pair);
    }

    Reader(FILE* filePointer)
        : count(0)
        , kvCount(0)
        , inputStream(filePointer)
        , fileHeadPointer(nullptr)
        , fileTailPointer(nullptr)
        , isEnd(0)
        , buffer(new char[bufferSize + 4])
        , buffer1(new char[bufferSize + 4])
        , nextKV(1) 
    {
        fileHeadPointer = buffer.get();
        fileTailPointer = buffer.get();
    }

    ~Reader() 
    {
        fclose(inputStream);
    }

};