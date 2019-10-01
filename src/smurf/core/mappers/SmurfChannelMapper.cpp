/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Channel Mapper
 * ----------------------------------------------------------------------------
 * File          : SmurfChannelMapper.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Descmiption :
 *   SMuRF Channel Mapper Class.
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
#include "smurf/core/mappers/SmurfChannelMapper.h"

namespace scm  = smurf::core::mappers;

scm::SmurfChannelMapper::SmurfChannelMapper()
:
    ris::Slave(),
    ris::Master(),
    disable(false),
    frameCnt(0),
    frameSize(0)
{
    std::cout << "SmurfChannelMapper created" << std::endl;
}

scm::SmurfChannelMapperPtr scm::SmurfChannelMapper::create()
{
    return boost::make_shared<SmurfChannelMapper>();
}

// Setup Class in python
void scm::SmurfChannelMapper::setup_python()
{
    bp::class_<scm::SmurfChannelMapper, scm::SmurfChannelMapperPtr, bp::bases<ris::Slave,ris::Master>, boost::noncopyable >("SmurfChannelMapper", bp::init<>())
        .def("setDisable",          &SmurfChannelMapper::setDisable)
        .def("getDisable",          &SmurfChannelMapper::getDisable)
        .def("getFrameCnt",         &SmurfChannelMapper::getFrameCnt)
        .def("getFrameSize",        &SmurfChannelMapper::getFrameSize)
        .def("clearCnt",            &SmurfChannelMapper::clearCnt)
    ;
    bp::implicitly_convertible< scm::SmurfChannelMapperPtr, ris::SlavePtr >();
    bp::implicitly_convertible< scm::SmurfChannelMapperPtr, ris::MasterPtr >();
}

void scm::SmurfChannelMapper::setDisable(bool d)
{
    disable = d;
}

const bool scm::SmurfChannelMapper::getDisable() const
{
    return disable;
}


const std::size_t scm::SmurfChannelMapper::getFrameCnt() const
{
    return frameCnt;
}

const std::size_t scm::SmurfChannelMapper::getFrameSize() const
{
    return frameSize;
}

void scm::SmurfChannelMapper::clearCnt()
{
    frameCnt         = 0;
}

void scm::SmurfChannelMapper::acceptFrame(ris::FramePtr frame)
{
    std::cout << "SmurfChannelMapper. Frame received..." << std::endl;
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