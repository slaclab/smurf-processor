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
    // Acquire lock on frame.
    rogue::interfaces::stream::FrameLockPtr lock{frame->lock()};

    // Only process the frame is the block is enable.
    if (!isRxDisabled())
    {
        // (smart) pointer to the smurf header in the input frame (Read-only)
        SmurfHeaderROPtr smurfHeaderIn(SmurfHeaderRO::create(frame));

        // Store the current and last frame numbers
        // - Previous frame number
        prevFrameNumber = frameNumber;  // Previous frame number

        // - Current frame number
        frameNumber = smurfHeaderIn->getFrameCounter();

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
    }

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(frame);
}