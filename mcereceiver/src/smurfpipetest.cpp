// smurfpipetest
#include "../../common/smurf2mce.h"

void error(const char *msg){ perror(msg);};  

class check_data
{
 public:
  uint last_CC_counter;

  check_data(void);
  bool test(MCE_t *data);

};

int main()
{
  int fifo_fd; // fifo  descritor
  int j, num; 
  int report_ratio = 100;
  bool runforever = true; 
  MCE_t data[MCE_frame_length]; // will hold received data
  check_data *C;
  C = new check_data();
  printf("starting SMuRF pipe test \n");
  

#if 0
  if(-1 == mkfifo(pipe_name, 0666)) // unlink, try agian
    {
      unlink(pipe_name);
    } 
#endif
  if(-1 == mkfifo(pipe_name, 0666)) 
    {
      error("error creating fifo\n");
      //exit(0);
    }
  if( -1 == (fifo_fd = open(pipe_name, O_RDONLY)))
    {
      printf("error opening fifo\n");
      unlink(pipe_name);  // delete the fifo
      exit(0);
    }
  printf("fifo_fd = %d\n", fifo_fd);
  for(j = 0; 1; j++)// loop forever
    {
      if(-1 == (num = read(fifo_fd, data, sizeof(MCE_t) * MCE_frame_length)))
	{ 
	  error("read error"); 
	  break;
	}
      if(!num) {j--; continue; }; // read 0 bytes, try again (inefficient)
      if(num != (MCE_frame_length * sizeof(MCE_t)))
	{
	  printf("frame len = %u, wanted %u\n", num, MCE_frame_length);
	}
      C->test(data); // check that daata is OK
      if(!(j % report_ratio))
	{
	  printf("int frame = %u, syncbox = %u , check = %x\n", j, data[mce_h_syncbox_offset] & 0xFFFFFFFF, data[MCE_frame_length-1]);
	}
    }
  close(fifo_fd);
  unlink(pipe_name);  // delete the fif o
}


check_data::check_data(void)
{
  last_CC_counter = 0; 
}

bool check_data::test(MCE_t *data)
{
  uint x;
  uint j;
  MCE_t checksum;
  x = data[MCEheader_CC_counter_offset];
  if ((x != last_CC_counter + 1))
    {
      last_CC_counter = x;
      printf("counter error %u \n", x); 
      return(false);
    }
  last_CC_counter = x;
  checksum = data[0];
  for(j = 1; j < MCE_frame_length; j++) checksum = checksum ^ data[j];
  if(checksum)
    {
      printf(" cerror %x dl = %x, %x,  %x\n", checksum,  data[6], data[MCE_frame_length-2], data[MCE_frame_length-1]);
      return(false);
    }
  
  return(true); 
}
