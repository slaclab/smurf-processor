/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data GeneralAnalogFilter
 * ----------------------------------------------------------------------------
 * File          : GeneralAnalogFilter.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *   SMuRF Data GeneralAnalogFilter Class.
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
#include "smurf/core/filters/GeneralAnalogFilter.h"

namespace scf = smurf::core::filters;

scf::GeneralAnalogFilter::GeneralAnalogFilter(std::size_t s)
:
    scc::BaseSlave(),
    scc::BaseMaster(),
    size(s)
{
    std::cout << "GeneralAnalogFilter of size " << size << " created" << std::endl;
}

scf::GeneralAnalogFilterPtr scf::GeneralAnalogFilter::create(std::size_t s)
{
    return boost::make_shared<GeneralAnalogFilter>(s);
}

void scf::GeneralAnalogFilter::setup_python()
{
    bp::class_<scf::GeneralAnalogFilter, scf::GeneralAnalogFilterPtr, bp::bases<scc::BaseSlave,scc::BaseMaster>, boost::noncopyable >("GeneralAnalogFilter",bp::init<std::size_t>())
    ;
    bp::implicitly_convertible< scf::GeneralAnalogFilterPtr, scc::BaseSlavePtr  >();
    bp::implicitly_convertible< scf::GeneralAnalogFilterPtr, scc::BaseMasterPtr >();
}

void scf::GeneralAnalogFilter::acceptFrame(ris::FramePtr frame)
{
    std::cout << "  GeneralAnalogFilter. Frame received..." << std::endl;
    std::cout << "  Size = " << frame->getPayload() << std::endl;

    // If the processing block is disabled, do not process the frame
    if (isRxDisabled())
    {

        // Is the Tx block is not disabled, send the same Rx frame
        if (! isTxDisabled())
        {
            // Update the Tx counters. This is define in the BaseMaster class
            updateTxCnts(frame->getPayload());

            sendFrame(frame);
        }

        return;
    }

    // Update the Rx counters. This is define in the BaseSlave class
    updateRxCnts(frame->getPayload());

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

    // Send the frame, if the Tx block is not disabled
    if (! isTxDisabled())
    {
        // Update the Tx counters. This is define in the BaseMaster class
        updateTxCnts(newFrame->getPayload());

        sendFrame(newFrame);
    }
}