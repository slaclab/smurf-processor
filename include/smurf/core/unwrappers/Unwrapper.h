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
#include "smurf/core/common/Helpers.h"

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
                void rxFrame(ris::FramePtr frame);

                // Get the number of mapper channels
                const std::size_t getNumCh() const;

            private:
                // Data type used to read the data from the input frame
                typedef int16_t input_data_t;

                // Data type used to write data to the output frame
                typedef int32_t output_data_t;

                // If we are above/below these and jump, assume a wrap
                const input_data_t upperUnwrap =  0x6000;
                const input_data_t lowerUnwrap = -0x6000;

                // Wrap counter steps
                const output_data_t stepUnwrap = 0x10000;

                std::size_t numCh;

                // Data buffer
                std::vector<output_data_t> currentData;
                std::vector<output_data_t> previousData;
                std::vector<output_data_t> wrapCounter;
            };
        }
    }
}

#endif