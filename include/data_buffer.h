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

    // Get a pointer to the next available buffer slot, ready to be written to.
    SmurfPacket    getWritePtr();

    // Get a pointer to the next available data packet, ready to be processed.
    SmurfPacket_RO getReadPtr();

    // Call after a new packet is fully written into the buffer. The writing
    // pointer will be move forward to the next empty cell in the buffer.
    void doneWriting();

    // Call after a packet is fully processed. The reading pointer
    // will be move forward to the next cell in the buffer.
    void doneReading();

    // Get buffer empty status
    const bool               isEmpty() const;

    // Get buffer full status
    const bool               isFull() const;

    // Get number of read overflows
    const int                getROFCnt() const;

    // Get number of write overflows
    const int                getWOFCnt() const;

    // Clear overflow counters
    void                     clearOFCnts();

    // Get buffer size
    const std::size_t        getSize() const;

    // Get a pointer to the buffer mutex
    std::mutex*              getMutex();

    // Get a pointer to the new data ready conditional variable
    std::condition_variable* getDataReady();

    // Print the buffer statistics information
    void printStatistic() const;

private:
    std::size_t size;                               // Buffer size
    std::vector<SmurfPacket> data;                  // Raw buffer data
    std::vector<SmurfPacket>::iterator readPtr;     // Read iterator
    std::vector<SmurfPacket>::iterator writePtr;    // Write iterator
    bool full;                                      // Buffer full flag
    bool empty;                                     // Buffer empty flag
    int writeCnt;                                   // Write operation counter
    int readCnt;                                    // Read operation counter
    int WOFCnt;                                     // Write overflow counter
    int ROFCnt;                                     // Read overflow counter
    std::mutex              mutex;                  // Buffer mutex
    std::condition_variable dataReady;              // New data ready conditional variable
};

#endif