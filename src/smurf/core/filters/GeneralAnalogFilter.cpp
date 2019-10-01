/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data GeneralAnalogFilter
 * ----------------------------------------------------------------------------
 * File          : GeneralAnalogFilter.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *   SMuRF Data GeneralAnalogFilter Class.
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
#include "smurf/core/filters/GeneralAnalogFilter.h"

namespace scf = smurf::core::filters;

scf::GeneralAnalogFilter::GeneralAnalogFilter(std::size_t s)
:
    ris::Slave(),
    ris::Master(),
    size(s)
{
    std::cout << "GeneralAnalogFilter of size " << size << " created" << std::endl;
}

scf::GeneralAnalogFilterPtr scf::GeneralAnalogFilter::create(std::size_t s)
{
    return boost::make_shared<GeneralAnalogFilter>(s);
}

void scf::GeneralAnalogFilter::setup_python()
{
    bp::class_<scf::GeneralAnalogFilter, scf::GeneralAnalogFilterPtr, bp::bases<ris::Slave,ris::Master>, boost::noncopyable >("GeneralAnalogFilter",bp::init<std::size_t>())
    ;
    bp::implicitly_convertible< scf::GeneralAnalogFilterPtr, ris::SlavePtr >();
    bp::implicitly_convertible< scf::GeneralAnalogFilterPtr, ris::MasterPtr >();
}

void scf::GeneralAnalogFilter::acceptFrame(ris::FramePtr frame)
{
    std::cout << "  GeneralAnalogFilter. Frame received..." << std::endl;
    std::cout << "  Size = " << frame->getPayload() << std::endl;

    sendFrame(frame);
}