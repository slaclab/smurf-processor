
#include "data_buffer.h"

DataBuffer::DataBuffer(std::size_t bufSize, std::size_t numReaders)
:
size          ( bufSize           ),
numberReaders ( numReaders        ),
full          ( numReaders, false ),
empty         ( numReaders, true  ),
writeCnt      ( 0                 ),
readCnt       ( numReaders, 0     ),
OWCnt         ( numReaders, 0     ),
ROFCnt        ( numReaders, 0     )
{
    for (std::size_t i(0); i < size; ++i)
        data.push_back(ISmurfPacket::create());

    writePtr = data.begin();

    for (std::size_t i(0); i < numberReaders; ++i)
        readPtr.push_back(data.begin());

    printf("DataBuffer created of size %zu, and number of readers %zu", size, numberReaders);
    printf("DataBuffeV2.size =  %zu\n", data.size());
};

DataBuffer::~DataBuffer()
{
    printf("DataBuffer destroyed\n");
};

SmurfPacket DataBuffer::getWritePtr()
{

    // If a reader's buffer is full, update its overwrite counter and move it
    // read pointer forward
    for(std::size_t i(0); i < numberReaders; ++i)
    {
        if (full.at(i))
        {
            ++OWCnt.at(i);
            ++readPtr.at(i);
        }
    }

    return *writePtr;
};

SmurfPacket_RO DataBuffer::getReadPtr(std::size_t i)
{
    // Verify is the buffer is empty
    if (empty.at(i))
    {
        // Increase the read overflow counter and throw exception
        ++ROFCnt.at(i);
        throw std::runtime_error("Trying to read when the buffer is empty");
    }
    else
    {
        return *readPtr.at(i);
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

    // Verify if the buffer is full for each reader.
    for (std::size_t i(0); i < numberReaders; ++i)
    {
        if (writePtr == readPtr.at(i))
            full.at(i) = true;
    }

    // The buffer is not empty after writing new data into it.
    std::fill(empty.begin(), empty.end(), false);

    // Update write counter
    ++writeCnt;

    // Notify listener that new data is ready to be processed.
    std::unique_lock<std::mutex> lock(mutex);
    dataReady.notify_all();
};

void DataBuffer::doneReading(std::size_t i)
{
    // Move the iterator forward.
    ++readPtr.at(i);

    // Verify is we arrived to the end of the buffer.
    // If so, move the iterator back to the start.
    if (readPtr.at(i) == data.end())
        readPtr.at(i) = data.begin();

    // Verify if the buffer is empty for each reader.
    if (readPtr.at(i) == writePtr)
        empty.at(i) = true;

    // The buffer is not full after reading data from it for that reader.
    full.at(i) = false;

    // Update read counter
    ++readCnt.at(i);
};

const bool DataBuffer::isEmpty(std::size_t i) const
{
    return empty.at(i);
};

const bool DataBuffer::isFull() const
{
    return ( std::find(full.begin(), full.end(), true) != full.end() );
};

const std::size_t DataBuffer::getROFCnt(std::size_t i) const
{
    return ROFCnt.at(i);
};

const std::size_t DataBuffer::getOWCnt(std::size_t i) const
{
    return OWCnt.at(i);
};

void DataBuffer::clearCnts()
{
    std::fill(ROFCnt.begin(), ROFCnt.end(), 0);
    std::fill(OWCnt.begin(), OWCnt.end(), 0);
};

const std::size_t DataBuffer::getSize() const
{
    return size;
};

const std::size_t DataBuffer::getNumReaders() const
{
    return numberReaders;
}

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

    std::cout << "Total read operations           : ";
    std::copy(readCnt.begin(), readCnt.end(), std::ostream_iterator<int>(std::cout, ", "));
    std::cout << std::endl;

    std::cout << "Total Overwrites                : ";
    std::copy(OWCnt.begin(), OWCnt.end(), std::ostream_iterator<int>(std::cout, ", "));
    std::cout << std::endl;

    std::cout << "Total read attempts while empty : ";
    std::copy(ROFCnt.begin(), ROFCnt.end(), std::ostream_iterator<int>(std::cout, ", "));
    std::cout << std::endl;

    std::cout << "Buffer 'empty' flag             : " << std::boolalpha;
    std::copy(empty.begin(), empty.end(), std::ostream_iterator<bool>(std::cout, ", "));
    std::cout << std::endl;

    std::cout << "Buffer 'full' flag              : " << std::boolalpha << isFull()  << std::endl;
    std::cout << "------------------------------"                                    << std::endl;
};
