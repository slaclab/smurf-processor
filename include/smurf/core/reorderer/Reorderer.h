#ifndef _SMURF_CORE_REORDERER_H_
#define _SMURF_CORE_REORDERER_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Re-orderer
 * ----------------------------------------------------------------------------
 * File          : Reorderer.h
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Data Re-orderer Class.
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
        namespace reorderer
        {
            class Reorderer;
            typedef boost::shared_ptr<Reorderer> ReordererPtr;

            class Reorderer : public ris::Slave, public ris::Master
            {
            public:
                Reorderer();
                ~Reorderer() {};

                static ReordererPtr create();

                static void setup_python();

                void acceptFrame(ris::FramePtr frame);

            private:
                std::vector<uint8_t> indexes;
            };
        }
    }
}

#endif