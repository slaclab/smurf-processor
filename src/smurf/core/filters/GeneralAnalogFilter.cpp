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
    b_coef(1,1),
    data( order, std::vector<output_data_t>(numCh) )
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
    // Check if the new order is different from the current one
    if ( o != order )
    {
        order = o;

        // When the order it change, reset the filter
        reset();

        std::cout << "Order set to: " << order << std::endl;

        std::cout << "a_coef.size() = " << a_coef.size() << ". Elements are:" << std::endl;
        for (std::vector<double>::iterator it = a_coef.begin(); it != a_coef.end(); ++it)
                std::cout << *it << ", ";
            std::cout << std::endl;

        std::cout << "b_coef.size() = " << b_coef.size() << ". Elements are:" << std::endl;
        for (std::vector<double>::iterator it = b_coef.begin(); it != b_coef.end(); ++it)
                std::cout << *it << ", ";
            std::cout << std::endl;

            std::cout << "data.size() = " << data.size() << std::endl;
            if (data.size() > 0)
                std::cout << "data.at(0).size() = " << data.at(0).size() << std::endl;
    }
}

void scf::GeneralAnalogFilter::setA(boost::python::list a)
{
    std::vector<double> temp;

    std::size_t listSize = len(a);

    // Verify that the input list is not empty.
    // If empty, set the coefficients vector to a = [1.0].
    if (listSize == 0)
    {
        // This should go to a logger instead
        std::cerr << "ERROR: Trying to set an empty set of a coefficients. Defaulting to 'a = [1.0]'"<< std::endl;
        temp.push_back(1.0);
        b_coef.swap(temp);

        return;
    }

    // Verify that the first coefficient is not zero.
    // if it is, set the coefficients vector to a = [1.0].
    if (a[0] == 0)
    {
        // This should go to a logger instead
        std::cerr << "ERROR: The first a coefficient can not be zero. Defaulting to 'a = [1.0]'"<< std::endl;
        temp.push_back(1.0);
        b_coef.swap(temp);
        return;
    }

    // Extract the coefficients coming from python into a temporal vector
    for (std::size_t i{0}; i < listSize; ++i)
    {
        temp.push_back(boost::python::extract<double>(a[i]));
    }

    // Update the a_coef vector with the new values
    a_coef.swap(temp);

    std::cout << "A coefficients set to: " << std::endl;
    for (std::vector<double>::iterator it = a_coef.begin(); it != a_coef.end(); ++it)
        std::cout << *it << ", ";
    std::cout << std::endl;
}

void scf::GeneralAnalogFilter::setB(boost::python::list b)
{
    std::vector<double> temp;

    std::size_t listSize = len(b);

    // Verify that the input list is not empty.
    // If empty, set the coefficients vector to a = [0.0].
    if (listSize == 0)
    {
        // This should go to a logger instead
        std::cerr << "ERROR: Trying to set an empty set of a coefficients. Defaulting to 'b = [0.0]'"<< std::endl;
        temp.push_back(0.0);
        b_coef.swap(temp);

        return;
    }

    // Extract the coefficients coming from python into a temporal vector
    for (std::size_t i{0}; i < len(b); ++i)
    {
        temp.push_back(boost::python::extract<double>(b[i]));
    }

    // Update the a_coef vector with the new values
    b_coef.swap(temp);

    std::cout << "B coefficients set to: " << std::endl;
    for (std::vector<double>::iterator it = b_coef.begin(); it != b_coef.end(); ++it)
        std::cout << *it << ", ";
    std::cout << std::endl;
}

void scf::GeneralAnalogFilter::setGain(double g)
{
    gain = g;

    std::cout << "Gain set to: " << gain << std::endl;
}

const std::size_t scf::GeneralAnalogFilter::getNumCh() const
{
    return numCh;
}

void scf::GeneralAnalogFilter::reset()
{
    // Resize and re-initialize the data buffer
    std::vector< std::vector<output_data_t> >(order, std::vector<output_data_t>(numCh)).swap(data);

    // Check that a coefficient vector size is at least 'order + 1'.
    // If not, add expand it with zeros.
    if ( a_coef.size() < (order + 1) )
        a_coef.resize( order +  1, 0);

    // Check that b coefficient vector size is at least 'order + 1'.
    // If not, add expand it with zeros.
    if ( b_coef.size() < (order + 1) )
        b_coef.resize( order +  1, 0);
}

void scf::GeneralAnalogFilter::rxFrame(ris::FramePtr frame)
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

        // When the number of channel change, reset the filter
        reset();

        std::cout << "data.size() = " << data.size() << std::endl;
        if (data.size() > 0)
            std::cout << "data.at(0).size() = " << data.at(0).size() << std::endl;
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