/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Re-orderer
 * ----------------------------------------------------------------------------
 * File          : Reorderer.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *   SMuRF Data Re-orderer Class.
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
#include "smurf/core/reorderer/Reorderer.h"

namespace scr  = smurf::core::reorderer;

scr::Reorderer::Reorderer()
:
    ris::Slave(),
    ris::Master(),
    disable(false)
{
    std::cout << "Reorderer created" << std::endl;
}

scr::ReordererPtr scr::Reorderer::create()
{
    return boost::make_shared<Reorderer>();
}

// Setup Class in python
void scr::Reorderer::setup_python()
{
    bp::class_<scr::Reorderer, scr::ReordererPtr, bp::bases<ris::Slave,ris::Master>, boost::noncopyable >("Reorderer", bp::init<>())
        .def("setDisable", &Reorderer::setDisable)
        .def("getDisable", &Reorderer::getDisable)
    ;
    bp::implicitly_convertible< scr::ReordererPtr, ris::SlavePtr >();
    bp::implicitly_convertible< scr::ReordererPtr, ris::MasterPtr >();
}

void scr::Reorderer::setDisable(bool d)
{
    disable = d;
}

const bool scr::Reorderer::getDisable() const
{
    return disable;
}

void scr::Reorderer::acceptFrame(ris::FramePtr frame)
{
    std::cout << "Reorderer. Frame received..." << std::endl;
    std::cout << "Size = " << frame->getPayload() << std::endl;

    // If the processing block is disabled, just send the frame
    // to the next slave.
    if (disable)
    {
        sendFrame(frame);
        return;
    }

    ris::FrameIterator it = frame->beginRead();

    union U
    {
        uint32_t w;
        uint8_t  b[4];
    };

    U frameNumber;

    for (std::size_t i{0}; i < 4; ++i)
            frameNumber.b[i] = *(it+84+i);

    std::cout << "Frame number = " << frameNumber.w << std::endl;

    for (std::size_t r{1}; r <= 0; ++r)
            std::cout << "Iterating order. r = " << r << std::endl;
    ris::FramePtr newFrame = reqFrame(128, true);
    ris::FrameIterator itOut = newFrame->beginWrite();

    for (std::size_t i{0}; i < 128; ++i)
            *(itOut+1) = *(it+1);

    newFrame->setPayload(128);

    sendFrame(newFrame);
}