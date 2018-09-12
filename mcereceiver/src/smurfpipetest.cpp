// smurfpipetest
#include "smurf2mce.h"

void error(const char *msg){ perror(msg);};  

int check_data(MCE_t  *data); // for testing, checks various data validity.


int main()
{
  int fifo_fd; // fifo descritor
  int j, num; 
  int num_frames = 100;
  int report_ratio = 100;
  bool runforever = true; 
  MCE_t data[MCE_frame_length]; // will hold received data
  printf("starting SMuRF pipe test \n");

  if(-1 == mkfifo(pipe_name, 0666)) // unlink, try agian
    {
      unlink(pipe_name);
    } 

  if(-1 == mkfifo(pipe_name, 0666)) 
    {
      error("error creating fifo\n");
      exit(0);
    }
  if( -1 == (fifo_fd = open(pipe_name, O_RDONLY)))
    {
      printf("error opening fifo\n");
      unlink(pipe_name);  // delete the fifo
      exit(0);
    }
  printf("fifo_fd = %d\n", fifo_fd); 
  for(j = 0; (j < num_frames) || runforever; j++)
    {
      if(-1 == (num = read(fifo_fd, &data, sizeof(MCE_t) * MCE_frame_length)))
	{ 
	  error("read error"); 
	  break;
	}
      if(!num) {j--; continue; };
      if(!(j % report_ratio)) printf("frame = %d \n", j); 
    }
  close(fifo_fd);
  unlink(pipe_name);  // delete the fif o
}


int check_data(MCE_t *data)
{
  return(0);
}
