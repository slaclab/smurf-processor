/**
 *-----------------------------------------------------------------------------
 * Title      : Python Module For Transmitter
 * ----------------------------------------------------------------------------
 * File       : module.cpp
 * Created    : 2016-09-27
 * ----------------------------------------------------------------------------
 * Description:
 * Python module setup
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
#include "smurf/core/transmitter/module.h"
#include "smurf/core/transmitter/Transmitter.h"

namespace bp  = boost::python;
namespace sct = smurf::core::transmitter;

void sct::setup_module()
{
    sct::Transmitter::setup_python();
}