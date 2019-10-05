/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Header2Smurf
 * ----------------------------------------------------------------------------
 * File          : Downsamlper.cpp
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

#include <boost/python.hpp>
#include "smurf/core/conventers/Header2Smurf.h"

namespace scc = smurf::core::conventers;

scc::Header2Smurf::Header2Smurf()
:
    sccommon::BaseSlave(),
    sccommon::BaseMaster()
{
}

scc::Header2SmurfPtr scc::Header2Smurf::create()
{
    return boost::make_shared<Header2Smurf>();
}

void scc::Header2Smurf::setup_python()
{
    bp::class_<scc::Header2Smurf, scc::Header2SmurfPtr, bp::bases<sccommon::BaseSlave,sccommon::BaseMaster>, boost::noncopyable >("Header2Smurf",bp::init<>())
    ;
    bp::implicitly_convertible< scc::Header2SmurfPtr, sccommon::BaseSlavePtr  >();
    bp::implicitly_convertible< scc::Header2SmurfPtr, sccommon::BaseMasterPtr >();
}

void scc::Header2Smurf::rxFrame(ris::FramePtr frame)
{
    // If the processing block is disabled, do not process the frame
    if (!isRxDisabled())
    {
        // Update the frame header
        SmurfHeaderPtr smurfHeaderOut(SmurfHeader::create(frame));

        smurfHeaderOut->setVersion(1);
    }

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(frame);
}