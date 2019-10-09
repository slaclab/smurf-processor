/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Processor
 * ----------------------------------------------------------------------------
 * File          : SmurfProcessor.cpp
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Data Processor Class.
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
#include "smurf/core/processors/SmurfProcessor.h"

namespace scp = smurf::core::processors;

scp::SmurfProcessor::SmurfProcessor()
:
    ris::Slave(),
    ris::Master(),
    disable(false),
    numCh(0),
    mask(maxNumOutCh,0),
    currentData(0),
    previousData(0),
    wrapCounter(0),
    order(0),
    gain(1),
    a(1,1),
    b(1,1),
    lastPointIndex(0),
    x( order, std::vector<filter_t>(numCh) ),
    y( order, std::vector<filter_t>(numCh) ),
    factor(1),
    sampleCnt(0),
    frameBuffer(SmurfHeader::SmurfHeaderSize + maxNumInCh * sizeof(fw_t))
{
}

scp::SmurfProcessorPtr scp::SmurfProcessor::create()
{
    return boost::make_shared<SmurfProcessor>();
}

void scp::SmurfProcessor::setup_python()
{
    bp::class_< scp::SmurfProcessor,
                scp::SmurfProcessorPtr,
                bp::bases<ris::Slave,ris::Master>,
                boost::noncopyable >
                ("SmurfProcessor",bp::init<>())
        .def("setDisable", &SmurfProcessor::setDisable)
        .def("getDisable", &SmurfProcessor::getDisable)
        // Channel mapping variables
        .def("getNumCh",   &SmurfProcessor::getNumCh)
        .def("setMask",    &SmurfProcessor::setMask)
        .def("getMask",    &SmurfProcessor::getMask)
        // Filter variables
        .def("setOrder",   &SmurfProcessor::setOrder)
        .def("getOrder",   &SmurfProcessor::getOrder)
        .def("setA",       &SmurfProcessor::setA)
        .def("getA",       &SmurfProcessor::getA)
        .def("setB",       &SmurfProcessor::setB)
        .def("getB",       &SmurfProcessor::getB)
        .def("setGain",    &SmurfProcessor::setGain)
        .def("getGain",    &SmurfProcessor::getGain)
        // Downsampler variables
        .def("setFactor",  &SmurfProcessor::setFactor)
        .def("getFactor",  &SmurfProcessor::getFactor)
    ;
    bp::implicitly_convertible< scp::SmurfProcessorPtr, ris::SlavePtr  >();
    bp::implicitly_convertible< scp::SmurfProcessorPtr, ris::MasterPtr >();
}

void scp::SmurfProcessor::setDisable(bool d)
{
    disable = d;
}

const bool scp::SmurfProcessor::getDisable() const
{
    return disable;
}

const std::size_t scp::SmurfProcessor::getNumCh() const
{
    return numCh;
}

void scp::SmurfProcessor::setMask(bp::list m)
{
    std::size_t listSize = len(m);

    // Check if the size of the list, is not greater than
    // the number of channels we can have in the output packet.
    if ( listSize > maxNumOutCh )
    {
        // This should go to a logger instead
        std::cerr << "ERROR: Trying to set a mask list of length = " << listSize \
                  << ", which is larger that the number of channel in a SMuRF packet = " \
                  <<  maxNumOutCh << std::endl;

        // Do not update the mask vector.
        return;
    }

    // We will use a temporal vector to hold the new data.
    // New data will be check as it is pushed to this vector. If there
    // are not error, this vector will be swap with 'mask'.
    std::vector<std::size_t> temp;

    for (std::size_t i{0}; i < listSize; ++i)
    {
        std::size_t val = bp::extract<std::size_t>(m[i]);

        // Check if the mask value is not greater than
        // the number of channel we received in the incoming frame
        if (val > maxNumInCh)
        {
            // This should go to a logger instead
            std::cerr << "ERROR: mask value at index " << i << " is " << val \
                      << ", which is greater the maximum number of channel we expect from an input frame = " \
                      << maxNumInCh << std::endl;

            // Do not update the mask vector.
            return;
        }

        // A valid number was found. Add it to the temporal vector
        temp.push_back(val);
    }

    // Take the mutex before changing the mask vector
    std::lock_guard<std::mutex> lock(mut);

    // At this point, all element in the mask list are valid.
    // Update the mask vector
    mask.swap(temp);

    // Update the number of mapped channels
    numCh = listSize;
}

const bp::list scp::SmurfProcessor::getMask() const
{
    bp::list temp;

    for (auto const &v : mask)
        temp.append(v);

    return temp;
}

void scp::SmurfProcessor::resetUnwrapper()
{
    std::vector<unwrap_t>(numCh).swap(currentData);
    std::vector<unwrap_t>(numCh).swap(previousData);
    std::vector<unwrap_t>(numCh).swap(wrapCounter);
}

void scp::SmurfProcessor::setOrder(std::size_t o)
{
    // Check if the new order is different from the current one
    if ( o != order )
    {
        // Take the mutex before changing the filter parameters
        // This make sure that the new order value is not used before
        // the a and b array are resized.
        std::lock_guard<std::mutex> lock(mut);

        order = o;

        // When the order it change, reset the filter
        resetFilter();
    }
}

const std::size_t scp::SmurfProcessor::getOrder() const
{
    return order;
}

void scp::SmurfProcessor::setA(bp::list l)
{
    std::vector<double> temp;

    // Take the mutex before changing the filter parameters
    // This make sure that the 'a' array is not used before it has
    // beem resized, if necessary.
    std::lock_guard<std::mutex> lock(mut);

    std::size_t listSize = len(l);

    // Verify that the input list is not empty.
    // If empty, set the coefficients vector to a = [1.0].
    if (listSize == 0)
    {
        // This should go to a logger instead
        std::cerr << "ERROR: Trying to set an empty set of a coefficients. Defaulting to 'a = [1.0]'"<< std::endl;
        temp.push_back(1.0);
        a.swap(temp);

        return;
    }

    // Verify that the first coefficient is not zero.
    // if it is, set the coefficients vector to a = [1.0].
    if (l[0] == 0)
    {
        // This should go to a logger instead
        std::cerr << "ERROR: The first a coefficient can not be zero. Defaulting to 'a = [1.0]'"<< std::endl;
        temp.push_back(1.0);
        a.swap(temp);
        return;
    }

    // Extract the coefficients coming from python into a temporal vector
    for (std::size_t i{0}; i < listSize; ++i)
    {
        temp.push_back(bp::extract<double>(l[i]));
    }

    // Update the a vector with the new values
    a.swap(temp);

    // Check that the a coefficient vector size is at least 'order + 1'.
    // If not, add expand it with zeros.
    if ( a.size() < (order + 1) )
        a.resize(order +  1, 0);
}

const bp::list scp::SmurfProcessor::getA() const
{
    bp::list temp;

    for (auto const &v : a)
        temp.append(v);

    return temp;
}

void scp::SmurfProcessor::setB(bp::list l)
{
    std::vector<double> temp;

    // Take the mutex before changing the filter parameters
    // This make sure that the 'b' array is not used before it has
    // beem resized, if necessary.
    std::lock_guard<std::mutex> lock(mut);

    std::size_t listSize = len(l);

    // Verify that the input list is not empty.
    // If empty, set the coefficients vector to a = [0.0].
    if (listSize == 0)
    {
        // This should go to a logger instead
        std::cerr << "ERROR: Trying to set an empty set of a coefficients. Defaulting to 'b = [0.0]'"<< std::endl;
        temp.push_back(0.0);
        b.swap(temp);

        return;
    }

    // Extract the coefficients coming from python into a temporal vector
    for (std::size_t i{0}; i < len(l); ++i)
    {
        temp.push_back(bp::extract<double>(l[i]));
    }

    // Update the a vector with the new values
    b.swap(temp);

    // Check that the b coefficient vector size is at least 'order + 1'.
    // If not, add expand it with zeros.
    if ( b.size() < (order + 1) )
        b.resize(order +  1, 0);
}

const bp::list scp::SmurfProcessor::getB() const
{
    bp::list temp;

    for (auto const &v : b)
        temp.append(v);

    return temp;
}

void scp::SmurfProcessor::setGain(double g)
{
    gain = g;
}

const double scp::SmurfProcessor::getGain() const
{
    return gain;
}

// Reset the filter. Resize and Zero-initialize the data buffer, and
// check if the coefficient vectors have the correct size, and expand
// if necessary, padding with zeros.
void scp::SmurfProcessor::resetFilter()
{
    // Resize and re-initialize the data buffer
    std::vector< std::vector<filter_t> >(order, std::vector<filter_t>(numCh)).swap(x);
    std::vector< std::vector<filter_t> >(order, std::vector<filter_t>(numCh)).swap(y);

    // Check that a coefficient vector size is at least 'order + 1'.
    // If not, add expand it with zeros.
    if ( a.size() < (order + 1) )
        a.resize(order +  1, 0);

    // Check that b coefficient vector size is at least 'order + 1'.
    // If not, add expand it with zeros.
    if ( b.size() < (order + 1) )
        b.resize(order +  1, 0);

    // Reset the index of the older point in the buffer
    lastPointIndex = 0;
}

void scp::SmurfProcessor::setFactor(std::size_t f)
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
    resetDownsampler();
}

const std::size_t scp::SmurfProcessor::getFactor() const
{
    return factor;
}

void scp::SmurfProcessor::resetDownsampler()
{
    sampleCnt = 0;
}

void scp::SmurfProcessor::acceptFrame(ris::FramePtr frame)
{

    Timer t{"acceptFrame"};

    // Copy the frame into a STL container
    {
        Timer t{"Frame2Buffer"};

        // Hold the frame lock
        ris::FrameLockPtr lockFrame{frame->lock()};

        std::vector<uint8_t>::iterator outIt(frameBuffer.begin());

        // Copy using BufferIterators in combination with std::copy for performance reasons
        for (ris::Frame::BufferIterator inIt=frame->beginBuffer(); inIt != frame->endBuffer(); ++inIt)
            outIt = std::copy((*inIt)->begin(), (*inIt)->endPayload(), outIt);
    }

    // Map and unwrap data at the same time
    {
        Timer t{"Map"};

        // Move the current data to the previous data
        previousData.swap(currentData);

        // Begining of the data area in the frameBuffer
        std::vector<uint8_t>::iterator inIt(frameBuffer.begin() + SmurfHeader::SmurfHeaderSize);

        // Output channel index
        std::size_t i{0};

        // Map and unwrap data in a single loop
        for(auto const& m : mask)
        {
            // Get the mapped value from the framweBuffer and cast it
            currentData.at(i) = static_cast<unwrap_t>(*(inIt + m * sizeof(fw_t)));

            // Check if the value wrapped
            if ((currentData.at(i) > upperUnwrap) && (previousData.at(i) < lowerUnwrap))
            {
                // Decrement wrap counter
                wrapCounter.at(i) -= stepUnwrap;
            }
            else if ((currentData.at(i) < lowerUnwrap) && (previousData.at(i) > upperUnwrap))
            {
                // Increment wrap counter
                wrapCounter.at(i) += stepUnwrap;
            }

            // Add the wrap counter to the value
            currentData.at(i) += wrapCounter.at(i);

            // increase output channel index
            ++i;
        }
    }

    // Filter data

    // Downsample data
}
