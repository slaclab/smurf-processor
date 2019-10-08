#ifndef _SMURF_CORE_MAPPERS_SMURFCHANNELMAPPER_H_
#define _SMURF_CORE_MAPPERS_SMURFCHANNELMAPPER_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Channel Mapper
 * ----------------------------------------------------------------------------
 * File          : SmurfChannelMapper.h
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Channel Mapper Class.
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
        namespace mappers
        {
            class SmurfChannelMapper;
            typedef boost::shared_ptr<SmurfChannelMapper> SmurfChannelMapperPtr;

            class SmurfChannelMapper : public ris::Slave, public ris::Master
            {
            public:
                SmurfChannelMapper();
                ~SmurfChannelMapper() {};

                static SmurfChannelMapperPtr create();

                static void setup_python();

                // Disable the processing block. The data
                // will just pass through to the next slave
                void       setDisable(bool d);
                const bool getDisable() const;

                // Set the Channel mask vector
                void setMask(boost::python::list m);

                // Get the number of mapper channels
                const std::size_t getNumCh() const;

                // Accept new frames
                void acceptFrame(ris::FramePtr frame);

            private:

                // Data type to be move
                typedef int16_t data_t;

                // Size of the data type.
                const std::size_t dataSize = sizeof(data_t);

                // This is the maximum number of channel we expect from an input frame.
                static const std::size_t maxNumInCh = 4095;

                // This is the maximum number of channel the output packet can hold.
                // The output frame size will be fixed to this size, even if not all
                // channel are mapped.
                static const std::size_t maxNumOutCh = 528;

                // Private data members
                bool                     disable; // Disable flag
                std::size_t              numCh;   // Number of mapped channels
                std::vector<std::size_t> mask;    // Channel mask file
                std::mutex               mut;     // Mutex
            };
        }
    }
}

#endif