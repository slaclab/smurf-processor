/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Channel Mapper
 * ----------------------------------------------------------------------------
 * File          : SmurfChannelMapper.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Descmiption :
 *   SMuRF Channel Mapper Class.
 *-----------------------------------------------------------------------------
 * This file is part of the smurf software platform. It is subject to
 * the license terms in the LICENSE.txt file found in the top-level directory
 * of this distribution and at:
    * https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
 * No part of the smurf software platform, including this file, may be
 * copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE.txt file.
 *-----------------------------------------------------------------------------
**/

#include <boost/python.hpp>
#include "smurf/core/mappers/SmurfChannelMapper.h"

namespace scm  = smurf::core::mappers;

scm::SmurfChannelMapper::SmurfChannelMapper()
:
    scc::BaseSlave(),
    scc::BaseMaster(),
    numCh(0),
    mask(0)
{
}

scm::SmurfChannelMapperPtr scm::SmurfChannelMapper::create()
{
    return boost::make_shared<SmurfChannelMapper>();
}

// Setup Class in python
void scm::SmurfChannelMapper::setup_python()
{
    bp::class_<scm::SmurfChannelMapper, scm::SmurfChannelMapperPtr, bp::bases<scc::BaseSlave,scc::BaseMaster>, boost::noncopyable >("SmurfChannelMapper", bp::init<>())
        .def("getNumCh", &SmurfChannelMapper::getNumCh)
        .def("setMask",  &SmurfChannelMapper::setMask)
    ;
    bp::implicitly_convertible< scm::SmurfChannelMapperPtr, scc::BaseSlavePtr >();
    bp::implicitly_convertible< scm::SmurfChannelMapperPtr, scc::BaseMasterPtr >();
}

void scm::SmurfChannelMapper::setMask(boost::python::list m)
{
    std::size_t listSize = len(m);

    // Check if the size of the list, is not greater than
    // the size of the SMuRF packet payload (in words)
    if ( listSize > SmurfPacket::SmurfPacketPayloadLength )
    {
        // This should go to a logger instead
        std::cerr << "ERROR: Trying to set a mask list of length = " << listSize \
                  << ", which is larger that the number of channel in a SMuRF packet = " \
                  <<  SmurfPacket::SmurfPacketPayloadLength << std::endl;

        // Do not update the mask vector.
        return;
    }

    // We will use a temporal vector to hold the new data.
    // New data will be check as it is pushed to this vector. If there
    // are not error, this vector will be swap with 'mask'.
    std::vector<std::size_t> temp;

    for (std::size_t i{0}; i < listSize; ++i)
    {
        std::size_t val = boost::python::extract<std::size_t>(m[i]);

        // Check if the mask value is not greater than
        // the number of channel we received in the incoming frame
        if (val > maxNumInCh)
        {
            // This should go to a logger instead
            std::cerr << "ERROR: mask value at index " << i << " is " << val \
                      << ", which is greater the maximum number of channel we expect from an input frame = " \
                      << maxNumInCh << std::endl;

            // Do not update the mask vector.
            return;
        }

        // A valid number was found. Add it to the temporal vector
        temp.push_back(val);
    }

    // At this point, all element in the mask list are valid.
    // Update the mask vector
    mask.swap(temp);

    // mask = m;
    numCh = mask.size();

    std::cout << "Resulting vector after setting:" << std::endl;
    for (std::vector<std::size_t>::iterator it = mask.begin(); it != mask.end(); ++it)
        std::cout << *(it) << std::endl;
}

const std::size_t scm::SmurfChannelMapper::getNumCh() const
{
    return numCh;
}

void scm::SmurfChannelMapper::rxFrame(ris::FramePtr frame)
{
    // If the processing block is disabled, do not process the frame
    if (isRxDisabled())
    {
        // Send the frame to the next slave.
        // This method will check if the Tx block is disabled, as well
        // as updating the Tx counters
        txFrame(frame);

        return;
    }

    // Request a new frame, to hold the header + payload, and set its payload
    // Although the number of active channel can change, and will be indicated in the
    // header of the packet, we will send frames of fix size.
    ris::FramePtr newFrame = reqFrame(SmurfPacket::SmurfPacketSize, true);
    newFrame->setPayload(SmurfPacket::SmurfPacketSize);

    // Iterator to the input frame
    ris::FrameIterator inFrameIt = frame->beginRead();

    // Iterator to the output frame
    ris::FrameIterator outFrameIt = newFrame->beginWrite();

    // Copy the header from the input frame to the output frame.
    for (std::size_t i{0}; i < SmurfHeader::SmurfHeaderSize; ++i)
            *(++outFrameIt) = *(++inFrameIt);

    // Now map the data from the input frame to the output frame according to the map vector
    for (std::vector<std::size_t>::iterator maskIt = mask.begin(); maskIt != mask.end(); ++maskIt)
    {
        // Copy each data word, byte-by-byte
        for (std::size_t i{0}; i < SmurfPacket::SmurfDataWordSize; ++i)
            *(outFrameIt + i) = *(inFrameIt + *maskIt * SmurfPacket::SmurfDataWordSize + i);

        // Move the output frame iterator to the new word cell
        outFrameIt += SmurfPacket::SmurfDataWordSize;
    }

    // Set to zero the rest of the content of the output frame.
    // This is only for convenience, as the header says the number of channel which have
    // valid data. The rest of payload will have only garbage.
    std::fill(outFrameIt, newFrame->endWrite(), 0);

    // Update the number of channel in the header of the output smurf frame
    SmurfHeaderPtr smurfHeaderOut(SmurfHeader::create(newFrame));
    smurfHeaderOut->setNumberChannels(numCh);

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(newFrame);
}