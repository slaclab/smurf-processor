/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Downsampler
 * ----------------------------------------------------------------------------
 * File          : Downsamlper.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Data Downsampler Class.
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
#include "smurf/core/downsamplers/Downsampler.h"

namespace scd = smurf::core::downsamplers;

scd::Downsampler::Downsampler()
:
    scc::BaseSlave(),
    scc::BaseMaster(),
    numCh(0),
    factor(1),
    sampleCnt(0)
{
}

scd::DownsamplerPtr scd::Downsampler::create()
{
    return boost::make_shared<Downsampler>();
}

void scd::Downsampler::setup_python()
{
    bp::class_<scd::Downsampler, scd::DownsamplerPtr, bp::bases<scc::BaseSlave,scc::BaseMaster>, boost::noncopyable >("Downsampler",bp::init<>())
        .def("setFactor", &Downsampler::setFactor)
        .def("getFactor", &Downsampler::getFactor)
        .def("getNumCh",  &Downsampler::getNumCh)
    ;
    bp::implicitly_convertible< scd::DownsamplerPtr, scc::BaseSlavePtr  >();
    bp::implicitly_convertible< scd::DownsamplerPtr, scc::BaseMasterPtr >();
}


void scd::Downsampler::setFactor(std::size_t f)
{
    factor = f;

    // When the factor is changed, reset the counter.
    reset();
}

const std::size_t scd::Downsampler::getFactor() const
{
    return factor;
}

const std::size_t scd::Downsampler::getNumCh() const
{
    return numCh;
}

void scd::Downsampler::reset()
{
    sampleCnt = 0;
}

void scd::Downsampler::rxFrame(ris::FramePtr frame)
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

    // Increase the sampler counter. Don't do anything until the factor is reach.
    // When the factor is reach, send out the current frame and reset the downsampler.
    if (++sampleCnt < factor)
        return;

    // Reset the downsampler
    reset();

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(frame);
}