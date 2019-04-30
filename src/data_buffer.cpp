
#include "data_buffer.h"

DataBuffer::DataBuffer(std::size_t s)
:
size     ( s     ),
full     ( false ),
empty    ( true  ),
writeCnt ( 0     ),
readCnt  ( 0     ),
WOFCnt   ( 0     ),
ROFCnt   ( 0     )
{
    for (std::size_t i(0); i < size; ++i)
        data.push_back(ISmurfPacket::create());

    writePtr = data.begin();
    readPtr = data.begin();

    printf("DataBuffer created of size %zu", size);
    printf("DataBuffeV2.size =  %zu\n", data.size());
};

DataBuffer::~DataBuffer()
{
    printf("DataBuffer destroyed\n");
};

SmurfPacket DataBuffer::getWritePtr()
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
        return *writePtr;
    }
};

// Get a pointer to the next available data packet, ready to be processed.
SmurfPacket_RO DataBuffer::getReadPtr()
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
        return *readPtr;
    }
};


void DataBuffer::doneWriting()
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

void DataBuffer::doneReading()
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

const bool DataBuffer::isEmpty() const
{
    return empty;
};

const bool DataBuffer::isFull() const
{
    return full;
};

const int DataBuffer::getROFCnt() const
{
    return ROFCnt;
};

const int DataBuffer::getWOFCnt() const
{
    return WOFCnt;
};

void DataBuffer::clearOFCnts()
{
    ROFCnt = 0; WOFCnt =0;
};

const std::size_t DataBuffer::getSize() const
{
    return size;
};

std::mutex* DataBuffer::getMutex()
{
    return &mutex;
};

std::condition_variable* DataBuffer::getDataReady()
{
    return &dataReady;
};

void DataBuffer::printStatistic() const
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
