#include <time.h> // used for the test  sleep function. Just for the t est prgram. 
#include "smurf2mce.h"  // defines all the SMURF stuff

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
  MCE_t mce_header[MCEheaderlength]; // header.
  MCEHeader(void); // creates header  
};


class SmurfHeader //generates and decodes SMURF data header
{
public:
  char header[smurfheaderlength]; // full header bytes
  SmurfHeader(void); // creates header with num samples
  void set_average_bit(int n);
  void clear_average_bit(int n);
  int get_average_bit(int n); 
};


#if 0
class Smurf2MCE // Translates from SMuRF data to MCE data, also unwraps
{
public:
  bool initialized;
  char *buffer; // holds raw input from PyRogute
  char *buffer_last; // holds last data
  char *b[2]; // dual buffers to allow last pulse subtraction
  int bufn;  // which buffer we are on.
  wrap_t *wrap_counter; // byte to track phase wraps. 
  char *mask; // masks which resonators we will use. 
  //char *header; // holds smurf header data .
  avgdata_t *average_samples; // holds the averaged sample data
  uint average_counter; // runnign counter of averages
  const char *port;  // character string that holds the port number
  const char *ip;  // character string that holds the ip addres or name
  Smurftcp *S; // tcp interface, use defaults for now.
  MCEHeader *M; // mce header class
  SmurfHeader *H; // Smurf header class

  Smurf2MCE(const char *port_number, const char *ip_string);
  void acceptframe_test(char* data, size_t size); // test version for local use, just a wrapper
  void process_frame(void); // does average, separates header
  uint process_header(void); // converts SMuRF header to MCE header. Return is number of averages
  void set_mask(char *new_mask, uint num); // set array mask .
  void clear_wrap(void){memset(wrap_counter, wrap_start, smurfsamples);}; // clears wrap counter
  ~Smurf2MCE(); // destructor
};

#endif
