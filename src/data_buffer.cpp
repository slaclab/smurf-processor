
#include "data_buffer.h"

template <typename T>
DataBuffer<T>::DataBuffer( std::size_t s )
:
size(s),
data(s),
readPtr(data.begin()),
writePtr(data.begin()),
full(false),
empty(true),
writeCnt(0),
readCnt(0),
WOFCnt(0),
ROFCnt(0)
{
    printf("DataBuffer created of size %zu\n", size);
};

template <typename T>
DataBuffer<T>::~DataBuffer()
{
    printf("DataBuffer destroyed\n");
};

// Get a pointer to the next empty cell in the buffer, ready to accept a
// new data packet.
template <typename T>
T* DataBuffer<T>::getWritePtr()
{
    // Verify if the buffer is full
    if (full)
    {
        // Increase the write overflow counter and throw exception
        ++WOFCnt;
        throw std::runtime_error("Trying to write when the buffer is full");
    }
    else
    {
        return &(*writePtr);
    }
};

// Get a pointer to the next available data packet, ready to be processed.
template <typename T>
T* DataBuffer<T>::getReadPtr()
{
    // Verify is the buffer is empty
    if (empty)
    {
        // Increase the read overflow counter and throw exception
        ++ROFCnt;
        throw std::runtime_error("Trying to read when the buffer is empty");
    }
    else
    {
        return &(*readPtr);
    }
};

// Call after a new packet is fully written into the buffer. The writing pointer will be move forward
// to the next empty cell in the buffer.
template <typename T>
void DataBuffer<T>::doneWriting()
{
    // Move the iterator forward.
    ++writePtr;

    // Verify is we arrived to the end of the buffer.
    // If so, move the iterator back to the start.
    if (writePtr == data.end())
        writePtr = data.begin();

    // Verify if the buffer is full.
    if (writePtr == readPtr)
        full = true;

    // The buffer is not empty after writing new data into it.
    empty = false;

    // Update write counter
    ++writeCnt;

    // Notify listener that new data is ready to be processed.
    std::unique_lock<std::mutex> lock(mutex);
    dataReady.notify_all();
};

// Call after a packet is fully processed. The reading pointer will be move forward to the
// next cell in the buffer.
template <typename T>
void DataBuffer<T>::doneReading()
{
    // Move the iterator forward.
    ++readPtr;

    // Verify is we arrived to the end of the buffer.
    // If so, move the iterator back to the start.
    if (readPtr == data.end())
        readPtr = data.begin();

    // Verify if the buffer is empty.
    if (readPtr == writePtr)
        empty = true;

    // The buffer is not full after reading data from it.
    full = false;

    // Update read counter
    ++readCnt;
};

template <typename T>
const bool DataBuffer<T>::isEmpty() const
{
    return empty;
};

template <typename T>
const bool DataBuffer<T>::isFull() const
{
    return full;
};

template <typename T>
const int DataBuffer<T>::getROFCnt() const
{
    return ROFCnt;
};

template <typename T>
const int DataBuffer<T>::getWOFCnt() const
{
    return WOFCnt;
};

template <typename T>
void DataBuffer<T>::clearOFCnts()
{
    ROFCnt = 0; WOFCnt =0;
};

template <typename T>
const std::size_t DataBuffer<T>::getSize() const
{
    return size;
};

template <typename T>
std::mutex* DataBuffer<T>::getMutex()
{
    return &mutex;
};

template <typename T>
std::condition_variable* DataBuffer<T>::getDataReady()
{
    return &dataReady;
};

template <typename T>
void DataBuffer<T>::printStatistic() const
{
    std::cout << "------------------------------"                                << std::endl;
    std::cout << "Data Buffer statistics:"                                       << std::endl;
    std::cout << "------------------------------"                                << std::endl;
    std::cout << "Buffer size                     : " << size                    << std::endl;
    std::cout << "Total write operations          : " << writeCnt                << std::endl;
    std::cout << "Total read operations           : " << readCnt                 << std::endl;
    std::cout << "Total write attempts while full : " << WOFCnt                  << std::endl;
    std::cout << "Total read attempts while empty : " << ROFCnt                  << std::endl;
    std::cout << "Buffer 'empty' flag             : " << std::boolalpha << empty << std::endl;
    std::cout << "Buffer 'full' flag              : " << std::boolalpha << full  << std::endl;
    std::cout << "------------------------------"                                << std::endl;
};

template class DataBuffer<uint8_t>;