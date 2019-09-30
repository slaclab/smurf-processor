/**
 *-----------------------------------------------------------------------------
 * Title      : SMuRF Data Filter
 * ----------------------------------------------------------------------------
 * File       : Filter.cpp
 * Created    : 2019-09-27
 * ----------------------------------------------------------------------------
 * Description:
 *   SMuRF Data Filter Class.
 * ----------------------------------------------------------------------------
 * This file is part of the smurf software platform. It is subject to
 * the license terms in the LICENSE.txt file found in the top-level directory
 * of this distribution and at:
 *    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
 * No part of the smurf software platform, including this file, may be
 * copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE.txt file.
 * ----------------------------------------------------------------------------
**/

#include <boost/python.hpp>
#include "smurf/core/filter/module.h"
#include "smurf/core/filter/Filter.h"

namespace bp  = boost::python;
namespace scf = smurf::core::filter;

void scf::setup_module()
{
    // map the IO namespace to a sub-module
    bp::object module(bp::handle<>(bp::borrowed(PyImport_AddModule("smurf.core.filter"))));

    // make "from mypackage import class1" work
    bp::scope().attr("filter") = module;

    // set the current scope to the new sub-module
    bp::scope io_scope = module;

    scf::Filter::setup_python();
}