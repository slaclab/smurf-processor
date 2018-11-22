
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

void error(const char *msg){perror(msg);};    // modify later to deal w ith errors

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
  uint *mask; // masks which resonators we will use. 
  avgdata_t *average_samples; // holds the averaged sample data (allocated in filter module)
  avgdata_t *average_mce_samples; // samples modified for MCE format
  avgdata_t *input_data; // with unwrap, before aveaging
  uint average_counter; // runnign counter of averages
  const char *port;  // character string that holds the port number
  const char *ip;  // character string that holds the ip addres or name
  uint last_syncword;
  uint frame_error_counter;
  uint last_frame_counter; 
  uint last_1hz_counter; 

  Smurftcp *S; // tcp interface, use defaults for now.
  MCEHeader *M; // mce header class
  SmurfHeader *H; // Smurf header class
  SmurfConfig *C; // holds smurf configuratino class
  SmurfDataFile *D; // outptut file for saving smurf data. 
  SmurfValidCheck *V; // checks timing etc . 
  SmurfFilter *F; // does low pass filter

  Smurf2MCE();
  void acceptFrame(ris::FramePtr frame);

  //void acceptframe_test(char* data, size_t size); // test version for local use, just a wrapper
  void process_frame(void); // does average, separates header
  void read_mask(char *filename);// reads file to create maks
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
  connect_delay.tv_sec = 0;  // 0 seconds in delay connect
  connect_delay.tv_nsec = 1000000;  // 0.001 second. 
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
  connect_link(0); // make tcp connection, 0 means don't disable
}

bool Smurftcp::connect_link(bool disable)
{
  if(connected) return(1);  // already connected
  if(!isdigit(port[0])) return(0); // can't connect, not a valid port
  disconnect_link(); // clean up previous link
  if (disable) return(0); // if connect is disabled
  if(0 > (sockfd = socket(AF_INET, SOCK_STREAM, 0)))   // creates a socket  
    {
      error("can't open socket");
      return(false);
    }
  // fcntl(sockfd, F_SETFL, O_NONBLOCK); // ADDED TO FIX BLOCKING doesn't work, won't connect
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
  // for select
  fd_set wfds;
  struct timeval tv;
  int isst = 0; 
  tv.tv_sec = 0;
  tv.tv_usec = 10000;  //10msec. 
  // end for select

  if (!connected)return;  // can't send
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
 
    // select stuff
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);
    if (-1 == select(sockfd+1, NULL, &wfds, NULL, &tv)) // can we write?
	{
	  printf("select error \n");
	  return;
	}
    isst = FD_ISSET(sockfd, &wfds); 
    if(!isst) return;  // just give up
   // end select stuff

    tmp = write(sockfd, tcpbuffer+bytes_written, tcplen - bytes_written);
    bytes_written += tmp; // increment bytes written  pointer
   
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
  average_counter= 1; 
  bufn = 0; // current buffer 
  internal_counter = 0;
  int j; 
  last_syncword = 0;
  last_1hz_counter = 0;
  frame_error_counter = 0; 
  last_frame_counter = 0;
  
  C = new SmurfConfig(); // will hold config info - testing for now
  S = new Smurftcp(C->port_number, C->receiver_ip);
  M = new MCEHeader();  // creates a MCE header class
  H = new SmurfHeader(); 
  D = new SmurfDataFile();  // holds output data
  V = new SmurfValidCheck();
  F = new SmurfFilter(smurfsamples, 16);  // 
 
  average_counter = 0; // counter used for test averaging , not  needed in real program
  for(j = 0; j < 2; j++)
    {  // allocate 2 buffers, so we can swap up/ back for background subtraction.
      if(!(b[j] = (uint8_t*)malloc(pyrogue_buffer_length)))  
	{
	  error("could not allocate smurf2mce buffer");
	  return;
	}
      memset(b[j], 0, pyrogue_buffer_length); // zero to start with
    }
 if(!(average_mce_samples = (avgdata_t*)malloc(smurfsamples * sizeof(avgdata_t))))
    {
      error("could not allocate mce data sample buffer");
      return;
    }
 if(!(input_data = (avgdata_t*)malloc(smurfsamples * sizeof(avgdata_t))))
    {
      error("could not allocate input data sample buffer");
      return;
    }
  if(!(wrap_counter = (wrap_t*)malloc(smurfsamples * sizeof(wrap_t))))
    {
      error("could not allocate wrap_counter");
      return;
    }
  if(!(mask = (uint*)malloc(smurfsamples * sizeof(uint))))
    {
      error("could not allocate mask  buffer");
      return;
    }
 
  memset(mask, 0, smurfsamples * sizeof(uint)); // set to all off to start
  memset(input_data, 0, smurfsamples * sizeof(avgdata_t)); // set to all off to start
  read_mask(NULL);  // will use real file name later
  memset(wrap_counter, wrap_start, smurfsamples * sizeof(wrap_t));
  initialized = true;

}




void Smurf2MCE::process_frame(void)
{
  smurf_t *d, *p;  // d is this buffer, p is last buffer; 
  char *pm; 
  //avgdata_t *a; // used for averaging loop
  uint j, k; 
  uint dctr; // counter for input data array
  char *tcpbuf; 
  uint32_t cnt; 
  int tmp;
  MCE_t checksum;  // fixed for now, #2 for testing
  uint tcp_buflen; // holds filled length oftcp buffer
  uint32_t *bufx; // holds tcp buffer mapped to 32 bit for checksum
  uint32_t avgtmp; 
  smurf_t dx; // current sample in loop
  
  
  H->copy_header(buffer); 
  d = (smurf_t*) (buffer+smurfheaderlength); // pointer to data
  p =  (smurf_t*) (buffer_last+smurfheaderlength);  // pointer to previous data set
 
  for(j = 0; j < smurfsamples; j++)
    {
      dctr = mask[j];
      if((mask[j] < 0) || (mask[j] > 4095))
	{
	  printf("bad mask %u \n", mask[j]); 
	  break;
	}
      
      if ((d[dctr] > upper_unwrap) && (p[dctr] < lower_unwrap)) // unwrap, add 1
	{
	  wrap_counter[j]-= 0x10000; // decrement wrap counter
	} else if((d[dctr] < lower_unwrap) && (p[dctr] > upper_unwrap))
	{
	  wrap_counter[j]+= 0x10000; // inccrement wrap counter
	}
	else; // nothing here
      input_data[j] = (avgdata_t)(d[dctr]) + (avgdata_t) wrap_counter[j];
    }
  
  average_samples = F->filter(input_data, C->filter_order, C->filter_a, C->filter_b); // Low Pass Filter

  cnt = H->average_control(C->num_averages);
  V->run(H);
  if(H->get_clear_bit()) // clear averages and wraps
    {
      memset(wrap_counter, wrap_start, smurfsamples * sizeof(wrap_t));  // clear wraps
      H->clear_average();  // clears averaging
      F->clear_filter();
    }

  if (!cnt)
    { 
      read_mask(NULL);  // re-read the mask file while held at zero
      last_frame_counter = H->get_frame_counter();
      last_1hz_counter = H->get_1hz_counter();
      return;  // just average, otherwise send frame  END OF FAST LOOP **************************
    }
  F->end_run();  // clears if we are doing a straight average
  M->make_header(); // increments counters, readies counter
  M->set_word( mce_h_offset_status, mce_h_status_value);
  M->set_word( MCEheader_CC_counter_offset, M->CC_frame_counter);
  M->set_word( MCEheader_row_len_offset,  H->get_row_len());
  M->set_word( MCEheader_num_rows_reported_offset, H->get_num_rows_reported());
  M->set_word( MCEheader_data_rate_offset, H->get_data_rate());  // test with fixed average
  M->set_word( MCEheader_CC_ARZ_counter, smurfsamples); 
  M->set_word( MCEheader_version_offset,  MCEheader_version); // can be in constructor
  M->set_word( MCEheader_num_rows_offset, H->get_num_rows()); 
  M->set_word( MCEheader_syncbox_offset, H->get_syncword());

  // data munging for MCE format
  for(j = 0;j < smurfsamples; j++)
    {
      average_mce_samples[j] = (average_samples[j] & 0x1FFFFFF) << 7;
    }


  if(C->data_frames) D->write_file(H->header, smurfheaderlength, average_samples, smurfsamples, C->data_frames, 
				   C->data_file_name, C->file_name_extend, H->disable_file_write());

  tcpbuf = S->get_buffer_pointer();  // returns location to put data (8 bytes beyond tcp start)
  memcpy(tcpbuf, M->mce_header, MCEheaderlength * sizeof(MCE_t));  // copy over MCE header to output buffer
  memcpy(tcpbuf+ MCEheaderlength * sizeof(MCE_t), average_mce_samples, smurfsamples * sizeof(avgdata_t)); //copy data 
  tcp_buflen =  MCEheaderlength * sizeof(MCE_t) + smurfsamples * sizeof(avgdata_t);
  bufx = (uint32_t*) tcpbuf;  // map buffer to 32 bit
  
  checksum  = bufx[0];		  
  for (j = 1; j < MCE_frame_length-1; j++) checksum = checksum ^ bufx[j]; // calculate checksu m
 
  memcpy(tcpbuf+ MCEheaderlength * sizeof(MCE_t) + smurfsamples * sizeof(avgdata_t), &checksum, sizeof(MCE_t)); 
   if (!(internal_counter++ % slow_divider))
     {
       C->read_config_file();  // checks for config changes
       printf("avg=%3u, sync=%6u, maxds = %2u, minds =  %2u, syncerr = %5u\n", cnt,H->get_syncword(), V->Syncbox->mindelta, 
	      V->Syncbox->maxdelta, V->Syncbox->error_count);
       printf("clr_avg= %d, dsabl_strm=%d, dsabl_file=%d\n", H->get_clear_bit(), H->disable_stream(),
       	      H->disable_file_write());
       for(uint nx = 0; nx < 4; nx++) printf("%6d ", average_samples[nx]);  // diagnostic printout
       printf("\n");
       S->connect_link(H->disable_stream()); // attempts to re-connect if not connected
       V->reset();
     }
   if (!H->disable_stream())
     {
       S->write_data(MCEheaderlength * sizeof(MCE_t) + smurfsamples * sizeof(avgdata_t) + sizeof(MCE_t));
     }
}





void Smurf2MCE::read_mask(char *filename)  // ugly, hard coded file name. fire the programmer
{
  FILE *fp;
  int ret;
  uint j = 0;
  uint x = 0; 
  uint m;

  fp = fopen("mask.txt", "r");
  if(fp==0)
    {
      printf("unable to open mask file \n");
      return;
    }
    for(j =0; j < smurfsamples; j++)
      {
	ret = fscanf(fp,"%u", &m);  // read next line 
	if ((ret == EOF) || (ret == 0)) break;  // done
	mask[j] = (m < smurf_raw_samples) ? m : 0; 
      } 
    fclose(fp);
}


Smurf2MCE::~Smurf2MCE() // destructor
{
  if(S) delete S; 
}



SmurfHeader::SmurfHeader()
{
 
  last_frame_count = 0; 
  first_cycle = 1; 
  average_counter = 0;  // number of frames avearaged so far
  data_ok = true;  // start of assuming data is OK, invalidate later.
  average_ok = true; 
  last_ext_counter = 0;  // this tracks the counter rolling over
  last_syncword = 0; 
  delta_syncword = 0;
  bigtimems=0; 
  lastbigtime = 0; 
  unix_dtime = 0;
}

void SmurfHeader::copy_header(uint8_t *buffer)
{
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

uint SmurfHeader::get_1hz_counter(void)
{
  return(pull_bit_field(header, h_1hz_counter_offset, h_1hz_counter_width));
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

uint SmurfHeader::get_epics_seconds(void)
{
  uint64_t x;
  x = pull_bit_field(header, h_epics_s_offset, h_epics_s_width); 
  return(x & 0xFFFFFFFF);  // pull out the counter. 
}

uint SmurfHeader::get_epics_nanoseconds(void)
{
  uint64_t x;
  x = pull_bit_field(header, h_epics_ns_offset, h_epics_ns_width); 
  return(x & 0xFFFFFFFF);  // pull out the counter. 
}

uint SmurfHeader::get_clear_bit(void)
{
  uint64_t x;
  x = pull_bit_field(header,  h_user0a_ctrl_offset, h_user0a_ctrl_width);
  return((x & (1 << h_ctrl_bit_clear))? 1 : 0);
}

uint SmurfHeader::disable_file_write(void)
{
  uint64_t x;
  x = pull_bit_field(header,  h_user0a_ctrl_offset, h_user0a_ctrl_width);
  return((x & (1 << h_ctrl_bit_disable_file))? 1 : 0);
}

uint SmurfHeader::disable_stream(void) //
{
  uint64_t x;
  x = pull_bit_field(header,  h_user0a_ctrl_offset, h_user0a_ctrl_width);
  return((x & (1 <<  h_ctrl_bit_disable_stream))? 1 :0);
}



uint SmurfHeader::get_num_rows(void)
{
  uint x;
  x = pull_bit_field(header, h_num_rows_offset, h_num_rows_width);
  return(x ? x: MCEheader_num_rows_value);  // if zero go to default
}

uint SmurfHeader::get_num_rows_reported(void)
{
  uint x;
  x = pull_bit_field(header, h_num_rows_reported_offset, h_num_rows_reported_width);
  return(x ? x: MCEheader_num_rows_reported_value);

}

uint SmurfHeader::get_row_len(void)
{
  uint x;
  x = pull_bit_field(header, h_row_len_offset, h_row_len_width);
  return(x ? x: MCEheader_row_len_value);
}

uint SmurfHeader::get_data_rate(void)
{
  uint x;
  x = pull_bit_field(header, h_data_rate_offset, h_data_rate_width);
  return(x ? x: MCEheader_data_rate_value);
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
      printf("jump, last = %x,  ct = %x \n", last_frame_count, x);
    }
  last_frame_count = x;
  if (!ok) data_ok = false; // invalidate data. 
  return(ok); 
}


void SmurfHeader::clear_average(void)
{
  average_counter==0;
}

uint SmurfHeader::average_control(int num_averages) // returns num averages when avearaging is done. 
{
  uint x=0, y; 
  timeb tm;  // wil lhold time imnformation
  ftime(&tm); // get time
  bigtimems = tm.millitm  + 1000 * tm.time; // make64 bit milliecond clock
  unix_dtime = bigtimems - lastbigtime;
  lastbigtime = bigtimems; 


  y = get_syncword();
  delta_syncword = y - last_syncword;
  last_syncword = y; 
  if (average_counter ==0) average_ok = data_ok;  // reset avearge ok bit.
  average_counter++; // increment number of frames averaged. 
  if (num_averages)
    {
      if (average_counter == num_averages) 
	{
	  average_counter = 0; 
	  return (num_averages); // end averaging with local control
	}
    }
  else{
    y = get_ext_counter();
    if (delta_syncword)  // 
      {
	x = average_counter; // number of averages
	average_counter = 0; // reset average
      }
    last_ext_counter = y; // copy over counter
    return(x);  // return, 0 to kep averaging, otehr to zero average. 
  }
  return(0);
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
  memset(receiver_ip, NULL, 20); // clear the IP string
  strcpy(filename, "smurf2mce.cfg");  // kludge for now. 
  num_averages = 0; // default value
  data_frames = 0;
  strcpy(receiver_ip, "127.0.0.1"); // default
  strcpy(port_number, "3333");  // temporary for now
  strcpy(data_file_name, "data"); 
  file_name_extend = 1;  // default is to append time 
  filter_order = 0; // default for block average
  for(uint j =0; j < 16; j++)
    {
      filter_a[j] = 0;
      filter_b[j] = 0;
    }
  filter_a[0] = 1;  
  filter_b[0] = 1; 
  ready = read_config_file();  
}


bool SmurfConfig::read_config_file(void)
{
  FILE *fp;
  int n, r; 
  char variable[100];
  char value[100];
  int tmp;
  double tmpd;
  char *endptr; // used but discarded in conversion
  if(!( fp = fopen(filename,"r"))) return(false); // open config file
  do{
    n = fscanf(fp, "%s", variable);  // read into buffer
    if(n != 1) continue; // eof or lost here
    n = fscanf(fp, "%s", value);  // read into buffer
    if(n != 1) continue; // probably lost if we got here
    if(!strcmp(variable, "num_averages"))
      {
	tmp = strtol(value, &endptr, 10);  // base 10, doh
	if (num_averages != tmp)
	  {
	    printf("num averages updated from %d to %d\n", num_averages, tmp);
	    num_averages = tmp;
	  }
	continue;
      }
    if(!strcmp(variable, "data_frames"))
      {
	tmp = strtol(value, &endptr, 10);  // base 10, doh
	if (data_frames != tmp)
	  {
	    printf("data_frames updated from %d to %d\n", data_frames, tmp);
	    data_frames = tmp;
	  }
	continue;
      }
    if(!strcmp(variable, "receiver_ip"))
      {
	if(strcmp(value, receiver_ip)) // update if different 
	  { 
	    printf("updated ip from %s,  to %s \n", receiver_ip, value);
	    strncpy(receiver_ip, value, 20); // copy into IP string
	  }
	continue;
      }
    if(!strcmp(variable, "port_number"))
      {
	if(strcmp(value, port_number)) // update if different 
	  { 
	    printf("updated port number from %s,  to %s \n", port_number, value);
	    strncpy(port_number, value, 8); // copy into IP string
	  }
	continue;
      }
    if(!strcmp(variable, "data_file_name"))
      {
	if(strcmp(value, data_file_name)) // update if different 
	  { 
	    printf("updated data file name from  %s,  to %s \n", data_file_name, value);
	    strncpy(data_file_name, value, 100); // copy into IP string
	  }
	continue;
      }
  
    if(!strcmp(variable, "file_name_extend"))
      {
	tmp = strtol(value, &endptr, 10);  // base 10, doh
	if (file_name_extend != tmp)
	  {
	    if(tmp) printf("adding time to file name");
	    else  printf("not adding time to file name");
	    file_name_extend = tmp;
	  }
	continue;
      }
    if(!strcmp(variable, "filter_order"))
      {
	tmp = strtol(value, &endptr, 10);  // base 10, doh
	if (filter_order != tmp)
	  {
	    printf("updated filter order from %d to %d\n", filter_order, tmp);
	    filter_order = tmp;
	  }
	continue;
      }
    for (uint n = 0; n < 16;  n++)
      {
	char tmpa[100]; // holds string
	char tmpb[100];
	sprintf(tmpa, "filter_a%d",n); 
	sprintf(tmpb, "filter_b%d",n);
	if(!strcmp(variable, tmpa))
	  {
	    tmpd = strtod(value, &endptr);  // conver to float
	    if (filter_a[n] != tmpd)
	      {
		printf("filter_a%d updated from %f to %f\n", n, filter_a[n], tmpd);
		filter_a[n] = tmpd;
	      }
	    break;
	  }
	if(!strcmp(variable, tmpb))
	  {
	    tmpd = strtod(value, &endptr);  // conver to float
	    if (filter_b[n] != tmpd)
	      {
		printf("filter_b%d updated from %f to %f\n", n, filter_b[n], tmpd);
		filter_b[n] = tmpd;
	      }
	    break;
	  }

      }
  }while ((n!=0) && (n != EOF));  // end when n ==0, end of file
  fclose(fp); // done with file
}


SmurfDataFile::SmurfDataFile(void)
{
  filename = (char*) malloc(1024 * sizeof(char)); // too big for 
  memset(filename, 0, 1024); // zero for now
  frame_counter = 0; // frames written
  header_length = smurfheaderlength; // from header file for now
  sample_points = smurfsamples;  // from header file (ugly)
  frame = (uint8_t*) malloc(60000); // just a big number for now
  fd = 0; // shows that we don't have a pointer yet
}

uint SmurfDataFile::write_file(uint8_t *header, uint header_bytes, avgdata_t *data, uint data_words, uint frames_to_write, char *fname, int name_mode,  bool disable)
{
  time_t tx;
  char tmp[100]; // for strings
  if(disable)
    {
      if(fd)  // close file
	{
	  close(fd);
	  fd = 0; 
	}
      frame_counter = 0;
       return(frame_counter); 
    }
  if(!fd) // need to open a file
    {
      memset(filename, 0, 1024); // zero for now
      strcat(filename, fname); // add file name
      if (name_mode)
	{
	  tx = time(NULL);
	  sprintf(tmp, "_%u.dat", (long)tx);  // LAZY - need to use a real time converter.  
	  strcat(filename, tmp);
	}
      else strcat(filename, ".dat");  // just use base name

      printf("new filename = %s \n", filename); 
     
      // if (!(fd = open(filename, O_WRONLY | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR))) // testing non blocking
      if (!(fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, S_IRUSR | S_IWUSR))) // testing non blocking
	{
	  printf("coult not open: %s \n", filename);
	  return(0); // failed to open file
	}
      else printf("opened file %s  fd = %d \n", filename, fd); 
    }
  memcpy(frame, header, header_bytes);
  memcpy(frame + h_num_channels_offset, &data_words, 4); // UGLY horrible kludge, need to fix.
  memcpy(frame+header_bytes, data, data_words * sizeof(avgdata_t)); 
  write(fd, frame, header_bytes + data_words * sizeof(avgdata_t));
  frame_counter++;
  if(frame_counter >= frames_to_write)
    {
      if(fd) close(fd);
      fd = 0;
      frame_counter = 0;
    }
  return(frame_counter);
}


void Smurf2MCE::acceptFrame ( ris::FramePtr frame ) 
{
  uint totread = 0;
  uint32_t nbytes = frame->getPayload();
  //printf("accept frame called \n");
  if (frame->getError() || (frame->getFlags() & 0x100))  // drop frame  (do we  need to read out buffer?)
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
  int tmp, j; 
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
    totread += size;
    } 
  uint target_size = smurfheaderlength * sizeof(char) + smurf_raw_samples * sizeof(smurf_t); 
  //printf("target size = %u  nbytes = %u, totread = %u \n", target_size, nbytes, totread);
  if ((target_size != nbytes) || (target_size !=totread)) printf("wrong size frame from Pyrogue \n");
  process_frame();
}

SmurfTime::SmurfTime(void) 
{
  current = 0;
  delta = 0;
  error_count = 0;
}

void SmurfTime::update(uint64_t val)
{
  delta = (val >= current) ? val-current : 0;
  current = val; 
  mindelta = (delta > mindelta) ? mindelta : delta; // collect min and max
  maxdelta = (delta < maxdelta) ? maxdelta : delta; 
  if (delta > max_allowed_delta) error_count++;
}


SmurfValidCheck::SmurfValidCheck() // just creates  all variables. 
{
  Unix_time = new SmurfTime();
  Syncbox = new SmurfTime();
  Timingsystem = new SmurfTime();
  Counter_1hz = new SmurfTime();
  Smurf_frame = new SmurfTime();
  init = false; 
  missed_syncbox = 0; 
  Unix_time->max_allowed_delta  = 1000000000;
  Syncbox->max_allowed_delta = 1; 
  Timingsystem->max_allowed_delta = 1000000000;
  Counter_1hz->max_allowed_delta = 1000000000;
  Smurf_frame->max_allowed_delta = 1; 

}




void SmurfValidCheck::run(SmurfHeader *H)
{
  timespec tmp_t;  // structure seconds, nanoseconds
  uint64_t tmp; 
  clock_gettime(CLOCK_REALTIME, &tmp_t);  // get time s, ns,  might be expensive
  tmp = 1000000000l * (uint64_t) tmp_t.tv_sec + (uint64_t) tmp_t.tv_nsec;  // brute force multiply to 64 vit
  Unix_time->update(tmp); 
  Syncbox->update(H->get_syncword());
  tmp = 1000000000l * (uint64_t) H->get_epics_seconds() + (uint64_t) H->get_epics_nanoseconds();
  Timingsystem->update(tmp);
  Counter_1hz->update(H->get_1hz_counter());
  Smurf_frame->update(H->get_frame_counter());
}


void SmurfValidCheck::reset()
{
  Unix_time->reset();
  Syncbox->reset();
  Timingsystem->reset();
  Counter_1hz->reset();
  Smurf_frame->reset();
}



SmurfFilter::SmurfFilter(uint num_samples, uint num_records)
{
  records = num_records;
  samples = num_samples;
  clear = false;
  xd = (double*)malloc(samples * records * sizeof(double));  // don't bother checking valid, only at startup, fix later
  yd =  (double*)malloc(samples * records * sizeof(double));
  output = (avgdata_t*) malloc(samples * sizeof(avgdata_t));
  bn = 0;
  clear_filter();
}


void SmurfFilter::clear_filter(void)
{
  memset(xd, 0, records * samples * sizeof(double));
  memset(yd, 0, records * samples * sizeof(double));
  samples_since_clear = 0;  // reset
  order_n = -1;
  bn = 0;  // ring buffer pointers back to zero
}

void SmurfFilter::end_run()
{
  if(order_n == -1)
    {
      clear_filter();
    }
}

 avgdata_t *SmurfFilter::filter(avgdata_t *data, int order, double *a, double *b)
{
  if(order_n != order){
    clear_filter();
  }
  order_n = order;  // used to clear when using the flat average filter
  samples_since_clear++;  
  if (order == -1) // special case flat average filter
    {
      for(uint n = 0; n <  samples;  n++)
	{
	  *(yd + bn * samples + n) += (double) (*(data+n)); // just sum into first record.
	  *(output+n) = (avgdata_t) (*(yd+bn*samples + n) / (double) samples_since_clear);  // convert 
	}
    }  
  else
    {
      bn = (bn + 1) % records;  // increment ring buffer pointer
      for (uint n = 0; n < samples; n++)
	{
	  *(xd + bn * samples + n) = (double) (*(data+n));  // convert to doubles
	}
      memset(yd + bn * samples, 0, samples * sizeof(double)); // clear new y data
      for (uint n = 0; n < samples; n++) // loop over channels for filter  
	{
	  *(yd + bn * samples + n) = b[0] * *(xd + bn * samples + n);
	  for(int r = 1; r <= order; r++) // one more record than order. (eg order = 0 is record)
	    {
	      int nx = (bn - r) % records;  // should give the correct buffer reference 
	      *(yd + bn * samples + n) += b[r] * *(xd + nx * samples + n) - a[r] * *(yd + nx * samples + n);
	    }
	  *(yd +bn * samples + n) = *(yd+bn * samples +n) / a[0];  // divide final answer
	  *(output+n) = (avgdata_t) *(yd+bn*samples + n); 
	}
    }
  return(output); 
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



