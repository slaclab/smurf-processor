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
#include <thread>
#include <atomic>
#include <smurf2mce.h>
#include <smurftcp.h>
#include "common.h"
#include "data_buffer.h"
#include "smurf_packet.h"

namespace bp = boost::python;
namespace ris = rogue::interfaces::stream;

// SmurfProcessor "acceptframe" is called by python for each smurf frame
// Smurf2mce definition should be in smurftcp.h, but doesn't work, not sure why
class SmurfProcessor : public rogue::interfaces::stream::Slave
{
public:
  uint32_t rxCount, rxBytes, rxLast;
  uint32_t    getCount()            { return rxCount;         } // Total frames
  uint32_t    getBytes()            { return rxBytes;         } // Total Bytes
  uint32_t    getLast()             { return rxLast;          } // Last frame size
  void        setDebug(bool debug)  { debug_ = debug; return; } // Last frame size
  std::size_t getFrameLossCnt()     { return frameLossCnt;    } // Get the lost frame counter
  void        clearFrameLossCnt()   { frameLossCnt = 0;       } // Clear the lost frame

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
  // const char *port;  // character string that holds the port number
  // const char *ip;  // character string that holds the ip addres or name
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
  virtual ~SmurfProcessor(); // destructor

      // Expose methods to python
  static void setup_python()
  {
    bp::class_<SmurfProcessor, boost::shared_ptr<SmurfProcessor>, bp::bases<ris::Slave>, boost::noncopyable >("SmurfProcessor",bp::init<>())
      .def("getCount", &SmurfProcessor::getCount)
      .def("getBytes", &SmurfProcessor::getBytes)
      .def("getLast",  &SmurfProcessor::getLast)
      .def("setDebug",  &SmurfProcessor::setDebug)
      .def("getFrameLossCnt", &Smurf2MCE::getFrameLossCnt)
      .def("clearFrameLossCnt", &Smurf2MCE::clearFrameLossCnt)
      .def("printTransmitStatistic", &SmurfProcessor::printTransmitStatistic)
    ;

    bp::implicitly_convertible<boost::shared_ptr<SmurfProcessor>, ris::SlavePtr>();
  };


  // Transmit method. Will run in the pktTransmitterThread thread.
  // Here is where the method 'transmit' is called.
  void pktTansmitter();

  // File writer method. Will run in the pktWriterThread thread.
  // Here is where new SMuRF packet will be written to files.
  void pktWriter();

  // This method is intended to be used to take SMuRF packet and send them to other
  // system.
  // This method is called whenever a new SMuRF packet is ready, and a SmurfPacket_RO object
  // (which is a smart pointer to a read-only interface to a Smurf packer object) is passed.
  // It must be overwritten by the user application
  virtual void transmit(SmurfPacket_RO sp) {};

  // Print statistic about the transmit methods
  void printTransmitStatistic() const;

private:
  bool debug_;
  static const unsigned queueDepth = 4000;
  // Queue
  rogue::Queue<boost::shared_ptr<rogue::interfaces::stream::Frame>> queue_;
  // Transmission thread
  boost::thread* thread_;
  //! Thread background
  void runThread();

  DataBuffer          txBuffer;             // Buffer for SMuRF packet passed to the transmit thread.
  boost::atomic<bool> runTxThread;          // Flag to indicate the TX thread to stop its loops
  const size_t        pktReaderIndexTx;     // Data buffer reader index for the transmitter
  const size_t        pktReaderIndexFile;   // Data buffer reader index for the file writer
  std::thread         pktTransmitterThread; // Thread where the SMuRF packet transmission will run
  std::thread         pktWriterThread;      // Thread where the SMuRF packet file writer will run
  std::size_t         frameLossCnt;         // Lost frame counter
};

#endif