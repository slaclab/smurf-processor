#ifndef _DATA_BUFFER_H_
#define _DATA_BUFFER_H_

#include <iostream>
#include <vector>
#include <condition_variable>
#include <mutex>

#include "smurf_packet.h"

class DataBuffer
{
public:
    DataBuffer(std::size_t s);
    ~DataBuffer();

    SmurfPacket    getWritePtr();
    SmurfPacket_RO getReadPtr();

    void doneWriting();

    void doneReading();

    const bool               isEmpty() const;
    const bool               isFull() const;
    const int                getROFCnt() const;
    const int                getWOFCnt() const;
    void                     clearOFCnts();
    const std::size_t        getSize() const;
    std::mutex*              getMutex();
    std::condition_variable* getDataReady();


    void printStatistic() const;

private:
    std::size_t size;
    std::vector<SmurfPacket> data;

    std::vector<SmurfPacket>::iterator readPtr;
    std::vector<SmurfPacket>::iterator writePtr;
    bool full;
    bool empty;
    int writeCnt;
    int readCnt;
    int WOFCnt;
    int ROFCnt;
    std::mutex              mutex;
    std::condition_variable dataReady;


};

#endif