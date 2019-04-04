#ifndef _SMURFPROCESSOR_H_
#define _SMURFPROCESSOR_H_

#include <rogue/interfaces/stream/Slave.h>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameIterator.h>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameLock.h>
#include <rogue/interfaces/stream/FrameIterator.h>
#include <rogue/interfaces/stream/Buffer.h>
#include <rogue/GilRelease.h>
#include <smurf2mce.h>
#include <smurftcp.h>
#include "data_buffer.h"

namespace bp = boost::python;
namespace ris = rogue::interfaces::stream;

void error(const char *msg);
uint64_t pull_bit_field(uint8_t *ptr, uint offset, uint width);
uint64_t get_unix_time(); // returns unix sysetm time as 64 bit nanoseconds

// Data type of the SMuRF packet buffer
typedef DataBuffer<uint8_t> smurf_packet_buffer_t;

// SmurfProcessor "acceptframe" is called by python for each smurf frame
// Smurf2mce definition should be in smurftcp.h, but doesn't work, not sure why
class SmurfProcessor : public rogue::interfaces::stream::Slave
{

  bool debug_;

  static const unsigned queueDepth = 4000;

// Queue
  rogue::Queue<boost::shared_ptr<rogue::interfaces::stream::Frame>> queue_;
// Transmission thread
  boost::thread* thread_;
//! Thread background
  void runThread(const char* endpoint);

  // Buffer for SMuRF packet passed to the transmit thread.
  smurf_packet_buffer_t packetBuffer;

public:
  uint32_t rxCount, rxBytes, rxLast;
  uint32_t getCount() { return rxCount; } // Total frames
  uint32_t getBytes() { return rxBytes; } // Total Bytes
  uint32_t getLast()  { return rxLast;  } // Last frame size
  void     setDebug(bool debug) { debug_ = debug; return;  } // Last frame size


  bool initialized;
  uint internal_counter, fast_internal_counter;  // first is mce frames, second is smurf frames
  uint8_t *buffer; // holds raw input from PyRogute
  uint8_t *buffer_last; // holds last data
  uint8_t *b[2]; // dual buffers to allow last pulse subtraction
  int bufn;  // which buffer we are on.
  wrap_t *wrap_counter; // byte to track phase wraps.
  uint *mask; // masks which resonators we will use.
  avgdata_t *average_samples; // holds the averaged sample data (allocated in filter module)
  // avgdata_t *average_mce_samples; // samples modified for MCE format
  avgdata_t *input_data; // with unwrap, before aveaging
  uint average_counter; // runnign counter of averages
  const char *port;  // character string that holds the port number
  const char *ip;  // character string that holds the ip addres or name
  uint last_syncword;
  uint frame_error_counter;
  uint last_frame_counter;
  uint last_1hz_counter;
  uint last_epicsns;


  // MCEHeader *M; // mce header class
  SmurfHeader *H; // Smurf header class
  SmurfConfig *C; // holds smurf configuratino class
  SmurfDataFile *D; // outptut file for saving smurf data.
  SmurfValidCheck *V; // checks timing etc .
  SmurfFilter *F; // does low pass filter
  SmurfTestData *T; // genreates test data


  SmurfProcessor();
  void acceptFrame(ris::FramePtr frame);
  void frameToBuffer(ris::FramePtr frame, uint8_t * const buffer);

  //void acceptframe_test(char* data, size_t size); // test version for local use, just a wrapper
  void read_mask(char *filename);// reads file to create maks
  void clear_wrap(void){memset(wrap_counter, wrap_start, smurfsamples);}; // clears wrap counter
   ~SmurfProcessor(); // destructor

      // Expose methods to python
  static void setup_python()
  {
    bp::class_<SmurfProcessor, boost::shared_ptr<SmurfProcessor>, bp::bases<ris::Slave>, boost::noncopyable >("SmurfProcessor",bp::init<>())
      .def("getCount", &SmurfProcessor::getCount)
      .def("getBytes", &SmurfProcessor::getBytes)
      .def("getLast",  &SmurfProcessor::getLast)
      .def("setDebug",  &SmurfProcessor::setDebug)
    ;

    bp::implicitly_convertible<boost::shared_ptr<SmurfProcessor>, ris::SlavePtr>();
  };

};

#endif