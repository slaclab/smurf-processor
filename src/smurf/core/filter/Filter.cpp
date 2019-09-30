/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Filter
 * ----------------------------------------------------------------------------
 * File          : Filter.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *   SMuRF Data Filter Class.
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
#include "smurf/core/filter/Filter.h"

namespace scf = smurf::core::filter;

scf::Filter::Filter(std::size_t s)
:
    ris::Slave(),
    ris::Master(),
    size(s)
{
    std::cout << "Filter of size " << size << " created" << std::endl;
}

scf::FilterPtr scf::Filter::create(std::size_t s)
{
    return boost::make_shared<Filter>(s);
}

void scf::Filter::setup_python()
{
    bp::class_<scf::Filter, scf::FilterPtr, bp::bases<ris::Slave,ris::Master>, boost::noncopyable >("Filter",bp::init<std::size_t>())
    ;
    bp::implicitly_convertible< scf::FilterPtr, ris::SlavePtr >();
    bp::implicitly_convertible< scf::FilterPtr, ris::MasterPtr >();
}

void scf::Filter::acceptFrame(ris::FramePtr frame)
{
    std::cout << "  Filter. Frame received..." << std::endl;
    std::cout << "  Size = " << frame->getPayload() << std::endl;

    sendFrame(frame);
}