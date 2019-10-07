/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Downsampler
 * ----------------------------------------------------------------------------
 * File          : Downsamlper.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Data Downsampler Class.
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
#include "smurf/core/downsamplers/Downsampler.h"

namespace scd = smurf::core::downsamplers;

scd::Downsampler::Downsampler()
:
    scc::BaseSlave(),
    scc::BaseMaster(),
    numCh(0),
    factor(1),
    sampleCnt(0)
{
}

scd::DownsamplerPtr scd::Downsampler::create()
{
    return boost::make_shared<Downsampler>();
}

void scd::Downsampler::setup_python()
{
    bp::class_<scd::Downsampler, scd::DownsamplerPtr, bp::bases<scc::BaseSlave,scc::BaseMaster>, boost::noncopyable >("Downsampler",bp::init<>())
        .def("setFactor", &Downsampler::setFactor)
        .def("getFactor", &Downsampler::getFactor)
        .def("getNumCh",  &Downsampler::getNumCh)
    ;
    bp::implicitly_convertible< scd::DownsamplerPtr, scc::BaseSlavePtr  >();
    bp::implicitly_convertible< scd::DownsamplerPtr, scc::BaseMasterPtr >();
}


void scd::Downsampler::setFactor(std::size_t f)
{
    factor = f;

    // When the factor is changed, reset the counter.
    reset();
}

const std::size_t scd::Downsampler::getFactor() const
{
    return factor;
}

const std::size_t scd::Downsampler::getNumCh() const
{
    return numCh;
}

void scd::Downsampler::reset()
{
    sampleCnt = 0;
}

void scd::Downsampler::rxFrame(ris::FramePtr frame)
{
    // Acquire lock on frame.
    rogue::interfaces::stream::FrameLockPtr lock{frame->lock()};

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


    // Increase the sampler counter. Don't do anything until the factor is reach.
    // When the factor is reach, send out the current frame and reset the downsampler.
    if (++sampleCnt < factor)
        return;

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

    // Copy the data
    for (std::size_t i{0}; i < numCh; ++i)
        helpers::setWord<output_data_t>(outFrameIt, i,
            static_cast<output_data_t>(helpers::getWord<input_data_t>(inFrameIt, i)));

    // Reset the downsampler
    reset();

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(outFrame);
}