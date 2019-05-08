#include <time.h> // used for the test  sleep function. Just for the t est prgram.
//#include "smurf2mce.h"  // defines all the SMURF stuff

#ifndef __SMURFTCP_H__
#define __SMURFTCP_H__

#include "smurf_packet.h"

void error(const char *msg); // error handler


// class Smurftcp  // does data transfer
// {
// public:
//   bool initialized;  // has all data been allocated, connection made.
//   int sockfd;  // socket
//   struct addrinfo *server;  // will hold server address structure
//   char *databuffer; // data before splitting into nibbles for tcpbuffer
//   char *tcpbuffer;  // data sent over tcp, uses lower nibble, top bit as marker.
//   char ip[100];
//   char port[100];
//   bool connected; // are we connected?
//   timespec connect_delay; // holds delay time to prevent hammering on connect()

//   Smurftcp(char *port_number, char *ip_string);  // constructor
//   bool connect_link(bool disable, char* port_number, char *ip_string); // tries to make tcp connection
//   bool disconnect_link(void); // cleans up link to try again
//   char *get_buffer_pointer(void );// returns pointer for writing data
//   void write_data(size_t bytes); // writes data to tcp, (does most of the work).


//   ~Smurftcp(); // destructor, probably not needed
// };

// class MCEHeader // generates the MCE header data
// {
// public:
//   MCE_t CC_frame_counter; // counts for each MCE frame
//   MCE_t mce_header[MCEheaderlength]; // creates header with counter

//   MCEHeader(void);  // creates header
//   void make_header(void); // creates new header, icrements counters etc.
//   void set_word(uint offset, uint32_t value); // set word in header

// };

class SmurfConfig  // controls smurf config, initially just reads config file, future - epics interface
{
 public:
  char *filename; // holds name of config file
  bool ready;  //file has been read, readyh to run. w
  int num_averages;  // for use when we are not using the external average trigger
  // char receiver_ip[40]; // stored ip address in text!
  // char port_number[8]; // por number for tcp connection, in text!
  char data_file_name[1024]; // name of data file including directory, but without unix time extension
  int file_name_extend; // 1 (default) is append time, 0 is no append, more in future
  int data_frames; // number of smples per output file.
  int filter_order; // for low pass filter
  filter_t filter_g;
  filter_t filter_a[16]; //for filter
  filter_t filter_b[16];

  SmurfConfig(void);
  bool read_config_file(void);
};

class SmurfTestData // generates test data, 4096 samples
{
 public:
  uint smurf_samples;
  uint MCE_samples;
  uint counter;
  uint toggle;
  uint16_t counter16;
  uint initial_sync; // initial syncbox number
  bool init;
  timespec delaytime; // used for forced frame drop

  SmurfTestData(uint smurf_samples_in, uint MCE_samples_in);  // doesn't need to do anythign yet
  smurf_t *gen_test_smurf_data(smurf_t* input, uint mode, uint sync, uint8_t param); // modifies smurf input data
  avgdata_t *gen_test_mce_data(avgdata_t *input, uint mode, uint sync, uint8_t param); // modifies MCE output data (give pointer to data)
};

// test data modes
// mode 0: normal opearation
// mode 1: set input smurf data to 0
// mode 2: set input smurf data to equal channel number
// mode 3: ch0 steps -20,000 to +20,000 on sync word / 1000
// mode 4: even spaced random number total range 1000 counts
// mode 5: sine waves frequency is (param+1)*flux_ramp_rate / 2^16 samples on all channels
// mode 8: set output mce data to zero
// mode 9: set output mce data equal to channel numbber
// mode 10: MCE output ramped data
// mode 14: force checksum errors 1/1000 mce frames
// mode 15: MCE force droppd frames, delay 250ms+param(ms) 1/1000 mce frames



class SmurfDataFile // writes data file to disk
{
  bool open_;
  int  part_;
 public:
  char *filename; // name with timestampe
  uint frame_counter; // counts  number of frames written
  uint header_length; // size of header
  uint sample_points; // sample points in frame
  uint8_t  *frame; // will hold frame data before writing
  uint fd; // file pointer

  SmurfDataFile(void);
  uint write_file(SmurfPacket_RO packet, SmurfConfig *config);
  // writes to file, creates new if needded. return frames written, 0 new.
};


class SmurfTime
{
 public:
  uint64_t current;  //current value
  uint64_t delta; // difference
  uint mindelta;
  uint maxdelta;
  uint max_allowed_delta;
  uint min_allowed_delta;
  uint error_count;


  SmurfTime(void);
  bool update(uint64_t val); // updates, takes delta moves current to previous, returns true if jump
  void reset(void){mindelta = 1000000; maxdelta = 0;};
};


// checks data timers etc.  Call before first branch out of fast loop
class SmurfValidCheck
{
 public:
  FILE *fp; // holds diagnoistc file info
  //SmurfTime *Unix_time;
  SmurfTime *Syncbox;
  SmurfTime *Timingsystem;
  SmurfTime *Counter_1hz;
  SmurfTime *Smurf_frame;
  SmurfTime *Unix_time;
  SmurfTime *Smurf2mce; // delay from initial call
  bool init;   // set after first data taken
  bool ready;
  uint missed_syncbox;
  uint last_frame_jump;  // frame# at last jump,
  uint frame_wait;  //wait n frames before reporting another jump to prevent overloading file
  uint64_t initial_timing_system;

  SmurfValidCheck(void);  // just initializes
  void run(SmurfHeader *H); // gets all timer differences
  void reset(void);
};

// Filter data fnction
// y(n) = (1/a(1))* b(1)*x(n) + b(2)*x(n-1) + b(nb+1)*x(n-nb) - a(2)*y(n-1) - a(nd+1)*y(n-nb)\
// from matlab docs implmentatin of general analog filter
// Special: if order = -1, use integrating filter, clear returns last integral /  samples
class SmurfFilter
{
 public:
  uint samples;  // 528 for smurf
  uint records; // number of past buffers,enough for 8th order filter
  filter_t *xd;  // memory block with input ring buffer (xd + records * sample) + sample
  filter_t *yd;    // array of ring buffer pointers output data from filter
  int bn;  // number of most recent ring buffer pointers
  uint samples_since_clear; // internal use
  avgdata_t *output; // output data
  bool clear;  // true if data is already cleared
  int order_n;

  SmurfFilter(uint num_samples, uint num_records); // allocates arrays
  void clear_filter(void);  // returns last sample, clears all arrays, resets ring buffer pointers,
  void end_run(void);
  avgdata_t *filter(avgdata_t *data, int order, filter_t *a, filter_t *b, filter_t g); // input channnle array, outputs filtered channel array
};





#endif
