/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Frame Statistics Module
 * ----------------------------------------------------------------------------
 * File          : FrameStatistics.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Descciption :
 *   SMuRF Frame Statistics Class.
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
#include "smurf/core/counters/FrameStatistics.h"

namespace scc = smurf::core::counters;

scc::FrameStatistics::FrameStatistics()
:
    sccommon::BaseSlave(),
    sccommon::BaseMaster(),
    firstFrame(true),
    frameLossCnt(0),
    frameOutOrderCnt(0),
    frameNumber(0),
    prevFrameNumber(0)
{
    std::cout << "FrameStatistics created" << std::endl;
}

scc::FrameStatisticsPtr scc::FrameStatistics::create()
{
    return boost::make_shared<FrameStatistics>();
}

// Setup Class in python
void scc::FrameStatistics::setup_python()
{
    bp::class_<scc::FrameStatistics, scc::FrameStatisticsPtr, bp::bases<sccommon::BaseSlave,sccommon::BaseMaster>, boost::noncopyable >("FrameStatistics", bp::init<>())
        .def("getFrameLossCnt",     &FrameStatistics::getFrameLossCnt)
        .def("getFrameOutOrderCnt", &FrameStatistics::getFrameOutOrderCnt)
    ;
    bp::implicitly_convertible< scc::FrameStatisticsPtr, sccommon::BaseSlavePtr  >();
    bp::implicitly_convertible< scc::FrameStatisticsPtr, sccommon::BaseMasterPtr >();
}

void scc::FrameStatistics::clearRxCnt()
{
    sccommon::BaseSlave::clearRxCnt();
    frameLossCnt     = 0;
    frameOutOrderCnt = 0;
}

const std::size_t scc::FrameStatistics::getFrameLossCnt() const
{
    return frameLossCnt;
}

const std::size_t scc::FrameStatistics::getFrameOutOrderCnt() const
{
    return frameOutOrderCnt;
}


void scc::FrameStatistics::rxFrame(ris::FramePtr frame)
{
    std::cout << "FrameStatistics. Frame received..." << std::endl;
    std::cout << "Size = " << frame->getPayload() << std::endl;

    // Only process the frame is the block is enable.
    if (!isRxDisabled())
    {
        // Store the current and last frame numbers
        // - Previous frame number
        prevFrameNumber = frameNumber;  // Previous frame number

        // // - Current frame number
        // union
        // {
        //     uint32_t w;
        //     uint8_t  b[4];
        // } fn;

        // // Get an iterator to the header of the frame
        // ris::FrameIterator it = frame->beginRead();

        // for (std::size_t i{0}; i < 4; ++i)
        //         fn.b[i] = *(it+84+i);

        // frameNumber = fn.w;
        SmurfHeader_RO smurf_header_ro(frame->beginRead());
        frameNumber = smurf_header_ro.getFrameCounter();

        // Check if we are missing frames, or receiving out-of-order frames
        if (firstFrame)
        {
            // Don't compare the first frame
            firstFrame = false;
        }
        else
        {
            // Discard out-of-order frames
            if ( frameNumber < prevFrameNumber )
            {
                ++frameOutOrderCnt;
                return;
            }

            // If we are missing frame, add the number of missing frames to the counter
            std::size_t frameNumberDelta = frameNumber - prevFrameNumber - 1;
            if ( frameNumberDelta )
              frameLossCnt += frameNumberDelta;
        }

        std::cout << "Frame number = " << frameNumber << std::endl;
    }

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(frame);
}