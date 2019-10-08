#ifndef _SMURF_CORE_FILTERS_GENERALANALOGFILTER_H_
#define _SMURF_CORE_FILTERS_GENERALANALOGFILTER_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data GeneralAnalogFilter
 * ----------------------------------------------------------------------------
 * File          : GeneralAnalogFilter.h
 * Created       : 2019-09-27
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Data GeneralAnalogFilter Class.
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

#include <iostream>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameLock.h>
#include <rogue/interfaces/stream/FrameIterator.h>
#include <rogue/interfaces/stream/Slave.h>
#include <rogue/interfaces/stream/Master.h>
#include <rogue/GilRelease.h>
#include "smurf/core/common/SmurfHeader.h"
#include "smurf/core/common/Helpers.h"

namespace bp  = boost::python;
namespace ris = rogue::interfaces::stream;

namespace smurf
{
    namespace core
    {
        namespace filters
        {
            class GeneralAnalogFilter;
            typedef boost::shared_ptr<GeneralAnalogFilter> GeneralAnalogFilterPtr;

            // This class implements a general analog filter, in the same way it
            // was done in the original smurf2mce code. That is:
            //
            // y(n) = gain / a(0) * [ b(0) * x(n) + b(1) * x(n -1) + ... + b(order) * x(n - order + 1)
            //                                    - a(1) * y(n -1) - ... - a(order) * y(n - order + 1) ]
            //
            class GeneralAnalogFilter : public ris::Slave, public ris::Master
            {
            public:
                GeneralAnalogFilter();
                ~GeneralAnalogFilter() {};

                static GeneralAnalogFilterPtr create();

                static void setup_python();

                // Disable the processing block. The data
                // will just pass through to the next slave
                void       setDisable(bool d);
                const bool getDisable() const;

                // Set/Get the filter order
                void              setOrder(std::size_t o);
                const std::size_t getOrder() const;

                // Set/Get the filter a coefficients
                void           setA(bp::list l);
                const bp::list getA() const;


                // Set/Get the filter b coefficients
                void           setB(bp::list l);
                const bp::list getB() const;

                // Set/Get the filter gain
                void         setGain(double g);
                const double getGain() const;

                // Reset the filter. Resize and Zero-initialize the data buffer, and
                // check if the coefficient vectors have the correct size, and expand
                // if necessary, padding with zeros.
                void reset();

                // Accept new frames
                void acceptFrame(ris::FramePtr frame);

            private:
                // Data type used to read the data from the input frame
                typedef int32_t input_data_t;

                // Data type used to write data to the output frame
                typedef int32_t output_data_t;

                bool                disable; // Disable flag
                std::size_t         numCh;   // Number of channels being processed
                std::size_t         order;   // Filter order
                double              gain;    // Filter gain
                std::vector<double> a;       // Filter's a coefficients
                std::vector<double> b;       // Filter's b coefficients
                std::mutex          mut;     // Mutex

                // Data buffers, needed to store all the pass data points.
                // The outer vector's size will depend on the filter's order, and
                // the inner vector's size will depend on the number of channels being processed
                std::size_t lastPointIndex;                    // Index of older data point in the buffer
                std::vector< std::vector<output_data_t> > x;   // pass inputs
                std::vector< std::vector<output_data_t> > y;   // pass output


            };
        }
    }
}

#endif