#ifndef _SMURF_CORE_PROCESSORS_SMURFPROCESSOR_H_
#define _SMURF_CORE_PROCESSORS_SMURFPROCESSOR_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Data Processor
 * ----------------------------------------------------------------------------
 * File          : SmurfProcessor.h
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

#include <iostream>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameLock.h>
#include <rogue/interfaces/stream/FrameIterator.h>
#include <rogue/interfaces/stream/Buffer.h>
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
        namespace processors
        {
            class SmurfProcessor;
            typedef boost::shared_ptr<SmurfProcessor> SmurfProcessorPtr;

            class SmurfProcessor : public ris::Slave, public ris::Master
            {
            public:
                SmurfProcessor();
                ~SmurfProcessor() {};

                static SmurfProcessorPtr create();

                static void setup_python();

                // Disable the processing block. The data
                // will just pass through to the next slave
                void       setDisable(bool d);
                const bool getDisable() const;

                //** CHANNEL MAPPING METHODS **//
                const std::size_t   getNumCh() const;           // Get the number of mapper channels
                void                setMask(bp::list m);        // Set the Channel mask vector
                const bp::list      getMask() const;            // Get the Channel mask vector

                //** UNWRAPPER METHODS **//
                void                resetUnwrapper();           // Resize and clear buffer

                //** FILTER METHODS **//
                void                setOrder(std::size_t o);    // Set the filter order
                const std::size_t   getOrder() const;           // Get the filter order
                void                setA(bp::list l);           // Set the filter a coefficients
                const bp::list      getA() const;               // Get the filter a coefficients
                void                setB(bp::list l);           // Set the filter b coefficients
                const bp::list      getB() const;               // Get the filter b coefficients
                void                setGain(double g);          // Set the filter gain
                const double        getGain() const;            // Get the filter gain
                void                resetFilter();              // Reset the filter

                //** DOWNSAMLER METHODS **//
                void                setFactor(std::size_t f);   // Set the downsampling factor
                const std::size_t   getFactor() const;          // Get the downsampling factor
                void                resetDownsampler();         // Reset the downsampler.


                // Accept new frames
                void acceptFrame(ris::FramePtr frame);

            private:
                //** DATA TYPES ** //
                typedef int16_t fw_t;      // Data type from firmware
                typedef int32_t unwrap_t;  // Data type out after unwrap
                typedef int32_t filter_t;  // Data type after filter

                // ** CONSTANTS **//
                // This is the maximum number of channel we expect from an input frame.
                static const std::size_t maxNumInCh = 4096;

                // This is the maximum number of channel the output packet can hold.
                // The output frame size will be fixed to this size, even if not all
                // channel are mapped.
                static const std::size_t maxNumOutCh = 528;

                // Unwrap related constants
                const fw_t      upperUnwrap =  0x6000;          // If we are above this and jump, assume a wrap
                const fw_t      lowerUnwrap = -0x6000;          // If we are below this and jump, assume a wrap
                const unwrap_t   stepUnwrap = 0x10000;          // Wrap counter steps

                //** VARIABLES **//
                std::vector<uint8_t> frameBuffer;               // Buffer to copy the input frame into a STL container
                bool                     disable;               // Disable flag
                std::mutex               mut;                   // Mutex
                // Channel mapping variables
                std::size_t              numCh;                 // Number of channels being processed
                std::vector<std::size_t> mask;                  // Channel mask file
                // Unwrap variables
                std::vector<unwrap_t>    currentData;           // Current data buffer
                std::vector<unwrap_t>    previousData;          // Previous data buffer
                std::vector<unwrap_t>    wrapCounter;           // Wrap counters
                // Filter variables
                std::size_t              order;                 // Filter order
                double                   gain;                  // Filter gain
                std::vector<double>      a;                     // Filter's a coefficients
                std::vector<double>      b;                     // Filter's b coefficients
                std::size_t              currentBlockIndex;     // Index of current data point in the buffer
                std::vector<double>      x;                     // pass inputs
                std::vector<double>      y;                     // pass output
                std::vector<filter_t>    outData;               // Result
                // Downsampler variables
                std::size_t              factor;                // Downsample factor
                std::size_t              sampleCnt;             // Sample counter
                // Transmit thread
                std::vector<uint8_t>    headerCopy;             // A copy of header to be send
                std::vector<filter_t>   dataCopy;               // A copy of the data to be send
                bool                    txDataReady;            // Flag to indicate new data is ready t be sent
                std::atomic<bool>       runTxThread;            // Flag used to stop the thread
                std::thread             pktTransmitterThread;   // Thread to send the data to the next slave
                std::condition_variable txCV;                   // Variable to notify the thread new data is ready
                std::mutex              txMutex;                // Mutex used for accessing the conditional variable

                //** METHOD **//
                void                    pktTansmitter();        // Send frame to the next slave


            };
        }
    }
}

#endif