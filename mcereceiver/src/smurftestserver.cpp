// SMuRF TCP test server

#include "../../common/smurf2mce.h"

void error(const char *msg){ perror(msg);};          // modify later to deal with errors


class Smurftestserver
{
public:
  char *tcpbuffer; // raw buffer of tcp data 
  bool inframe; // are we currently in the middle of a frame?

  char *data_frames[numframes]; //  holds set of reorded MCE data frames
  uint data_frame_n; // which frame we are on now
  uint frame_n;  // location in current frame
  char *output_ptr[numframes]; // pointers to output frames (returned to main program)
  uint start_frame;  // after read, which frame do we start with as we cycle through frames
  uint num_finished_frames; // number of completed frames

  bool initialized; // memory allocated
  bool connected;  // client connected
  int sockfd, fd;  // socket file descriptors. Need 2nd socket after connection to client 
  const char *portnum;   // which port to use - string (due to goofy linux function)
 
  struct addrinfo *server;  // will hold server address structure

  Smurftestserver(const char *port_number, const char *ip_string);  // constructor
  uint read_data(void);  // reads data 
  bool connect_tcp(); // makes tcp connection
  void disconnect_tcp(void); // close socket connection
  ~Smurftestserver(); // destructor, probably not needed 
};

class Smurfpipe // writes to named pipe
{
public:
  int fifo_fd;  // file pointer to pipe
  Smurfpipe();
  ~Smurfpipe();
  int write_pipe(MCE_t* data, int points); 
};


Smurftestserver::Smurftestserver(const char *port_number = "5433", const char *ip_string = "134.79.228.97")
{
  portnum = port_number;
  initialized = false;
  connected = false; 
  inframe = false; // are we in the middle of a data frame. 
  uint j;
  if (!(tcpbuffer = (char*)malloc(tcpreclen))) // tcp receive buffer longer to allow multiple frames if we get behind
    {
      error("cant allocate memory");
      return;
    }
  for(j = 0; j <  numframes; j++)
    {
      if(!(data_frames[j] = (char*)malloc(tcplen)))
	{
	  error("could not allocate data frame\n");
          return; 
	}
    }
  data_frame_n = 0; // first frame
  frame_n = 0;  // pointer at start of first frame
  if(0 > (sockfd = socket(AF_INET, SOCK_STREAM, 0)))  { error("can't open socket"); return;}  // opens socket
  if (getaddrinfo(NULL, portnum, NULL, &server)){ error("error trying to resolve address or port"); return; }
  // was NULL rather than 127.0.0.1 , need to understand this

  printf("sockfd %d , ai_addr %x, ai_addrlen %x\n", sockfd, server->ai_addr, server->ai_addrlen);
  if (bind(sockfd, server->ai_addr, server->ai_addrlen)){ error(" error binding socket"); return; }
  initialized = true; 
  connected = connect_tcp(); // calls connection. 
}

bool Smurftestserver::connect_tcp()
{
  if(!initialized){ printf("not initialized");  return(false);}; // not initialized
  printf("connect tcp \n");
  listen(sockfd, 5); // listen for connections
  fd = accept(sockfd, NULL, NULL); // not recording client info
  printf("sockets sockfd = %d ,  fd = %d \n", sockfd, fd);
  connected = true; 
  return(true); // connected 
}


void Smurftestserver::disconnect_tcp()
{
  if(!initialized){connected = 0;  return;}; // not initilaized to beging with, just return
  printf("disconnect tcp\n");
  close(fd); 
  connected = false;  
}


Smurftestserver::~Smurftestserver()
{
  int j; 
  if (initialized)
  {
    close(fd);
    close(sockfd);
    if (tcpbuffer) free(tcpbuffer);
    for( j = 0; j < numframes; j++) {if(data_frames[j]) free(data_frames[j]);}
  }
}

uint Smurftestserver::read_data(void)
{
  uint num_finished_frames; 
  uint last_read; 
  int j=0, k=0, tst;;
  uint32_t *test, *test2; 
  int framex = 0; 

  fd_set rfds;    // for select() command 
  struct timeval tv;
  int isst=0;  // used by select()
  
  tv.tv_sec = 0;
  tv.tv_usec = 10000; //select timeout, 10msec (should be enough). 
  
  num_finished_frames = 0;
  start_frame = data_frame_n; 
  do{
    FD_ZERO(&rfds); // clear registers used for select. 
    FD_SET(fd, &rfds); 
    if (-1 == select(fd+1, &rfds, NULL, NULL, &tv)) {error("select error" ); return(0);} // can we read ?
    isst= FD_ISSET(fd,&rfds);
    if (!isst) return(0);                // can we read from this file descriptor
    last_read = read(fd, tcpbuffer, tcplen); // READ DATA
    if(last_read == -1) { error("read error");	return(0);}
    if(last_read == 0) 
      {
	printf("zero length read, socket closed"); 
	disconnect_tcp();
	connect_tcp(); // wait for new connection
	return(0); 
      }
    if (last_read & 0x01) printf("odd nubmer of bytes read - need to deal with this");
    for (k = 0; k< last_read; k+=2) // byte in frame
      {
	if( tcpbuffer[k] & 0x80) // found a marker
	  {
	    if(inframe)// we are in the middle of a frame
	      {
		if (frame_n != datalen)
		  {
		    printf("frame size error %u , tcplen %u\n", frame_n, tcplen);  // data bytes
		  }else
		  {
		    num_finished_frames++;  // increment which frame we are writing
		    data_frame_n = (data_frame_n + 1) % numframes;   // increment or wrap frame pointer
		  }
		frame_n = 0; // frame counter to zero
	      }
	    else
	      {
		inframe = true; 
	      }
	  }

	*(data_frames[data_frame_n]+frame_n++) = tcpbuffer[k] & 0x0F | ((tcpbuffer[k+1] & 0x0F)<<4);  // combine bytes
	if(frame_n == tcplen) // new frame
	  {
	    num_finished_frames++; 
	    frame_n = 0;  // reset data pointer in frame
	    data_frame_n = (data_frame_n + 1) % numframes;  // select new frame
	    inframe = false;  // no longer in a frame
	  }  
      }
  }while(num_finished_frames ==0); // keep trying until we get a frame
  for(j = 0; (j < num_finished_frames) && (j < numframes); j++)
    {
      framex= (start_frame + j)% numframes; 
      test = (uint32_t*) data_frames[framex];
      test2 = (uint32_t*) (data_frames[framex] + 4); 
      if (*test != header) printf("got wrong header\n");
      if (*test2 != tcplen) printf("got wrong data length\n"); 
      output_ptr[j] = (data_frames[framex ] + tcp_header_size);  // genreate pointers.
      if( MCE_header_version != ((MCE_t) *(output_ptr[j] + mce_h_offset_header_version * sizeof(MCE_t))))
	{
	  error("wrong header detected");
	}
    }
  return(num_finished_frames); 
}


Smurfpipe::Smurfpipe()
{
  if(-1 == (fifo_fd = open(pipe_name, O_WRONLY)))
    {
      error("unable to open pipe \n");
    }
  printf("fiifo_fd = %d \n", fifo_fd);
}


Smurfpipe::~Smurfpipe()
{
  close(fifo_fd); 
}


int Smurfpipe:: write_pipe(MCE_t* data, int points)
{
  int n; 
  n =  write(fifo_fd, data, points * sizeof(MCE_t));
}

int main()
{
  int j, k, m, r, x; 
  uint recframes = 0;
  MCE_t header[numframes][MCEheaderlength];
  MCE_t data[numframes][smurfsamples]; 
  MCE_t *tmp;
  uint number_to_record = 50; // number output frames to record. 
  int report_ratio = 100;
  bool runforever = true;  // keep running
  uint channel_to_record = 1; // which channel to record
  MCE_t record_data[number_to_record];  // holds data for single channel
  uint num_received[number_to_record];  
  uint max_rec_frames = 0;
  Smurftestserver *S; // create S.
  Smurfpipe *P; // create pipe
  S = new Smurftestserver(server_port_number, server_ip_addr);
  P = new Smurfpipe();
  for (j = 0; (j< number_to_record) || runforever; )
  { 
    recframes = S->read_data();
    for(k = 0; k < recframes; k++)
      {
	tmp = (uint32_t*)(S->output_ptr[k]+sizeof(MCE_t)*(k+MCEheaderlength));
	for(m = 0; m < smurfsamples; m++)
	{
	  data[k][m] = *(tmp+m) -average_sample_offset;
	}
        tmp = (MCE_t*) S->output_ptr[0] +  mce_h_offset_header_version;
      } 
    if (recframes == 0) continue; // no frames receive this time
    if(!(j%report_ratio)) printf("frame = %d \n", j);
    P->write_pipe((MCE_t*) S->output_ptr[recframes-1], MCE_frame_length);   // now write latest frame
    j++;
  }    
  printf("done receiving \n");
}
