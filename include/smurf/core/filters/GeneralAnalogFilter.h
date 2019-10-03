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
#include "smurf/core/common/BaseSlave.h"
#include "smurf/core/common/BaseMaster.h"
#include "smurf/core/common/SmurfHeader.h"
#include "smurf/core/common/Helpers.h"

namespace bp  = boost::python;
namespace ris = rogue::interfaces::stream;
namespace scc = smurf::core::common;

namespace smurf
{
    namespace core
    {
        namespace filters
        {
            class GeneralAnalogFilter;
            typedef boost::shared_ptr<GeneralAnalogFilter> GeneralAnalogFilterPtr;

            class GeneralAnalogFilter : public scc::BaseSlave, public scc::BaseMaster
            {
            public:
                GeneralAnalogFilter();
                ~GeneralAnalogFilter() {};

                static GeneralAnalogFilterPtr create();

                static void setup_python();

                // This will be call by the BaseSlave class after updating
                // the base counters
                void rxFrame(ris::FramePtr frame);

                // Set the filter order
                void setOrder(std::size_t o);

                // Set the filter a coefficients
                void setA(boost::python::list a);

                // Set the filter b coefficients
                void setB(boost::python::list b);

                // Set the filter gain
                void setGain(double g);

                // Get the number of mapper channels
                const std::size_t getNumCh() const;

                // Reset the filter. Resize and Zero-initialize the data buffer, and
                // check if the coefficient vectors have the correct size, and expand
                // if necessary, padding with zeros.
                void reset();

            private:
                // Data type used to read the data from the input frame
                typedef int32_t input_data_t;

                // Data type used to write data to the output frame
                typedef int32_t output_data_t;

                std::size_t         numCh;  // Number of channels being processed
                std::size_t         order;  // Filter order
                double              gain;   // Filter gain
                std::vector<double> a_coef; // Filter's a coefficients
                std::vector<double> b_coef; // Filter's b coefficients

                // Data vector, needed to store all the pass data points.
                // The outer vector's size will depend on the filter's order, and
                // the inner vector's size will depend on the number of channels being processed
                std::vector< std::vector<output_data_t> > data;

            };
        }
    }
}

#endif