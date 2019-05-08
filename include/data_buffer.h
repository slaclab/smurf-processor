#ifndef _DATA_BUFFER_H_
#define _DATA_BUFFER_H_

#include <iostream>
#include <vector>
#include <deque>
#include <iterator>
#include <algorithm>
#include <condition_variable>
#include <mutex>

#include "smurf_packet.h"

// This class implements a data buffer between new Smurf packet created, and the readers.
// At this moment there are 2 readers: the user custom transmitter, and file writer.
// The implementation is a circular buffer, which will overwrite old data is the readers
// don't process the packets fast enough.
// Readers get read-only (smart) pointer to the data, not a copy of it. The writer on the other
// hand receives a read-write (smart) pointer to the data cell.
class DataBuffer
{
public:
    // Constructor. The arguments are the buffer size (bufSize) and the number of readers (numReaders).
    DataBuffer(std::size_t bufSize, std::size_t numReaders);
    ~DataBuffer();

    // Get a pointer to the next available buffer slot, ready to be written to.
    SmurfPacket    getWritePtr();

    // Get a pointer to the next available data packet, ready to be processed.
    // Argument is the reader index.
    SmurfPacket_RO getReadPtr(std::size_t i);

    // Call after a new packet is fully written into the buffer. The writing
    // pointer will be move forward to the next empty cell in the buffer.
    void doneWriting();

    // Call after a packet is fully processed. The reading pointer
    // will be move forward to the next cell in the buffer.
    // Argument is the reader index.
    void doneReading(std::size_t i);

    // Get buffer empty status for each reader.
    // Argument is the reader index.
    const bool               isEmpty(std::size_t i) const;

    // Get buffer full status
    const bool               isFull() const;

    // Get number of read overflows (number of read tries when buffer is empty) for each reader.
    // Argument is the reader index.
    const std::size_t        getROFCnt(std::size_t i) const;

    // Get number of overwrites (number of time data have been overwrite) for each reader.
    // Argument is the reader index.
    const std::size_t        getOWCnt(std::size_t i) const;

    // Clear counters
    void                     clearCnts();

    // Get buffer size
    const std::size_t        getSize() const;

    // Get the number of readers
    const std::size_t        getNumReaders() const;

    // Get a pointer to the buffer mutex
    std::mutex*              getMutex();

    // Get a pointer to the new data ready conditional variable
    std::condition_variable* getDataReady();

    // Print the buffer statistics information
    void printStatistic() const;

private:
    typedef std::vector<SmurfPacket>::iterator data_it_t; // Iterator data type

    std::size_t              size;          // Buffer size
    std::size_t              numberReaders; // Number of readers
    std::vector<SmurfPacket> data;          // Raw buffer data
    std::vector<data_it_t>   readPtr;       // Read iterator (one for each reader)
    data_it_t                writePtr;      // Write iterator
    std::deque<bool>         full;          // Buffer full flag (one for each reader)
    std::deque<bool>         empty;         // Buffer empty flag (one for each reader)
    std::size_t              writeCnt;      // Write operation counter
    std::vector<std::size_t> readCnt;       // Read operation counter (one for each reader)
    std::vector<std::size_t> OWCnt;         // Overwrite counter (one for each reader)
    std::vector<std::size_t> ROFCnt;        // Read overflow counter (one for each reader)
    std::mutex               mutex;         // Buffer mutex
    std::condition_variable  dataReady;     // New data ready conditional variable
};

#endif