#include <time.h> // used for the test  sleep function. Just for the t est prgram. 
//#include "smurf2mce.h"  // defines all the SMURF stuff

#ifndef __SMURFTCP_H__
#define __SMURFTCP_H__

void error(const char *msg); // error handler


class Smurftcp  // does data transfer
{
public:
  bool initialized;  // has all data been allocated, connection made. 
  int sockfd;  // socket 
  struct addrinfo *server;  // will hold server address structure
  char *databuffer; // data before splitting into nibbles for tcpbuffer
  char *tcpbuffer;  // data sent over tcp, uses lower nibble, top bit as marker.
  const char *ip;
  const char *port;  
  bool connected; // are we connected? 
  timespec connect_delay; // holds delay time to prevent hammering on connect()

  Smurftcp(const char *port_number, const char *ip_string);  // constructor
  bool connect_link(void); // tries to make tcp connection
  bool disconnect_link(void); // cleans up link to try again
  char *get_buffer_pointer(void );// returns pointer for writing data
  void write_data(size_t bytes); // writes data to tcp, (does most of the work).
  

  ~Smurftcp(); // destructor, probably not needed
};

class MCEHeader // generates the MCE header data
{
public:
  MCE_t CC_frame_counter; // counts for each MCE frame
  MCE_t mce_header[MCEheaderlength]; // creates header with counter

  MCEHeader(void);  // creates header
  void make_header(void); // creates new header, icrements counters etc.
  void set_word(uint offset, uint32_t value); // set word in header
  
};


class SmurfHeader //generates and decodes SMURF data header
{
public:
  uint8_t *header; // full header bytes
  uint last_frame_count; 
  bool first_cycle; 
  bool data_ok; // set to indicate taht data has passed internal checks.
  bool average_ok;  // set for a bad header somewhere in average
  uint average_counter; 
  uint last_ext_counter; 
  uint last_syncword; 
  uint delta_syncword;
  uint64_t bigtimems; // time in millliseconds
  uint64_t lastbigtime; 
  uint64_t unix_dtime; 
  uint32_t epics_seconds; 
  uint32_t epics_nanoseconds;

  SmurfHeader(void); // creates header with num samples
  void copy_header(uint8_t *buffer); 
  uint get_version(void); 
  uint get_ext_counter(void);
  uint get_1hz_counter(void); 
  uint get_frame_counter(void);
  bool check_increment(void); // checks that the frame counter i ncremented by 1;
  uint get_average_bit(void) { return(0);}; // place holder 
  uint get_syncword(void); // returns 20 bit MCE sync word 
  uint get_epics_nanoseconds(void);
  uint get_epics_seconds(void); 
  uint get_clear_bit(void);  // 1 means clear averaging and unwrap
  uint disable_file_write(void); // 1 means don't write a local output file
  uint disable_stream(void); // 1 means don't stream to MCE 
  uint average_control(int num); // num=0 means use external average,  
  uint get_num_rows(void);  // num rows from header
  uint get_num_rows_reported(void);
  uint get_row_len(void);
  uint get_data_rate(void);
  
  void clear_average(); // clears aveage counters
};


class SmurfConfig  // controls smurf config, initially just reads config file, future - epics interface
{
 public:
  char *filename; // holds name of config file
  bool ready;  //file has been read, readyh to run. w 
  int num_averages;  // for use when we are not using the external average trigger
  char receiver_ip[20]; // stored ip address in text!
  char port_number[8]; // por number for tcp connection, in text!
  char data_file_name[1024]; // name of data file including directory, but without unix time extension
  int data_frames; // number of smples per output file. 
  
  
  SmurfConfig(void);
  bool read_config_file(void);
};


class SmurfDataFile // writes data file to disk
{
 public:
  char *filename; // name with timestampe
  uint frame_counter; // counts  number of frames written
  uint header_length; // size of header
  uint sample_points; // sample points in frame
  uint8_t  *frame; // will hold frame data before writing
  uint fd; // file pointer

  SmurfDataFile(void);
  uint write_file(uint8_t *header, uint header_bytes, avgdata_t *data, uint data_words, uint frames_to_write, char *fname, bool disable);
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
  uint error_count; 

  SmurfTime(void);
  void update(uint64_t val); // updates, takes delta moves current to previous
  void reset(void){mindelta = 1000000; maxdelta = 0;};
};


// checks data timers etc.  Call before first branch out of fast loop
class SmurfValidCheck
{
 public:
  
  SmurfTime *Unix_time;
  SmurfTime *Syncbox;
  SmurfTime *Timingsystem;
  SmurfTime *Counter_1hz;
  SmurfTime *Smurf_frame;
  bool init;   // set after first data taken
  uint missed_syncbox; 

  SmurfValidCheck(void);  // just initializes
  void run(SmurfHeader *H); // gets all timer differences
  void reset(void); 
};


#endif
