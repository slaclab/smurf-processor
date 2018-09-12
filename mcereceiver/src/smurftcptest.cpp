// Test program for smurf link
#include "smurftcp.h" 



int  main()  // just for testing things, not for deployment. 
{
  smurf_t smurf_data[smurf_raw_samples]; // the raw 16 bit integer data 
  char smurf_header[smurfheaderlength]; // holds the SMuRF header
  char output_buffer[pyrogue_buffer_length]; 
  char *b; 
  uint j, k; 
  const char* tmp = server_port_number;
  uint counter = 0;  //just counts  up to generate fake data
  char testmask[smurf_raw_samples]; // data mask. 
  Smurf2MCE *M;
  SmurfHeader *H;

  int rate = 2000; // repetition rate
  int cycles = 200; // number of cycles
  int report_ratio = 800; 
  int averages = 8;
  bool runforever = true; 

  M = new Smurf2MCE (server_port_number, server_ip_addr); // create the smurf MCE class.
  H = new SmurfHeader(); // generate smurf header

  struct timespec timer; 
  timer.tv_sec = 0;
  timer.tv_nsec = 1000000000/rate;  // nanoseconds delay for testing looop

  
  memset(smurf_data, 0xAA, smurf_raw_samples * sizeof(smurf_t)); // should never see this, masked otu
  memset(testmask, 1,smurf_raw_samples * sizeof(char));  // set mask
  memset(testmask, 0, 10 * sizeof(char)); // clear mask for first 10 points
  M->set_mask(testmask, smurf_raw_samples); // mask off first 10 points
  

  counter = 0;
  for(k = 0; (k < cycles) || runforever; k++) // main loop, can be set for endless run
    { 
      nanosleep(&timer, NULL); // sets loop speed
      // setting up data
      for (j = 0; j < 10; j++) smurf_data[j] =0xFFFF;  // test, shoudl be masked. 
      smurf_data[10] = 1; // testing
      smurf_data[11] = k; // different test
      smurf_data[12] = k * 1000;  // warp test
      for(j = 13; j < smurf_raw_samples+10; j++) smurf_data[j] =  j-10; 
      // done setting up data
      
      if(k % averages) H->clear_average_bit(0);  // set averaging
      else H->set_average_bit(0);
      memcpy(output_buffer, H->header, smurfheaderlength * sizeof(char));
      memcpy(output_buffer + smurfheaderlength * sizeof(char), smurf_data, smurf_raw_samples * sizeof(smurf_t));
      M->acceptframe_test(output_buffer, smurfdatalength); 
      if (!(k % report_ratio))printf("cycle = %d\n", k); 
    }						    
  printf("generated  %d smurf frames\n", k);

  delete M; 

}
