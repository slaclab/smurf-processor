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

scf::GeneralAnalogFilter::GeneralAnalogFilter()
:
    scc::BaseSlave(),
    scc::BaseMaster(),
    numCh(0),
    order(0),
    gain(1),
    a_coef(1,1),
    b_coef(1,1)
{
}

scf::GeneralAnalogFilterPtr scf::GeneralAnalogFilter::create()
{
    return boost::make_shared<GeneralAnalogFilter>();
}

void scf::GeneralAnalogFilter::setup_python()
{
    bp::class_<scf::GeneralAnalogFilter, scf::GeneralAnalogFilterPtr, bp::bases<scc::BaseSlave,scc::BaseMaster>, boost::noncopyable >("GeneralAnalogFilter",bp::init<>())
        .def("setOrder", &GeneralAnalogFilter::setOrder)
        .def("setA",     &GeneralAnalogFilter::setA)
        .def("setB",     &GeneralAnalogFilter::setB)
        .def("setGain",  &GeneralAnalogFilter::setGain)
        .def("getNumCh", &GeneralAnalogFilter::getNumCh)
    ;
    bp::implicitly_convertible< scf::GeneralAnalogFilterPtr, scc::BaseSlavePtr  >();
    bp::implicitly_convertible< scf::GeneralAnalogFilterPtr, scc::BaseMasterPtr >();
}

void scf::GeneralAnalogFilter::setOrder(std::size_t o)
{
    order = o;
}

void scf::GeneralAnalogFilter::setA(boost::python::list a)
{
    std::vector<double> temp;

    // Extract the coefficients coming from python into a temporal vector
    for (std::size_t i{0}; i < len(a); ++i)
    {
        temp.push_back(boost::python::extract<double>(a[i]));
    }

    // Update the a_coef vector with the new values
    a_coef.swap(temp);
}

void scf::GeneralAnalogFilter::setB(boost::python::list b)
{
    std::vector<double> temp;

    // Extract the coefficients coming from python into a temporal vector
    for (std::size_t i{0}; i < len(b); ++i)
    {
        temp.push_back(boost::python::extract<double>(b[i]));
    }

    // Update the a_coef vector with the new values
    b_coef.swap(temp);
}

void scf::GeneralAnalogFilter::setGain(double g)
{
    gain = g;
}

// Get the number of mapper channels
const std::size_t getNumCh() const;

void scf::GeneralAnalogFilter::rxtFrame(ris::FramePtr frame)
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

    // (smart) pointer to the smurf header in the input frame (Read-only)
    SmurfHeaderROPtr smurfHeaderIn(SmurfHeaderRO::create(frame));

    // Get the number of channels in the input frame. The number of output channel will be the same
    std::size_t newNumCh = smurfHeaderIn->getNumberChannels();

    // Verify if the frame size has changed
    if (numCh != newNumCh)
    {

        // Update the number of channels we are processing
        numCh = newNumCh;

        // Resize and clear the data buffers
        currentData.resize(  numCh, 0 );
        previousData.resize( numCh, 0 );
        wrapCounter.resize(  numCh, 0 );
    }

    // Request a new frame, to hold the same payload as the input frame
    //std::size_t outFrameSize = SmurfHeader::SmurfHeaderSize + sizeof(output_data_t) * numCh;
    // For now we want to keep packet of the same size, so let's do this instead:
    std::size_t outFrameSize = SmurfHeader::SmurfHeaderSize +
        ( ( frame->getPayload() - SmurfHeader::SmurfHeaderSize )/sizeof(input_data_t) ) * sizeof(output_data_t);
    ris::FramePtr outFrame = reqFrame(outFrameSize, true);
    outFrame->setPayload(outFrameSize);

    // Iterator to the input frame
    ris::FrameIterator inFrameIt = frame->beginRead();

    // Iterator to the output frame
    ris::FrameIterator outFrameIt = outFrame->beginWrite();

    // Copy the header from the input frame to the output frame.
    for (std::size_t i{0}; i < SmurfHeader::SmurfHeaderSize; ++i)
            *(++outFrameIt) = *(++inFrameIt);

    // Filter the data

    // Print a few work to verify the mapping works
    std::cout << "  === FILTER === " << std::endl;
    std::cout << "INDEX    INPUT FRAME     OUTPUT FRAME" << std::endl;
    std::cout << "=====================================" << std::endl;
    {
        ris::FrameIterator in = frame->beginRead();
        ris::FrameIterator out = outFrame->beginRead();

        in += SmurfHeader::SmurfHeaderSize;
        out += SmurfHeader::SmurfHeaderSize;
        for (std::size_t i{0}; i < 20; ++i)
            std::cout << i << std::hex << "  0x" << unsigned(*(in+i)) << "  0x" << unsigned(*(out+i)) << std::dec << std::endl;
    }
    std::cout << "=====================================" << std::endl;

    // Send the frame to the next slave.
    // This method will check if the Tx block is disabled, as well
    // as updating the Tx counters
    txFrame(outFrame);
}