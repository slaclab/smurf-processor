#ifndef _SMURF_CORE_COMMON_BASESLAVE_H_
#define _SMURF_CORE_COMMON_BASESLAVE_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Base Slave
 * ----------------------------------------------------------------------------
 * File          : BaseSlave.h
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Base Class for all Slave Devices.
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
        namespace common
        {
            class BaseSlave;
            typedef boost::shared_ptr<BaseSlave> BaseSlavePtr;

            class BaseSlave : public ris::Slave
            {
            public:
                BaseSlave();
                virtual ~BaseSlave() {};

                static BaseSlavePtr create();

                static void setup_python();

                // Derivated classes need to class this method to
                // update the counters
                void updateCnts(std::size_t s);

                // Disable the processing block. The data
                // will just pass through to the next slave
                void       disableRx(bool d);
                const bool isRxDisabled()       const;

                // Get the frame counter
                const std::size_t getRxFrameCnt() const;

                // Get the last frame size (in bytes)
                const std::size_t getRxFrameSize() const;

                // Clear all counter. It can be redefined in the derivated class
                // if other counters need to be clear.
                virtual void clearRxCnt();

            private:
                bool        disable;    // Disable flag
                std::size_t frameCnt;   // Frame counter
                std::size_t frameSize;  // Last frame size (bytes)
            };
        }
    }
}

#endif