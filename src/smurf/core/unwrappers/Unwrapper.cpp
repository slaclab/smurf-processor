/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Unwrapper
 * ----------------------------------------------------------------------------
 * File          : Unwrapper.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Descuiption :
 *   SMuRF Unwrapper Class.
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
#include "smurf/core/unwrappers/Unwrapper.h"

namespace scu  = smurf::core::unwrappers;

scu::Unwrapper::Unwrapper()
:
    ris::Slave(),
    ris::Master(),
    disable(false),
    frameCnt(0),
    frameSize(0)
{
    std::cout << "Unwrapper created" << std::endl;
}

scu::UnwrapperPtr scu::Unwrapper::create()
{
    return boost::make_shared<Unwrapper>();
}

// Setup Class in python
void scu::Unwrapper::setup_python()
{
    bp::class_<scu::Unwrapper, scu::UnwrapperPtr, bp::bases<ris::Slave,ris::Master>, boost::noncopyable >("Unwrapper", bp::init<>())
        .def("setDisable",          &Unwrapper::setDisable)
        .def("getDisable",          &Unwrapper::getDisable)
        .def("getFrameCnt",         &Unwrapper::getFrameCnt)
        .def("getFrameSize",        &Unwrapper::getFrameSize)
        .def("clearCnt",            &Unwrapper::clearCnt)
    ;
    bp::implicitly_convertible< scu::UnwrapperPtr, ris::SlavePtr >();
    bp::implicitly_convertible< scu::UnwrapperPtr, ris::MasterPtr >();
}

void scu::Unwrapper::setDisable(bool d)
{
    disable = d;
}

const bool scu::Unwrapper::getDisable() const
{
    return disable;
}


const std::size_t scu::Unwrapper::getFrameCnt() const
{
    return frameCnt;
}

const std::size_t scu::Unwrapper::getFrameSize() const
{
    return frameSize;
}

void scu::Unwrapper::clearCnt()
{
    frameCnt         = 0;
}

void scu::Unwrapper::acceptFrame(ris::FramePtr frame)
{
    std::cout << "Unwrapper. Frame received..." << std::endl;
    std::cout << "Size = " << frame->getPayload() << std::endl;

    // If the processing block is disabled, just send the frame
    // to the next slave.
    if (disable)
    {
        sendFrame(frame);
        return;
    }

    //Increase the frame counter
    ++frameCnt;

    // Update the last frame size
    frameSize = frame->getPayload();


    // Request a new frame
    ris::FramePtr newFrame = reqFrame(128, true);

    // Iterator to the input frame
    ris::FrameIterator itIn = frame->beginRead();

    // Iterator to the output frame
    ris::FrameIterator itOut = newFrame->beginWrite();

    // Copy the header from the input frame to the output frame.
    for (std::size_t i{0}; i < 128; ++i)
            *(itOut+1) = *(itIn+1);

    // Set the frame size
    newFrame->setPayload(128);

    // Send the frame
    sendFrame(newFrame);
}