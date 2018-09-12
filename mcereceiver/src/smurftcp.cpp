// SMuRF TCP client

#include <smurftcp.h>  // class definitions

void error(const char *msg){perror(msg);};    // modify later to deal with errors

Smurftcp::Smurftcp( const char *port_number = "5433",  const char *ip_string = "127.0.0.1")
{
  initialized = false;
  port = port_number;
  ip = ip_string;
  connected = false;
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
      return(false);
    }
  return(connected = true);
}

bool Smurftcp::disconnect_link(void) // clean up link
{
  if(!connected) return(false);  // already connected
  if (sockfd!= -1  ) close(sockfd);
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
      disconnect_link();
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

Smurf2MCE::Smurf2MCE(const char *port_number, const char *ip_string)
{
  initialized = false;
  port = port_number;
  ip = ip_string; 
  average_counter= 1; 
  bufn = 0; // current buffer 
  int j; 
  S = new Smurftcp(port, ip);
  M = new MCEHeader();  // creates a MCE header class
  H = new SmurfHeader(); 
  average_counter = 0; // counter used for test averaging , not  needed in real program
  for(j = 0; j < 2; j++)
    {  // allocate 2 buffers, so we can swap up/ back for background subtraction.
      if(!(b[j] = (char*)malloc(pyrogue_buffer_length)))  
	{
	  error("could not allocate smurf2mce buffer");
	  return;
	}
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
  uint j; 
  uint actr; // counter for average array;
  uint dctr; // counter for input data array
  char *tcpbuf; 
  uint32_t cnt; 
  int tmp;
  memcpy(H->header, buffer, smurfheaderlength);
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
  if (!(cnt=process_header())) return;  // just average, otherwise send frame
  for (j = 0; j < smurfsamples; j++)   // divide out number of samples
    average_samples[j] = (avgdata_t) (((double)average_samples[j])/cnt + average_sample_offset);
  tcpbuf = S->get_buffer_pointer();  // returns location to put data (8 bytes beyond tcp start)
  memcpy(tcpbuf, M->mce_header, MCEheaderlength * sizeof(MCE_t));  // copy over MCE header to output buffer
  memcpy(tcpbuf+ MCEheaderlength * sizeof(MCE_t), average_samples, smurfsamples * sizeof(avgdata_t)); //copy data
  S->write_data(MCEheaderlength * sizeof(MCE_t) + smurfsamples * sizeof(avgdata_t));
  memset(average_samples, 0, smurfsamples * sizeof(avgdata_t));
}

uint32_t Smurf2MCE::process_header(void)
{
  uint tot_averages;
  if (H->get_average_bit(0))
    {
      tot_averages = average_counter;
      average_counter = 1; 
      return(tot_averages);  
    }
  average_counter++;
    return(0);
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
  memset(header, 0, smurfheaderlength);  // clear initial falues
}

void SmurfHeader::set_average_bit(int n)
{ 
  if( n < 8) header[h_averaging_bits_offset] |= 1 << n; 
  else if (n < 16) header[h_averaging_bits_offset+1] |= 1 << (n-8);
  else ; // too big  
}

void SmurfHeader::clear_average_bit(int n)
{ 
  if( n < 8) header[h_averaging_bits_offset] &= ~(1 << n); 
  else if (n < 16) header[h_averaging_bits_offset+1] &= ~(1 << (n-8));
  else ; // too big  
}




int SmurfHeader::get_average_bit(int n)
{
  if( n < 8) return(header[h_averaging_bits_offset] & (1 << n)); 
  else if (n < 16) return(header[h_averaging_bits_offset+1] & 1 << (n-8));
  else return(0); 
}


MCEHeader::MCEHeader()
{
  memset(mce_header, 0, MCEheaderlength * sizeof(MCE_t));
  mce_header[mce_h_offset_header_version] = MCE_header_version;  // current version. 
}

