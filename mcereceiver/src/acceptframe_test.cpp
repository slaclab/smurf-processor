#include "smurftcp.h"

//        // modify later to deal with errors

void Smurf2MCE::acceptframe_test(char *data, size_t size) // size not currently used. 
{
  int tmp, j;
  buffer = b[bufn]; // buffer swap 
  bufn = bufn ? 0 : 1; // swap buffer reference
  buffer_last = b[bufn]; // now that we've swapped them
  memcpy(buffer, data, size);  // just simple for now. (replace example code)
  process_frame();
} 
