#ifndef _SMURF_CORE_REORDERER_H_
#define _SMURF_CORE_REORDERER_H_

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
#include <rogue/interfaces/stream/Master.h>
#include <rogue/interfaces/stream/Slave.h>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameLock.h>
#include <rogue/interfaces/stream/FrameIterator.h>

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
                const bool getDisable()       const;

                // Get the frame counter
                const std::size_t getFrameCnt() const;

                // Get the last frame size (in bytes)
                const std::size_t getFrameSize() const;

                // Clear all counter
                void clearCnt();

                void acceptFrame(ris::FramePtr frame);

            private:
                bool                 disable;           // Disable flag
                std::size_t          frameCnt;          // Frame counter
                std::size_t          frameSize;         // Last frame size (bytes)
            };
        }
    }
}

#endif