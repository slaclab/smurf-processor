#ifndef _SMURF_CORE_CONVENTERS_Header2Smurf_H_
#define _SMURF_CORE_CONVENTERS_Header2Smurf_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Header2Smurf
 * ----------------------------------------------------------------------------
 * File          : Header2Smurf.h
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Data Header2Smurf Class.
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
#include "smurf/core/common/Helpers.h"

namespace bp  = boost::python;
namespace ris = rogue::interfaces::stream;
namespace sccommon = smurf::core::common;   // sccommon would conflict with smurf::core::counters


namespace smurf
{
    namespace core
    {
        namespace conventers
        {
            class Header2Smurf;
            typedef boost::shared_ptr<Header2Smurf> Header2SmurfPtr;

            // This class converts the header in the frame to the Smurf Header
            class Header2Smurf : public sccommon::BaseSlave, public sccommon::BaseMaster
            {
            public:
                Header2Smurf();
                ~Header2Smurf() {};

                static Header2SmurfPtr create();

                static void setup_python();

                // This will be call by the BaseSlave class after updating
                // the base counters
                void rxFrame(ris::FramePtr frame);
            };
        }
    }
}

#endif