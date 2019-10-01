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

namespace scc  = smurf::core::counters;

scc::FrameStatistics::FrameStatistics()
:
    ris::Slave(),
    ris::Master(),
    disable(false),
    frameCnt(0),
    frameSize(0),
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
    bp::class_<scc::FrameStatistics, scc::FrameStatisticsPtr, bp::bases<ris::Slave,ris::Master>, boost::noncopyable >("FrameStatistics", bp::init<>())
        .def("setDisable",          &FrameStatistics::setDisable)
        .def("getDisable",          &FrameStatistics::getDisable)
        .def("getFrameCnt",         &FrameStatistics::getFrameCnt)
        .def("getFrameSize",        &FrameStatistics::getFrameSize)
        .def("getFrameLossCnt",     &FrameStatistics::getFrameLossCnt)
        .def("getFrameOutOrderCnt", &FrameStatistics::getFrameOutOrderCnt)
        .def("clearCnt",            &FrameStatistics::clearCnt)
    ;
    bp::implicitly_convertible< scc::FrameStatisticsPtr, ris::SlavePtr >();
    bp::implicitly_convertible< scc::FrameStatisticsPtr, ris::MasterPtr >();
}

void scc::FrameStatistics::setDisable(bool d)
{
    disable = d;
}

const bool scc::FrameStatistics::getDisable() const
{
    return disable;
}


const std::size_t scc::FrameStatistics::getFrameCnt() const
{
    return frameCnt;
}

const std::size_t scc::FrameStatistics::getFrameSize() const
{
    return frameSize;
}

void scc::FrameStatistics::clearCnt()
{
    frameCnt         = 0;
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


void scc::FrameStatistics::acceptFrame(ris::FramePtr frame)
{
    std::cout << "FrameStatistics. Frame received..." << std::endl;
    std::cout << "Size = " << frame->getPayload() << std::endl;

    // Only process the frame is the block is enable.
    // Otherwise, it will just send to the next slave.
    if (!disable)
    {
        // Store the current and last frame numbers
        // - Previous frame number
        prevFrameNumber = frameNumber;  // Previous frame number

        // - Current frame number
        union
        {
            uint32_t w;
            uint8_t  b[4];
        } fn;

        // Get an iterator to the header of the frame
        ris::FrameIterator it = frame->beginRead();

        for (std::size_t i{0}; i < 4; ++i)
                fn.b[i] = *(it+84+i);

        frameNumber = fn.w;


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

        //Increase the frame counter
        ++frameCnt;

        // Update the last frame size
        frameSize = frame->getPayload();

        std::cout << "Frame number = " << frameNumber << std::endl;
    }

    // Send the same frame to the next slave
    sendFrame(frame);
}