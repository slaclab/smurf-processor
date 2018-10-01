
/*
 *-----------------------------------------------------------------------------
 * Title      : Source code for MceTransmit
 *-----------------------------------------------------------------------------
 * File       : exoTest.py
 * Created    : 2018-02-28
 *-----------------------------------------------------------------------------
 * This file is part of the rogue_example software. It is subject to
 * the license terms in the LICENSE.txt file found in the top-level directory
 * of this distribution and at:
 *    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
 * No part of the rogue_example software, inclu ding this file, may be
 * copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE.txt  file.
 *-----------------------------------------------------------------------------
*/



#include <rogue/interfaces/stream/Slave.h>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameIterator.h>
#include <boost/python.hpp>
#include <boost/python/module.hpp>
#include <smurf2mce.h>
#include <smurftcp.h>

void error(const char *msg){perror(msg);};    // modify later to deal with errors

uint64_t pull_bit_field(uint8_t *ptr, uint offset, uint width); 

namespace bp = boost::python;
namespace ris = rogue::interfaces::stream;

class Smurf2MCE : public rogue::interfaces::stream::Slave {
      
public:
  uint32_t rxCount, rxBytes, rxLast;
  uint32_t getCount() { return rxCount; } // Total frames
  uint32_t getBytes() { return rxBytes; } // Total Bytes
  uint32_t getLast()  { return rxLast;  } // Last frame size


  bool initialized;
  uint internal_counter;
  uint8_t *buffer; // holds raw input from PyRogute
  uint8_t *buffer_last; // holds last data
  uint8_t *b[2]; // dual buffers to allow last pulse subtraction
  int bufn;  // which buffer we are on.
  wrap_t *wrap_counter; // byte to track phase wraps. 
  char *mask; // masks which resonators we will use. 
  avgdata_t *average_samples; // holds the averaged sample data
  uint average_counter; // runnign counter of averages
  const char *port;  // character string that holds the port number
  const char *ip;  // character string that holds the ip addres or name
  Smurftcp *S; // tcp interface, use defaults for now.
  MCEHeader *M; // mce header class
  SmurfHeader *H; // Smurf header class
  SmurfConfig *C; // holds smurf configuratino class


  Smurf2MCE();
  void acceptFrame(ris::FramePtr frame);

  //void acceptframe_test(char* data, size_t size); // test version for local use, just a wrapper
  void process_frame(void); // does average, separates header
  void set_mask(char *new_mask, uint num); // set array mask .
  void clear_wrap(void){memset(wrap_counter, wrap_start, smurfsamples);}; // clears wrap counter
   ~Smurf2MCE(); // destructor



      // Expose methods to python
  static void setup_python() {
         bp::class_<Smurf2MCE, boost::shared_ptr<Smurf2MCE>, bp::bases<ris::Slave>, boost::noncopyable >("Smurf2MCE",bp::init<>())
            .def("getCount", &Smurf2MCE::getCount)
            .def("getBytes", &Smurf2MCE::getBytes)
            .def("getLast",  &Smurf2MCE::getLast)
         ;
         bp::implicitly_convertible<boost::shared_ptr<Smurf2MCE>, ris::SlavePtr>();
  };

};



Smurftcp::Smurftcp( const char* port_number,  const char* ip_string)
{
  printf("tcp connect %s    %s  \n",  port_number, ip_string);
  initialized = false;
  port = port_number;
  ip = ip_string;
  connected = false;
  connect_delay.tv_sec = 0;  // no seconds in delay connect
  connect_delay.tv_nsec = 100000000;  // 0.1 second. 
  if(!(tcpbuffer = (char*)malloc(tcplen)))
    {
      error("could not allocate write buffer");
      return;
    } 
  if(!(databuffer = (char*)malloc(datalen)))
    {
      error("could not allocate tcp  buffer");
      return;
    }
  signal(SIGPIPE, SIG_IGN);  // ignore broken pipe error, will reconnect on error
  initialized = true;
  connect_link(); // make tcp connection
}

bool Smurftcp::connect_link(void)
{
  if(connected) return(1);  // already connected
  disconnect_link(); // clean up previous link
  if(0 > (sockfd = socket(AF_INET, SOCK_STREAM,0)))   // creates a socket   stream
    {
      error("can't open socket");
      return(false);
    }
  if (getaddrinfo(ip, port, NULL, &server))
    {
      error("error trying to resolve address or port");
      return(false);
    }
  if (0 > connect(sockfd, server->ai_addr, server->ai_addrlen))
    {
      error("error connecting to socket");
      close(sockfd);
      sockfd = -1; 
      return(false);
    }
  return(connected = true);
}

bool Smurftcp::disconnect_link(void) // clean up link
{
  nanosleep(&connect_delay, NULL); // delay 100msec to avoid hammering on tcp connect
  if(!connected) return(false);  // already disconnected
  if (sockfd != -1  ) close(sockfd);
  printf("disconnecting \n");
  return(connected = false);
}




void Smurftcp::write_data(size_t bytes) // bytes is the input size, need to add buffer. 
{
  uint tmp, tst, j;
  uint bytes_written = 0;  // tracks how many bytes have been written.
  uint32_t *t; // just a kludge to add a header. 
  if (!connected)
    {
      printf("trying to re-connect");
     
      connect_link();
    } 
  t = (uint32_t*) databuffer; 
  *t++ = header; // mixcelaneous header
  *t = tcplen;
 
  for( j= 0; j < datalen; j++)  // split bytes to allow use of markers
  {
    tcpbuffer[2*j] = databuffer[j] & 0xF;
    tcpbuffer[2*j+1] = (databuffer[j] & 0xF0) >> 4;
  }
  tcpbuffer[0] = tcpbuffer[0] | 0x80; // add marker
  do{
    tmp = write(sockfd, tcpbuffer+bytes_written, tcplen - bytes_written);
    bytes_written += tmp; // increment bytes written pointer
    if (-1 == tmp)
      {
	connected = false;
	return;
      }
  }while(bytes_written < tcplen); 
}

char *Smurftcp::get_buffer_pointer()
{
  return(databuffer + tcp_header_size);  
}

Smurftcp::~Smurftcp() // destructor (probalby not needed)
{
  if (connected)
  {
    close(sockfd);
    printf("closing socket \n");
  }
}





Smurf2MCE::Smurf2MCE()
{
  rxCount = 0;
  rxBytes = 0;
  rxLast = 0; // from test program
  initialized = false;
  port = server_port_number;
  ip = server_ip_addr; 
  average_counter= 1; 
  bufn = 0; // current buffer 
  internal_counter = 0;
  int j; 


  S = new Smurftcp(port, ip);
  M = new MCEHeader();  // creates a MCE header class
  H = new SmurfHeader(); 
  C = new SmurfConfig(); // will hold config info - testing for now
  average_counter = 0; // counter used for test averaging , not  needed in real program
  for(j = 0; j < 2; j++)
    {  // allocate 2 buffers, so we can swap up/ back for background subtraction.
      if(!(b[j] = (uint8_t*)malloc(pyrogue_buffer_length)))  
	{
	  error("could not allocate smurf2mce buffer");
	  return;
	}
      //printf("buffer = %x, len = %d \n", b[j], pyrogue_buffer_length);
      memset(b[j], 0, pyrogue_buffer_length); // zero to start with
    }
  if(!(average_samples = (avgdata_t*)malloc(smurfsamples * sizeof(avgdata_t))))
    {
      error("could not allocate data sample buffer");
      return;
    }
  if(!(wrap_counter = (wrap_t*)malloc(smurfsamples * sizeof(wrap_t))))
    {
      error("could not allocate wrap_counter");
      return;
    }
  if(!(mask = (char*)malloc(pyrogue_buffer_length)))
    {
      error("could not allocate mask  buffer");
      return;
    }
  memset(average_samples, 0, smurfsamples * sizeof(avgdata_t)); // clear average data to start
  memset(mask, 0, pyrogue_buffer_length); // set to all off to start
  memset(wrap_counter, wrap_start, smurfsamples * sizeof(wrap_t));
  initialized = true;

}




void Smurf2MCE::process_frame(void)
{
  smurf_t *d, *p;  // d is this buffer, p is last buffer; 
  char *pm; 
  avgdata_t *a,  *astop; // used for averaging loop
  uint j, k; 
  uint actr; // counter for average array;
  uint dctr; // counter for input data array
  char *tcpbuf; 
  uint32_t cnt; 
  int tmp;
  MCE_t checksum;  // fixed for now, #2 for testing
  uint tcp_buflen; // holds filled lengthof tcp buffer
  uint32_t *bufx; // holds tcp buffer mapped to 32 bit for checksum
  H->copy_header(buffer); 
#if 0
  for (k = 0; k < 16; k++)
    {
      printf("|%1x", k); 
      for(j = 0; j < 8; j++)
	{
	  tmp = k * 8 + j; 
	  printf(" %2x",(0xFF & H->header[tmp]));
	}
    }
  printf("\n"); 
#endif
  if(!H->check_increment()) printf("bad increment %u \n", H->get_frame_counter());
  d = (smurf_t*) (buffer+smurfheaderlength); // pointer to data
  p =  (smurf_t*) (buffer_last+smurfheaderlength);  // pointer to previous data set
  astop = average_samples + smurfsamples;
  a = average_samples;
  for (actr = 0, dctr = 0; (dctr < pyrogue_buffer_length) && (actr < smurfsamples); dctr++) 
    {
      if (!mask[dctr]) continue;   // mask is zero, just continue loop counters. 
      if ((d[dctr] > upper_unwrap) && (p[dctr] < lower_unwrap)) // unwrap, add 1
	{
	  wrap_counter[actr]--; // decrement wrap counter
	} else if((d[dctr] < lower_unwrap) && (p[dctr] > upper_unwrap))
	{
	  wrap_counter[actr]++; // inccrement wrap counter
	}
	else; // nothing here
              // add counter wrap to data  
	a[actr++] += (avgdata_t)(d[dctr]) + 0x8000 + (0xFFFFFF &(((uint16_t) wrap_counter[actr])<<16));
    }
 
  if (!(cnt = H->average_control())) return;  // just average, otherwise send frame
  
  M->make_header(); // increments counters, readies counter
  M->set_word( mce_h_offset_status, mce_h_status_value);
  M->set_word( MCEheader_CC_counter_offset, M->CC_frame_counter);
  M->set_word( MCEheader_row_len_offset,  MCEheader_row_len_value);
  M->set_word( MCEheader_num_rows_reported_offset, MCEheader_num_rows_reported_value);
  M->set_word( MCEheader_data_rate_offset, cnt);  // use internal averaged frames
  M->set_word( MCEheader_CC_ARZ_counter, smurfsamples); 
  M->set_word( MCEheader_version_offset,  MCEheader_version); // can be in constructor
  M->set_word( MCEheader_num_rows_offset, MCEheader_num_rows_value); 


  M->set_word( MCEheader_syncbox_offset, H->get_syncword());
  for (j = 0; j < smurfsamples; j++)   // divide out number of samples
    average_samples[j] = (avgdata_t) (((double)average_samples[j])/cnt + average_sample_offset); // do in double
  tcpbuf = S->get_buffer_pointer();  // returns location to put data (8 bytes beyond tcp start)
  memcpy(tcpbuf, M->mce_header, MCEheaderlength * sizeof(MCE_t));  // copy over MCE header to output buffer
  memcpy(tcpbuf+ MCEheaderlength * sizeof(MCE_t), average_samples, smurfsamples * sizeof(avgdata_t)); //copy data 
  tcp_buflen =  MCEheaderlength * sizeof(MCE_t) + smurfsamples * sizeof(avgdata_t);
  bufx = (uint32_t*) tcpbuf;  // map buffer to 32 bit
  checksum  = bufx[0];

  for (j = 1; j < MCE_frame_length-1; j++) checksum =checksum ^ bufx[j]; // calculate checksum
 
  

  memcpy(tcpbuf+ MCEheaderlength * sizeof(MCE_t) + smurfsamples * sizeof(avgdata_t), &checksum, sizeof(MCE_t)); 
   if (!(internal_counter++ % 100))
     {
       printf( "avg= %3u, sync = %6u, intctr = %6u, frmctr = %6u\n", cnt, H->get_syncword(),internal_counter,
	     H->get_frame_counter());
       //printf("d = %x %x %x\n", bufx[6], bufx[MCE_frame_length-2], bufx[MCE_frame_length-1]);

     }


  S->write_data(MCEheaderlength * sizeof(MCE_t) + smurfsamples * sizeof(avgdata_t) + sizeof(MCE_t));
  memset(average_samples, 0, smurfsamples * sizeof(avgdata_t));
}






void Smurf2MCE::set_mask(char *new_mask, uint num)  // UGLY, need to do this rigth 
{
  int j, sum = 0;
  memcpy(mask,new_mask, num); // copies mask over
  for (j = 0; j < smurf_raw_samples; j++) // check for correct total points
    {
      if (sum == smurfsamples) mask[j] = 0;  // limit maximum mask size
      sum += mask[j]; 
    }
  if (sum < smurfsamples)
    {
      for (j = 0; j < smurf_raw_samples; j++)
	{
	  if (mask[j] == 0)
	    {
	      mask[j]++; 
	      sum++;
	      if (sum == smurfsamples) break;  // ok have right length mask. 
	    }
	}
    }
}


Smurf2MCE::~Smurf2MCE() // destructor
{
  if(S) delete S; 
}



SmurfHeader::SmurfHeader()
{
  //memset(header, 0, smurfheaderlength);  // clear initial falues
  last_frame_count = 0; 
  first_cycle = 1; 
  average_counter = 0;  // number of frames avearaged so far
  data_ok = true;  // start of assuming data is OK, invalidate later.
  average_ok = true; 
  last_ext_counter = 0;  // this tracks the counter rolling over
}

void SmurfHeader::copy_header(uint8_t *buffer)
{
  //memcpy(header, buffer, smurfheaderlength);
  header = buffer;  // just move the pointer 
  data_ok = true;  // This is where we first get new data so star with header OK. 
}


uint SmurfHeader::get_version(void)
{
  return(pull_bit_field(header, h_version_offset, h_version_width));
}


uint SmurfHeader::get_frame_counter(void)
{
  return(pull_bit_field(header, h_frame_counter_offset, h_frame_counter_width));
}

uint SmurfHeader::get_ext_counter(void)
{
  return(pull_bit_field(header, h_ext_counter_offset, h_ext_counter_width));
}

uint SmurfHeader::get_syncword(void)
{
  uint64_t x;
  x = pull_bit_field(header, h_mce_syncword_offset, h_mce_syncword_width); 
  return(x & 0xFFFFFFFF);  // pull out the counter. 
}


bool SmurfHeader::check_increment()
{
  uint x;
  bool ok = 0; 
  x = get_frame_counter();  if(first_cycle)
    {
      first_cycle = false;
      ok =  true;
    }
  else if (x == (last_frame_count + 1))
    {
      ok = true;;
    }
  else if (!(last_frame_count ^ ((1 << h_frame_counter_width)-1))) // all FFFF
    {
      ok = true; 
    }
  else 
    {
      ok = false; 
    }
  last_frame_count = x;
  if (!ok) data_ok = false; // invalidate data. 
  return(ok); 
}


uint SmurfHeader::average_control() // returns num averages when avearaging is done. 
{
  uint x=0, y; 
  if (average_counter ==0) average_ok = data_ok;  // reset avearge ok bit.
  average_counter++; // increment number of frames averaged. 
  y = get_ext_counter();
  if (last_ext_counter > y)  // TEST TEST TEST - until we have averaging bits
    {
      x = average_counter; // number of averages
      average_counter = 0; // reset average
    }
  last_ext_counter = y; // copy over counter
  return(x);  // return, 0 to kep averaging, otehr to zero average. 
}



MCEHeader::MCEHeader()
{
  memset(mce_header, 0, MCEheaderlength * sizeof(MCE_t));
  mce_header[MCEheader_version_offset] = MCEheader_version;  // current version.
  CC_frame_counter = 0; // counter for MCE frame
}


void MCEHeader::make_header(void)
{
  mce_header[MCEheader_CC_counter_offset] = CC_frame_counter++;  // increment counter, put in header
  return;
}

void MCEHeader::set_word(uint offset, uint32_t value)
{
  mce_header[offset] = value & 0xffffffff; // just write value. 
}


SmurfConfig::SmurfConfig(void)
{
  ready = false;  // has file ben read yet?
  filename = (char*) malloc(1024 * sizeof(char));
  variable = (char*) malloc(10000 * sizeof(char)); // big for now check later
  value = (char*) malloc(10000 * sizeof(char)); // big for now check later
  strcpy(filename, "smurf2mce.cfg");  // kludge for now. 
  ready = read_config_file(filename);  
}


bool SmurfConfig::read_config_file(char *fname)
{
  FILE *fp;
  int n;
  if(!( fp = fopen(fname,"r"))) return(false); // open config file
  do{
    n = fscanf(fp, "%s %s\n", variable, value);  // read into buffer
    if(n != 2) continue; // only know how to read 
    if(!strcmp(variable, "receiver_ip")) strncpy(ip, value, 100); // copy into IP string
    printf("found ip = %s \n", ip);

  }while (n);  // end when n ==0, end of file
  fclose(fp); // done with file
 }



void Smurf2MCE::acceptFrame ( ris::FramePtr frame ) 
{
  uint32_t nbytes = frame->getPayload();
  //printf("accept frame called \n");
  if (frame->getError() || (frame->getFlags() & 0x100))  // drop frame  (do we need to read out buffer?)
    {
      printf("frame error \n");
      return;  // don't copy data or process
    }
  buffer = b[bufn]; // buffer swap 
  bufn = bufn ? 0 : 1; // swap buffer reference
  buffer_last = b[bufn]; // now that we've swapped them
  rxLast   = nbytes;
  rxBytes += nbytes;
  rxCount++;
  int tmp, j, tmpsize; 

         // Iterators to start and end of frame
  rogue::interfaces::stream::Frame::iterator iter = frame->beginRead();
  rogue::interfaces::stream::Frame::iterator  end = frame->endRead();

         // Example destination for data copy
  // uint8_t *buff = (uint8_t *)malloc (nbytes);
  uint8_t  *dst = buffer; // was *dst = buff

         //Iterate through contigous buffers
  while ( iter != end ) 
    {

            //  Get contigous size
    auto size = iter.remBuffer ();

            // Get the data pointer from current position
    auto *src = iter.ptr ();

            // Copy some data
    memcpy(dst, src, size);

            // Update destination pointer and source iterator
    dst  += size;
    iter += size;
    tmpsize = size; // ugly, fix later
    } 
  process_frame();

}


uint64_t pull_bit_field(uint8_t *ptr, uint offset, uint width)
{
  uint64_t x;  // will hold version number
  uint64_t r;
  uint64_t tmp;
  if(width > sizeof(uint64_t)) error("field width too big"); 
  r = (1UL << (width*8)) -1; 
  memcpy(&x, ptr+offset, width); // move the bytes over
  tmp = r & (uint64_t)x; 
  return(r & tmp);
}







BOOST_PYTHON_MODULE(MceTransmit) {
   PyEval_InitThreads();
   try {
     Smurf2MCE::setup_python();
   } catch (...) {
      printf("Failed to load module. import rogue first\n");
   }
   printf("Loaded my module\n");
};



