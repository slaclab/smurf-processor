/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Base Slave
 * ----------------------------------------------------------------------------
 * File          : BaseSlave.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Base Class for all Slave Devices.
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
#include "smurf/core/common/BaseSlave.h"

namespace scc  = smurf::core::common;

scc::BaseSlave::BaseSlave()
:
    ris::Slave(),
    disable(false),
    frameCnt(0),
    frameSize(0)
{
};

scc::BaseSlavePtr scc::BaseSlave::create()
{
    return boost::make_shared<BaseSlave>();
}

void scc::BaseSlave::setup_python()
{
    bp::class_<scc::BaseSlave, scc::BaseSlavePtr, bp::bases<ris::Slave>, boost::noncopyable >("BaseSlave", bp::init<>())
        .def("disableRx",      &BaseSlave::disableRx)
        .def("isRxDisabled",   &BaseSlave::isRxDisabled)
        .def("getRxFrameCnt",  &BaseSlave::getRxFrameCnt)
        .def("getRxFrameSize", &BaseSlave::getRxFrameSize)
        .def("clearRxCnt",     &BaseSlave::clearRxCnt)
    ;
    bp::implicitly_convertible< scc::BaseSlavePtr, ris::SlavePtr >();
}

void scc::BaseSlave::updateRxCnts(std::size_t s)
{
    ++frameCnt;
    frameSize = s;
}

void scc::BaseSlave::disableRx(bool d)
{
    disable = d;
}

const bool scc::BaseSlave::isRxDisabled() const
{
    return disable;
}

const std::size_t scc::BaseSlave::getRxFrameCnt() const
{
    return frameCnt;
}

const std::size_t scc::BaseSlave::getRxFrameSize() const
{
    return frameSize;
}

void scc::BaseSlave::clearRxCnt()
{
    frameCnt = 0;
}

void scc::BaseSlave::acceptFrame(ris::FramePtr frame)
{
    // Update the frame counter
    ++frameCnt;

    //Update the last frame size
    frameSise = frame->getPayload();

    // Now call the rxFrame, which will be defined in the derivated class
    rxFrame(frame);
}
