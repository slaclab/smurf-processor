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
#include <rogue/interfaces/stream/Master.h>
#include <rogue/interfaces/stream/Slave.h>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameLock.h>
#include <rogue/interfaces/stream/FrameIterator.h>
#include "smurf/core/common/BaseSlave.h"

namespace bp  = boost::python;
namespace ris = rogue::interfaces::stream;
namespace scc = smurf::core::common;

namespace smurf
{
    namespace core
    {
        namespace mappers
        {
            class SmurfChannelMapper;
            typedef boost::shared_ptr<SmurfChannelMapper> SmurfChannelMapperPtr;

            class SmurfChannelMapper : public scc::BaseSlave, public ris::Master
            {
            public:
                SmurfChannelMapper();
                ~SmurfChannelMapper() {};

                static SmurfChannelMapperPtr create();

                static void setup_python();

                void acceptFrame(ris::FramePtr frame);
            };
        }
    }
}

#endif