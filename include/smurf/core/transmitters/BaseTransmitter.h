#ifndef _SMURF_CORE_TRANSMITTERS_BASETRANSMITTER_H_
#define _SMURF_CORE_TRANSMITTERS_BASETRANSMITTER_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Base Transmitter
 * ----------------------------------------------------------------------------
 * File          : BaseTransmitter.h
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Data Base Transmitter Class.
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

namespace bp  = boost::python;
namespace ris = rogue::interfaces::stream;
namespace scc = smurf::core::common;

namespace smurf
{
    namespace core
    {
        namespace transmitters
        {
            class BaseTransmitter;
            typedef boost::shared_ptr<BaseTransmitter> BaseTransmitterPtr;

            class BaseTransmitter : public scc::BaseSlave
            {
            public:
                BaseTransmitter();
                ~BaseTransmitter() {};

                static BaseTransmitterPtr create();

                static void setup_python();

                // This will be call by the BaseSlave class after updating
                // the base counters
                void rxFrame(ris::FramePtr frame);
            };
        }
    }
}

#endif