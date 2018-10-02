
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
  SmurfDataFile *D; // outptut file for saving smurf data. 

  Smurf2MCE();
  void acceptFrame(ris::FramePtr frame);

  //void acceptframe_test(char* data, size_t size); // test version for local use, just a wrapper
  void process_frame(void); // does average, separates header
  void set_mask(char *new_mask, uint num); // set array mask
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

  C = new SmurfConfig(); // will hold config info - testing for now
  //S = new Smurftcp(port, ip);
  S = new Smurftcp(C->port_number, C->receiver_ip);
  M = new MCEHeader();  // creates a MCE header class
  H = new SmurfHeader(); 
  D = new SmurfDataFile();  // holds output data
 
  average_counter = 0; // counter used for test averaging , not  needed in real program
  for(j = 0; j < 2; j++)
    {  // allocate 2 buffers, so we can swap up/ back for background subtraction.
      if(!(b[j] = (uint8_t*)malloc(pyrogue_buffer_length)))  
	{
	  error("could not allocate smurf2mce buffer");
	  return;
	}
      //printf("buffer = %x, len = %d \n", b[j], pyrogue_buffer_length);80
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
  read_mask(NULL);  // will use real file name later
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
 
  if (!(cnt = H->average_control(C->num_averages))) return;  // just average, otherwise send frame
  
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


  if(C->data_frames) // file writing turned on
    {
      D->write_file(H->header, smurfheaderlength, average_samples, smurfsamples, C->data_frames, C->data_file_name);
    }

  tcpbuf = S->get_buffer_pointer();  // returns location to put data (8 bytes beyond tcp start)
  memcpy(tcpbuf, M->mce_header, MCEheaderlength * sizeof(MCE_t));  // copy over MCE header to output buffer
  memcpy(tcpbuf+ MCEheaderlength * sizeof(MCE_t), average_samples, smurfsamples * sizeof(avgdata_t)); //copy data 
  tcp_buflen =  MCEheaderlength * sizeof(MCE_t) + smurfsamples * sizeof(avgdata_t);
  bufx = (uint32_t*) tcpbuf;  // map buffer to 32 bit
  checksum  = bufx[0];

  for (j = 1; j < MCE_frame_length-1; j++) checksum =checksum ^ bufx[j]; // calculate checksum
 
 
  memcpy(tcpbuf+ MCEheaderlength * sizeof(MCE_t) + smurfsamples * sizeof(avgdata_t), &checksum, sizeof(MCE_t)); 
   if (!(internal_counter++ % slow_divider))
     {
       C->read_config_file();  // checks for config changes
       printf( "avg= %3u, sync = %6u, intctr = %6u, frmctr = %6u\n", cnt, H->get_syncword(),internal_counter,
	     H->get_frame_counter());

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
	if(m < smurf_raw_samples){
	  mask[m] = 1; // set mask.
	  x++;
	}
      } 
    fclose(fp);
    printf("set mask for %d \n", x); 
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


uint SmurfHeader::average_control(int num_averages) // returns num averages when avearaging is done. 
{
  uint x=0, y; 
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
    if (last_ext_counter > y)  // TEST TEST TEST - until we have averaging bits
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
  ready = read_config_file();  
}


bool SmurfConfig::read_config_file(void)
{
  FILE *fp;
  int n, r; 
  char variable[100];
  char value[100];
  int tmp;
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

uint SmurfDataFile::write_file(uint8_t *header, uint header_bytes, avgdata_t *data, uint data_words, uint frames_to_write, char *fname)
{
  time_t tx;
  char tmp[100]; // for strings
  if(!fd) // need to open a file
    {
      tx = time(NULL);
      memset(filename, 0, 1024); // zero for now
      strcat(filename, fname); // add file name
      sprintf(tmp, "_%u.dat", (long)tx);  // LAZY - need to use a real time converter.  
      strcat(filename, tmp);
      printf("new filename = %s \n", filename); 
      if (!(fd = open(filename, O_WRONLY | O_CREAT)))
	{
	  printf("coult not open: %s \n", filename);
	  return(0); // failed to open file
	}
    }
  memcpy(frame, header, header_bytes);
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



