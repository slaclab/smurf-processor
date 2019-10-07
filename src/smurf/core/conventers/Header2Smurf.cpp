/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Header2Smurf
 * ----------------------------------------------------------------------------
 * File          : Downsamlper.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Data Header2Smurf Class.
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
#include "smurf/core/conventers/Header2Smurf.h"

namespace scc = smurf::core::conventers;

scc::Header2Smurf::Header2Smurf()
:
    sccommon::BaseSlave(),
    sccommon::BaseMaster(),
    tesBias(reqFrame(TesBiasArray::TesBiasBufferSize, true)),
    tba(TesBiasArray::create(tesBias->beginWrite()))
{
    tesBias->setPayload(TesBiasArray::TesBiasBufferSize);
    tba->setDataIt(tesBias->beginWrite());
}

scc::Header2SmurfPtr scc::Header2Smurf::create()
{
    return boost::make_shared<Header2Smurf>();
}

void scc::Header2Smurf::setup_python()
{
    bp::class_<scc::Header2Smurf, scc::Header2SmurfPtr, bp::bases<sccommon::BaseSlave,sccommon::BaseMaster>, boost::noncopyable >("Header2Smurf",bp::init<>())
        .def("setTesBias", &Header2Smurf::setTesBias)
    ;
    bp::implicitly_convertible< scc::Header2SmurfPtr, sccommon::BaseSlavePtr  >();
    bp::implicitly_convertible< scc::Header2SmurfPtr, sccommon::BaseMasterPtr >();
}

void scc::Header2Smurf::setTesBias(std::size_t index, int32_t value)
{
    // Hold the mutex while the data tesBias array is being written to.
    std::lock_guard<std::mutex> lock(*tba->getMutex());

    tba->setWord(index, value);
}

void scc::Header2Smurf::rxFrame(ris::FramePtr frame)
{
    // If the processing block is disabled, do not process the frame
    if (!isRxDisabled())
    {
        // Update the frame header
        SmurfHeaderPtr smurfHeaderOut(SmurfHeader::create(frame));

        // Stet he protocol version
        smurfHeaderOut->setVersion(1);

        // Copy the TES Bias values
        {
            // Hold the mutex while the data tesBias array is being written to.
            std::lock_guard<std::mutex> lock(*tba->getMutex());

            for(std::size_t i{0}; i < TesBiasArray::TesBiasCount; ++i)
                smurfHeaderOut->setTESBias(i, tba->getWord(i));
        }

        // Set the UNIX time
        timespec tmp;
        clock_gettime(CLOCK_REALTIME, &tmp);
        smurfHeaderOut->setUnixTime(1000000000l * static_cast<uint64_t>(tmp.tv_sec) + static_cast<uint64_t>(tmp.tv_nsec));
    }

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(frame);
}