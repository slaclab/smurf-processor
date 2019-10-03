/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Unwrapper
 * ----------------------------------------------------------------------------
 * File          : Unwrapper.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Descuiption :
 *   SMuRF Unwrapper Class.
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
#include "smurf/core/unwrappers/Unwrapper.h"

namespace scu  = smurf::core::unwrappers;

scu::Unwrapper::Unwrapper()
:
    scc::BaseSlave(),
    scc::BaseMaster(),
    currentData(0),
    previousData(0),
    wrapCounter(0)
{
}

scu::UnwrapperPtr scu::Unwrapper::create()
{
    return boost::make_shared<Unwrapper>();
}

// Setup Class in python
void scu::Unwrapper::setup_python()
{
    bp::class_<scu::Unwrapper, scu::UnwrapperPtr, bp::bases<scc::BaseSlave,scc::BaseMaster>, boost::noncopyable >("Unwrapper", bp::init<>())
    ;
    bp::implicitly_convertible< scu::UnwrapperPtr, ris::SlavePtr >();
    bp::implicitly_convertible< scu::UnwrapperPtr, ris::MasterPtr >();
}

void scu::Unwrapper::rxtFrame(ris::FramePtr frame)
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

    // (smart) pointer to the smurf header in the input frame (Read-only)
    SmurfHeaderROPtr smurfHeaderIn(SmurfHeaderRO::create(frame));

    // Get the number of channels in the input frame. The number of output channel will be the same
    std::size_t numCh = smurfHeaderIn->getNumberChannels();

    // Request a new frame, to hold the same payload as the input frame
    std::size_t outFrameSize = SmurfHeader::SmurfHeaderSize + sizeof(output_data_t) * numCh;
    ris::FramePtr outFrame = reqFrame(outFrameSize, true);
    outFrame->setPayload(outFrameSize);

    // Iterator to the input frame
    ris::FrameIterator inFrameIt = frame->beginRead();

    // Iterator to the output frame
    ris::FrameIterator outFrameIt = outFrame->beginWrite();

    // Copy the header from the input frame to the output frame.
    for (std::size_t i{0}; i < SmurfHeader::SmurfHeaderSize; ++i)
            *(++outFrameIt) = *(++inFrameIt);

    // Unwrap data
    for(std::size_t i{0}; i < numCh; ++i)
    {
        currentData.at(i) = static_cast<output_data_t>(helpers::getWord<input_data_t>(inFrameIt, i));

        if ((currentData.at(i) > upperUnwrap) && (previousData.at(i) < lowerUnwrap))
        {
            // Decrement wrap counter
            wrapCounter.at(i) -= stepUnwrap;
        }
        else if ((currentData.at(i) < lowerUnwrap) && (previousData.at(i) > upperUnwrap))
        {
            // Increment wrap counter
            wrapCounter.at(i) += stepUnwrap;
        }

        // Write the final value to the output frame
        helpers::setWord(outFrameIt, i, currentData.at(i) + wrapCounter.at(i));
    }

    // Print a few work to verify the mapping works
    std::cout << "  === MAPPING === " << std::endl;
    std::cout << "INDEX    INPUT FRAME     OUTPUT FRAME" << std::endl;
    std::cout << "=====================================" << std::endl;
    {
        ris::FrameIterator in = frame->beginRead();
        ris::FrameIterator out = outFrame->beginRead();

        in += SmurfHeader::SmurfHeaderSize;
        out += SmurfHeader::SmurfHeaderSize;
        for (std::size_t i{0}; i < 20; ++i)
            std::cout << i << "  " << unsigned(*(in+i)) << "  " << unsigned(*(out+i)) << std::endl;
    }
    std::cout << "=====================================" << std::endl;

    // Now the current Data vector will be the previous data vector, so swap then
    previousData.swap(currentData);

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(outFrame);
}