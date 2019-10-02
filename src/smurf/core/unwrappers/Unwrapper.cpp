/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Unwrapper
 * ----------------------------------------------------------------------------
 * File          : Unwrapper.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Descuiption :
 *   SMuRF Unwrapper Class.
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

#include <boost/python.hpp>
#include "smurf/core/unwrappers/Unwrapper.h"

namespace scu  = smurf::core::unwrappers;

scu::Unwrapper::Unwrapper()
:
    scc::BaseSlave(),
    scc::BaseMaster()
{
}

scu::UnwrapperPtr scu::Unwrapper::create()
{
    return boost::make_shared<Unwrapper>();
}

// Setup Class in python
void scu::Unwrapper::setup_python()
{
    bp::class_<scu::Unwrapper, scu::UnwrapperPtr, bp::bases<scc::BaseSlave,scc::BaseMaster>, boost::noncopyable >("Unwrapper", bp::init<>())
    ;
    bp::implicitly_convertible< scu::UnwrapperPtr, ris::SlavePtr >();
    bp::implicitly_convertible< scu::UnwrapperPtr, ris::MasterPtr >();
}

void scu::Unwrapper::rxtFrame(ris::FramePtr frame)
{
    // If the processing block is disabled, do not process the frame
    if (isRxDisabled())
    {
        // Send the frame to the next slave.
        // This method will check if the Tx block is disabled, as well
        // as updating the Tx counters
        txFrame(frame);

        return;
    }

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(frame);
}