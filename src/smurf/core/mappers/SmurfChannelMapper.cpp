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
    std::cout << "SmurfChannelMapper created" << std::endl;
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
    // the size of the SMuRF packet
    if ( listSize > SmurfHeader::SmurfHeaderLength )
    {
        // This should go to a logger instead
        std::cerr << "ERROR: Trying to set a mask list of length = " << listSize \
                  << ", which is larger that the number of channel in a SMuRF packet = " \
                  <<  SmurfHeader::SmurfHeaderLength << std::endl;

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
        // the number of channel we received in the FW frame
        if (val > 2048)
        {
            // This should go to a logger instead
            std::cerr << "ERROR: mask value at index " << i << " is " << val \
                      << ", which is greater the maximum number of channel in the received frame = " \
                      << 2048 << std::endl;

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
    std::cout << "SmurfChannelMapper. Frame received..." << std::endl;
    std::cout << "Size = " << frame->getPayload() << std::endl;

    // If the processing block is disabled, do not process the frame
    if (isRxDisabled())
    {

        // Send the frame to the next slave.
        // This method will check if the Tx block is disabled, as well
        // as updating the Tx counters
        txFrame(frame);

        return;
    }

    // Request a new frame
    ris::FramePtr newFrame = reqFrame(128, true);

    // Iterator to the input frame
    ris::FrameIterator itIn = frame->beginRead();

    // Iterator to the output frame
    ris::FrameIterator itOut = newFrame->beginWrite();

    // Copy the header from the input frame to the output frame.
    for (std::size_t i{0}; i < 128; ++i)
            *(itOut+1) = *(itIn+1);

    // Set the frame size
    newFrame->setPayload(128);

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(newFrame);
}