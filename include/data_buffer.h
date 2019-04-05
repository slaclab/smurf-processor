#ifndef _DATA_BUFFER_H_
#define _DATA_BUFFER_H_

#include <iostream>
#include <vector>
#include <condition_variable>
#include <mutex>

// Buffer of SMuRF packets
template <typename T>
class DataBuffer
{
public:
    DataBuffer(std::size_t d, std::size_t s);

    virtual ~DataBuffer();

    // Get a pointer to the next empty cell in the buffer, ready to accept a
    // new data packet.
    T* getWritePtr();

    // Get a pointer to the next available data packet, ready to be processed.
    T* getReadPtr();

    // Call after a new packet is fully written into the buffer. The writing pointer will be move forward
    // to the next empty cell in the buffer.
    void doneWriting();

    // Call after a packet is fully processed. The reading pointer will be move forward to the
    // next cell in the buffer.
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
    std::size_t depth;
    std::size_t size;
    std::vector< std::vector<T> > data;
    typename std::vector< std::vector<T> >::iterator readPtr;
    typename std::vector< std::vector<T> >::iterator writePtr;
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