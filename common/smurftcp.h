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
  void set_syncword(uint value); // puts syncword in header
 
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

  SmurfHeader(void); // creates header with num samples
  void copy_header(uint8_t *buffer); 
  uint get_version(void); 
  uint get_ext_counter(void);
  uint get_frame_counter(void);
  bool check_increment(void); // checks that the frame counter incremented by 1;
  uint get_average_bit(void) { return(0);}; // place holder 
  uint get_syncword(void); // returns 20 bit MCE sync word 
  uint average_control(void); 

};



#endif
