#ifndef _SMURF_CORE_DOWNSAMPLERS_DOWNSAMPLER_H_
#define _SMURF_CORE_DOWNSAMPLERS_DOWNSAMPLER_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Downsampler
 * ----------------------------------------------------------------------------
 * File          : Downsampler.h
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

#include <iostream>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameLock.h>
#include <rogue/interfaces/stream/FrameIterator.h>
#include <rogue/interfaces/stream/Slave.h>
#include <rogue/interfaces/stream/Master.h>
#include <rogue/GilRelease.h>
#include "smurf/core/common/SmurfHeader.h"
#include "smurf/core/common/Helpers.h"

namespace bp  = boost::python;
namespace ris = rogue::interfaces::stream;

namespace smurf
{
    namespace core
    {
        namespace downsamplers
        {
            class Downsampler;
            typedef boost::shared_ptr<Downsampler> DownsamplerPtr;

            // This class implements a general data downsampler by the specific factor.
            class Downsampler : public ris::Slave, public ris::Master
            {
            public:
                Downsampler();
                ~Downsampler() {};

                static DownsamplerPtr create();

                static void setup_python();

                // Disable the processing block. The data
                // will just pass through to the next slave
                void       setDisable(bool d);
                const bool getDisable() const;

                // Get the number of channels being processed
                const std::size_t getNumCh() const;

                // Set/Ger the downsampling factor
                void setFactor(std::size_t f);
                const std::size_t getFactor() const;

                // Reset the downsampler. Resets the sampler counter.
                void reset();

                // Accept new frames
                void acceptFrame(ris::FramePtr frame);

            private:
                bool        disable;   // Disable flag
                std::size_t factor;    // Downsample factor
                std::size_t sampleCnt; // Sample counter
            };
        }
    }
}

#endif