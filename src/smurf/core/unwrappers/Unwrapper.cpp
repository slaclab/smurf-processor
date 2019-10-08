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
    ris::Slave(),
    ris::Master(),
    disable(false),
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
    bp::class_< scu::Unwrapper, scu::UnwrapperPtr,
                bp::bases<ris::Slave,ris::Master>,
                boost::noncopyable >
                ("Unwrapper", bp::init<>())
        .def("setDisable", &Unwrapper::setDisable)
        .def("getDisable", &Unwrapper::getDisable)
    ;
    bp::implicitly_convertible< scu::UnwrapperPtr, ris::SlavePtr  >();
    bp::implicitly_convertible< scu::UnwrapperPtr, ris::MasterPtr >();
}

void scu::Unwrapper::setDisable(bool d)
{
    disable = d;
}

const bool scu::Unwrapper::getDisable() const
{
    return disable;
}

void scu::Unwrapper::reset()
{
    std::vector<output_data_t>(numCh).swap(currentData);
    std::vector<output_data_t>(numCh).swap(previousData);
    std::vector<output_data_t>(numCh).swap(wrapCounter);
}

void scu::Unwrapper::acceptFrame(ris::FramePtr frame)
{
    rogue::GilRelease noGil;

    // Acquire lock on frame.
    ris::FrameLockPtr lock{frame->lock()};

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


    if (disable)
    {
        // If the processing block is disabled, we still need to cast the
        // input data to the output data type, just without unwrapping
        for(std::size_t i{0}; i < numCh; ++i)
            helpers::setWord<output_data_t>(outFrameIt, i,
                static_cast<output_data_t>(helpers::getWord<input_data_t>(inFrameIt, i)));
    }
    else
    {
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
            helpers::setWord<output_data_t>(outFrameIt, i, currentData.at(i) + wrapCounter.at(i));
        }

        // Now the current Data vector will be the previous data vector, so swap then
        previousData.swap(currentData);
    }

    // Send the frame to the next slave.
    sendFrame(outFrame);
}