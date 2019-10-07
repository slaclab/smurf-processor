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
    numCh(0),
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
        .def("getNumCh", &Unwrapper::getNumCh)
    ;
    bp::implicitly_convertible< scu::UnwrapperPtr, scc::BaseSlavePtr  >();
    bp::implicitly_convertible< scu::UnwrapperPtr, scc::BaseMasterPtr >();
}

const std::size_t scu::Unwrapper::getNumCh() const
{
    return numCh;
}

void scu::Unwrapper::reset()
{
    std::vector<output_data_t>(numCh).swap(currentData);
    std::vector<output_data_t>(numCh).swap(previousData);
    std::vector<output_data_t>(numCh).swap(wrapCounter);
}

void scu::Unwrapper::rxFrame (ris::FramePtr frame)
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
    std::size_t newNumCh = smurfHeaderIn->getNumberChannels();

    // Verify if the frame size has changed
    if (numCh != newNumCh)
    {

        // Update the number of channels we are processing
        numCh = newNumCh;

        // The new number of channel changes, reset the buffers
        reset();
    }

    // Request a new frame, to hold the same payload as the input frame
    //std::size_t outFrameSize = SmurfHeader::SmurfHeaderSize + sizeof(output_data_t) * numCh;
    // For now we want to keep packet of the same size, so let's do this instead:
    std::size_t outFrameSize = SmurfHeader::SmurfHeaderSize +
        ( ( frame->getPayload() - SmurfHeader::SmurfHeaderSize )/sizeof(input_data_t) ) * sizeof(output_data_t);
    ris::FramePtr outFrame = reqFrame(outFrameSize, true);
    outFrame->setPayload(outFrameSize);

    // Fill the output frame payload with zeros.
    // This is only for convenience, as the header says the number of channel which have
    // valid data. The rest of payload will have only garbage.
    //std::fill(outFrame->beginWrite() + SmurfHeader::SmurfHeaderSize + numCh * sizeof(output_data_t),
    //    outFrame->endWrite(), 0);

    // Iterator to the input frame
    ris::FrameIterator inFrameIt = frame->beginRead();

    // Iterator to the output frame
    ris::FrameIterator outFrameIt = outFrame->beginWrite();

    // Copy the header from the input frame to the output frame.
    outFrameIt = std::copy(inFrameIt, inFrameIt + SmurfHeader::SmurfHeaderSize, outFrameIt);

    // Unwrap the data
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

    // Now the current Data vector will be the previous data vector, so swap then
    previousData.swap(currentData);

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(outFrame);
}