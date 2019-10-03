#ifndef _SMURF_CORE_UNWRAPPERS_UNWRAPPER_H_
#define _SMURF_CORE_UNWRAPPERS_UNWRAPPER_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Unwrapper
 * ----------------------------------------------------------------------------
 * File          : Unwrapper.h
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Unwrapper Class.
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
#include "smurf/core/common/BaseSlave.h"
#include "smurf/core/common/BaseMaster.h"
#include "smurf/core/common/SmurfHeader.h"
#include "smurf/core/common/SmurfPacket.h"

namespace bp  = boost::python;
namespace ris = rogue::interfaces::stream;
namespace scc = smurf::core::common;

namespace smurf
{
    namespace core
    {
        namespace unwrappers
        {
            class Unwrapper;
            typedef boost::shared_ptr<Unwrapper> UnwrapperPtr;

            class Unwrapper : public scc::BaseSlave, public scc::BaseMaster
            {
            public:
                Unwrapper();
                ~Unwrapper() {};

                static UnwrapperPtr create();

                static void setup_python();

                // This will be call by the BaseSlave class after updating
                // the base counters
                void rxtFrame(ris::FramePtr frame);

            private:
                std::vector<uint8_t> prevData;  // Data from the previous frame
                // std::vector<> wrap_counter
            };
        }
    }
}

#endif