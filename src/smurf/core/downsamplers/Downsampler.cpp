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
    ris::Slave(),
    ris::Master(),
    disable(false),
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
    bp::class_< scd::Downsampler,
                scd::DownsamplerPtr,
                bp::bases<ris::Slave,ris::Master>,
                boost::noncopyable >
                ("Downsampler",bp::init<>())
        .def("setDisable", &Downsampler::setDisable)
        .def("getDisable", &Downsampler::getDisable)
        .def("setFactor",  &Downsampler::setFactor)
        .def("getFactor",  &Downsampler::getFactor)
    ;
    bp::implicitly_convertible< scd::DownsamplerPtr, ris::SlavePtr  >();
    bp::implicitly_convertible< scd::DownsamplerPtr, ris::MasterPtr >();
}

void scd::Downsampler::setDisable(bool d)
{
    disable = d;
}

const bool scd::Downsampler::getDisable() const
{
    return disable;
}

void scd::Downsampler::setFactor(std::size_t f)
{
    // Check if the factor is 0
    if (0 == f)
    {
        // This should go to a logger instead
        std::cerr << "ERROR: Trying to set factor = 0."<< std::endl;
        return;
    }

    factor = f;

    // When the factor is changed, reset the counter.
    reset();
}

const std::size_t scd::Downsampler::getFactor() const
{
    return factor;
}

void scd::Downsampler::reset()
{
    sampleCnt = 0;
}

void scd::Downsampler::acceptFrame(ris::FramePtr frame)
{
    rogue::GilRelease noGil;

    // If the processing block is disabled, do not process the frame
    if (disable)
    {
        // Send the frame to the next slave.
        sendFrame(frame);

        return;
    }

    // Increase the sampler counter. Don't do anything until the factor is reach.
    // When the factor is reach, send out the current frame and reset the downsampler.
    if (++sampleCnt < factor)
        return;

    // Reset the downsampler
    reset();

    // Send the frame to the next slave.
    sendFrame(frame);
}