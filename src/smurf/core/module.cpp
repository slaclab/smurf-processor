 /**
 *-----------------------------------------------------------------------------
 * Title      : Python Module
 * ----------------------------------------------------------------------------
 * File       : module.cpp
 * Created    : 2016-09-27
 * ----------------------------------------------------------------------------
 * Description:
 *   Python module setup
 * ----------------------------------------------------------------------------
 * This file is part of the rogue software platform. It is subject to
 * the license terms in the LICENSE.txt file found in the top-level directory
 * of this distribution and at:
 *    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
 * No part of the rogue software platform, including this file, may be
 * copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE.txt file.
 * ----------------------------------------------------------------------------
**/

#include <boost/python.hpp>
#include "smurf/core/module.h"
#include "smurf/core/reorderer/module.h"
#include "smurf/core/filter/module.h"
#include "smurf/core/transmitter/module.h"

namespace bp  = boost::python;
namespace sc = smurf::core;

void sc::setup_module()
{
   sc::reorderer::setup_module();
   sc::filter::setup_module();
   sc::transmitter::setup_module();
}