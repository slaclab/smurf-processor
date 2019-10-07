#ifndef _SMURF_CORE_COUNTERS_FRAMESTATISTICS_H_
#define _SMURF_CORE_COUNTERS_FRAMESTATISTICS_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Frame Statistics Module
 * ----------------------------------------------------------------------------
 * File          : FrameStatistics.h
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Frame Statistics Class.
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

#include <iostream>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameLock.h>
#include <rogue/interfaces/stream/FrameIterator.h>
#include <rogue/GilRelease.h>
#include "smurf/core/common/BaseSlave.h"
#include "smurf/core/common/BaseMaster.h"
#include "smurf/core/common/SmurfHeader.h"

namespace bp  = boost::python;
namespace ris = rogue::interfaces::stream;
namespace sccommon = smurf::core::common;   // scc would conflict with smurf::core::counters

namespace smurf
{
    namespace core
    {
        namespace counters
        {
            class FrameStatistics;
            typedef boost::shared_ptr<FrameStatistics> FrameStatisticsPtr;

            class FrameStatistics : public sccommon::BaseSlave, public sccommon::BaseMaster
            {
            public:
                FrameStatistics();
                ~FrameStatistics() {};

                static FrameStatisticsPtr create();

                static void setup_python();

                // Get number of lost frames
                const std::size_t getFrameLossCnt() const;

                // Get the number of out-of-order frames
                const std::size_t getFrameOutOrderCnt() const;

                // This will be call by the BaseSlave class after updating
                // the base counters
                virtual void rxFrame(ris::FramePtr frame);

                // Clear the Rx counter. Override the base class implementation to
                // clear all the other counters with the same command.
                virtual void clearRxCnt();

            private:
                bool        firstFrame;        // Flag to indicate we are processing the first frame
                std::size_t frameLossCnt;      // Number of frame lost
                std::size_t frameOutOrderCnt;  // Counts the number of times we received an out-of-order frame
                std::size_t frameNumber;       // Current frame number
                std::size_t prevFrameNumber;   // Last frame number
            };
        }
    }
}

#endif