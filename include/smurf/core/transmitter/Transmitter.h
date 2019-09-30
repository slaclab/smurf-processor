#ifndef _SMURF_CORE_TRANSMITTER_H_
#define _SMURF_CORE_TRANSMITTER_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Transmitter
 * ----------------------------------------------------------------------------
 * File          : Transmitter.h
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Data Transmitter Class.
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

namespace smurf
{
    namespace core
    {
        namespace transmitter
        {
            class Transmitter;
            typedef std::shared_ptr<Transmitter> TransmitterPtr;

            class Transmitter
            {
            public:
                Transmitter();
                ~Transmitter() {};

                static TransmitterPtr create();

                static void setup_python();
            };
        }
    }
}

#endif