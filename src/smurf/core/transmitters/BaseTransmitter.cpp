/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Base Transmitter
 * ----------------------------------------------------------------------------
 * File          : BaseTransmitter.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *   SMuRF Data Base Transmitter Class.
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
#include "smurf/core/transmitters/BaseTransmitter.h"

namespace bp  = boost::python;
namespace sct = smurf::core::transmitters;

sct::BaseTransmitter::BaseTransmitter()
:
    scc::BaseSlave()
{
}

sct::BaseTransmitterPtr sct::BaseTransmitter::create()
{
    return boost::make_shared<BaseTransmitter>();
}

// Setup Class in python
void sct::BaseTransmitter::setup_python()
{
    bp::class_<sct::BaseTransmitter, sct::BaseTransmitterPtr, bp::bases<scc::BaseSlave>, boost::noncopyable >("BaseTransmitter",bp::init<>())
    ;
    bp::implicitly_convertible< sct::BaseTransmitterPtr, scc::BaseSlavePtr >();
}

void sct::BaseTransmitter::rxFrame(ris::FramePtr frame)
{
    rogue::GilRelease noGil;

    // If the processing block is disabled, do not process the frame
    if (isRxDisabled())
    	return;
}