// SMuRF TCP test server

#include "../../common/smurf2mce.h"
#include <zmq.hpp>
#include <unistd.h>
#include "Queue.h"
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>

void error(const char *msg){ perror(msg);};          // modify late r to deal with errors

typedef boost::shared_ptr<zmq::message_t> MessagePtr;

class SmurfReceiver
{

      static const unsigned queueDepth = 200;  //1 second of buffering

      unsigned fullCount_;

      zmq::context_t context_;

      zmq::socket_t socket_;

      Queue<MessagePtr> messageQueue_; 

      boost::thread* thread_;

      void runThread();

   public:

      SmurfReceiver(const char* portnum);

      ~SmurfReceiver();

      MessagePtr receive(); 

      unsigned getFullCount();

};

SmurfReceiver::SmurfReceiver(const char* portnum) : fullCount_(0), context_(1), socket_(context_, ZMQ_PULL)
{

   printf("binding to %s\n", portnum);

   socket_.bind(portnum);

   messageQueue_.setThold(queueDepth);

   thread_ = new boost::thread(boost::bind(&SmurfReceiver::runThread, this));

};

void SmurfReceiver::runThread()
{

   printf("\n");

   printf("Starting SmurfReceiver::runThread()\n");

   printf("\n");

   try {

      while(1) {

         MessagePtr m = boost::make_shared<zmq::message_t>(MCE_frame_length * sizeof(MCE_t));

         socket_.recv( m.get() );

         if ( messageQueue_.busy() ) {

            continue; // drop frame

         }

         messageQueue_.push( m );

         boost::this_thread::interruption_point();

      }

   } catch(boost::thread_interrupted&) {}

}

MessagePtr SmurfReceiver::receive()
{

   return messageQueue_.pop();

}

unsigned SmurfReceiver::getFullCount()
{
   return fullCount_;
}

SmurfReceiver::~SmurfReceiver()
{

}

class Smurfpipe // writes to named pipe
{
public:
  int fifo_fd;  // file pointer to pipe
  Smurfpipe(const char* pipename);
  ~Smurfpipe();
  int write_pipe(MCE_t* data, int points); 
};

Smurfpipe::Smurfpipe(const char *pipename)
{
  signal(SIGPIPE, SIG_IGN);  // ignores broken pipe - TESTING
  printf("pipe name = [%s] \n", pipename);
  if(-1 == (fifo_fd = open(pipename, O_WRONLY))) // blocking
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
  //printf("start -");
  n =  write(fifo_fd, data, points * sizeof(MCE_t));
  //printf("done %u\n", n);
  return n;
}

int main()
{
  char portnum[100]; // enough space
  strcpy(portnum, "tcp://*:3333");  // default port
  char pipe_name[256];
  char variable[256];
  char value[256];
  int j, n, b;
  int fifo_error = 0;


  FILE *cfgfp;  // pointer to config file
  
  if((cfgfp = fopen("smurfrec.cfg", "r")))
    {
      for(j = 0; j < 2; j++) // just 2 things to read
	{
	  n = fscanf(cfgfp, "%s", variable);  // read into buffer
	  if(n != 1) continue; // eof or lost here
	  n = fscanf(cfgfp, "%s", value);  // read into buffer
	  if(n != 1) continue; // probably lost if we got here
	  if(!strcmp(variable, "port_number"))
	    { 
	    strncpy(portnum + 8, value, 92); // copy over port number
	    }
	  if(!strcmp(variable, "named_pipe"))
	    { 
	    strncpy(pipe_name, value, 256); // copy over port number
	    }
	}
    }else printf("couldn't open smurfrec.cfg file");


  SmurfReceiver R(portnum);

  MCE_t *tmp;
  uint last_syncbox, syncbox;
  uint missing_frames = 0;

  last_syncbox = syncbox = 0;
 
  Smurfpipe P(pipe_name); // create pipe
  j = 0; 
  while(1)
  { 
    MessagePtr m   = R.receive(); //blocking
    tmp = (MCE_t*) (m->data()); // convert pointer for testing output
    syncbox = *(tmp+10);
    if(j > 1000) // wait for startup
      {
	if (syncbox != (last_syncbox + 1)) missing_frames++; 
      } else {
        fifo_error = 0;
      }
    last_syncbox = syncbox;
    if(!(j%slow_divider))
      {
	printf("S1: lclfrm=%10d ,sync=%10d, data0= %6d d_shft7=%6d,  missfrm = %u\n", j,syncbox, *(tmp+43), ((int32_t)*(tmp+43))/128,  missing_frames );
	printf("S2: row_len=%4u, num_rows_reported=%3u, data_rate = %6u, num_rows=%3u\n", *(tmp+2), *(tmp+3), *(tmp+4), *(tmp+9));
	printf("S3: bytes_written=%i, fifo_error=%i, smurf_full_count=%u\n", b, fifo_error, R.getFullCount());
	printf("\n");
      }
    if ( P.write_pipe((MCE_t*) tmp, MCE_frame_length) == -1 )   // blocking
        fifo_error++;
    j++;
    
  }    
}
