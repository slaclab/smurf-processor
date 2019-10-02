/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Base Master
 * ----------------------------------------------------------------------------
 * File          : BaseMaster.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Base Class for all Master Devices.
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
#include "smurf/core/common/BaseMaster.h"

namespace scc  = smurf::core::common;

scc::BaseMaster::BaseMaster()
:
    ris::Master(),
    disable(false),
    frameCnt(0),
    frameSize(0)
{
};

scc::BaseMasterPtr scc::BaseMaster::create()
{
    return boost::make_shared<BaseMaster>();
}

void scc::BaseMaster::setup_python()
{
    bp::class_<scc::BaseMaster, scc::BaseMasterPtr, bp::bases<ris::Master>, boost::noncopyable >("BaseMaster", bp::init<>())
        .def("disableTx",      &BaseMaster::disableTx)
        .def("isTxDisabled",   &BaseMaster::isTxDisabled)
        .def("getTxFrameCnt",  &BaseMaster::getTxFrameCnt)
        .def("getTxFrameSize", &BaseMaster::getTxFrameSize)
        .def("clearTxCnt",     &BaseMaster::clearTxCnt)
    ;
    bp::implicitly_convertible< scc::BaseMasterPtr, ris::MasterPtr >();
}

void scc::BaseMaster::updateTxCnts(std::size_t s)
{
    ++frameCnt;
    frameSize = s;
}

void scc::BaseMaster::disableTx(bool d)
{
    disable = d;
}

const bool scc::BaseMaster::isTxDisabled() const
{
    return disable;
}

const std::size_t scc::BaseMaster::getTxFrameCnt() const
{
    return frameCnt;
}

const std::size_t scc::BaseMaster::getTxFrameSize() const
{
    return frameSize;
}

void scc::BaseMaster::clearTxCnt()
{
    frameCnt = 0;
}

void scc::BaseMaster::txFrame(ris::FramePtr frame)
{
    // If the Tx block is disable, don't do anything
    if (disable)
        return;

    // Update the frame counter
    ++frameCnt;

    //Update the last frame size
    frameSize = frame->getPayload();

    // Now call the sendFrame method from ris::Master
    sendFrame(frame);
}
