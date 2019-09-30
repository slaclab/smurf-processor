/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Transmitter
 * ----------------------------------------------------------------------------
 * File          : Transmitter.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *   SMuRF Data Transmitter Class.
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
#include "smurf/core/transmitter/Transmitter.h"

namespace bp  = boost::python;
namespace sct = smurf::core::transmitter;

sct::Transmitter::Transmitter()
:
    ris::Slave()
{
    std::cout << "Transmitter created" << std::endl;
}

sct::TransmitterPtr sct::Transmitter::create()
{
    return boost::make_shared<Transmitter>();
}

// Setup Class in python
void sct::Transmitter::setup_python()
{
    bp::class_<sct::Transmitter, sct::TransmitterPtr, bp::bases<ris::Slave>, boost::noncopyable >("Transmitter",bp::init<>())
    ;
    bp::implicitly_convertible< sct::TransmitterPtr, ris::SlavePtr >();
}

void sct::Transmitter::acceptFrame(ris::FramePtr frame)
{
    std::cout << "    Transmitter. Frame received..." << std::endl;
    std::cout << "    Size = " << frame->getPayload() << std::endl;
}