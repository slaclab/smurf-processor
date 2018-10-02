#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h> 
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#ifndef __SMURF2MCE_H__
#define __SMURF2MCE_H__


// do we really need all these includes? 

//const uint smurf_raw_samples = 1024; // samples before masking. 
// NOTE: need 1 extra word in MCE data for checksum!!!

typedef int16_t smurf_t;  // raw smurf data type. (int? uint? need to know what we get)
typedef int32_t avgdata_t;  // data type for averaged data was int but needs int32 (important fix)
typedef uint32_t MCE_t;  //  data used in mce system
typedef int32_t wrap_t; // type for wrap counter, just offset for now. 

const uint slow_divider = 200; // sets divisiion ration from average to print out rate.

const uint tcpreclen = 0x80000;  // allow for multiple  reads in one frame proably big enough
const size_t pyrogue_buffer_length = 0x8000; // not sure what the maximum size could be 
const uint smurf_raw_samples = 4096; // samples before masking.  this is from the smurf to transmitter
const uint smurfsamples = 528;  // number of SMuRF samples in a frame was 528 (av
const uint smurfheaderlength =128; // number of bytes in smurf header

const uint tcp_header_size = 8; // number of bytes in tcp header for data checking

const uint MCEheaderlength = 43; // words in MCE header note words are 32 bit


//  legnth of expected data from  pyrogue
const uint smurfdatalength = smurf_raw_samples * sizeof(smurf_t) + smurfheaderlength; 

const uint MCE_frame_length = MCEheaderlength + smurfsamples+1; // number of words in MCE data. +1 Checksum

// bytes of data before unpacking to nibbles including checksum to tcp interface
const uint datalen = tcp_header_size + MCEheaderlength*sizeof(MCE_t) + smurfsamples * sizeof(MCE_t) + sizeof(MCE_t);
const uint tcplen = datalen * 2; // after byte split



const uint32_t header = 0x89ABCDEF; // header used on TCP to help test transmission
// number of frames that can be received at one time.  (shoudl only need one unless lin k is slow) 
const uint numframes = 8; 
// unwrap rules
const int upper_unwrap = 0x6000;  // if we are above this and jump, assume a wrap
const int lower_unwrap = -0x6000; // if we are below this and jump, assume a wrap
const wrap_t wrap_start = 0x0;  //starting wrap value
const uint32_t average_sample_offset= 0; // used to offset average data to avoid wrap, may not be neeeded. 


// MCE header



const int mce_h_offset_status = 0;
const int mce_h_status_value = 0x0080C10;  // MCE header word (see excel sheet) 

const uint MCEheader_CC_counter_offset = 1; // this holds a counter we use internally for mce frames

const uint MCEheader_row_len_offset = 2; // mysterious thing in MCE
const uint MCEheader_row_len_value = 256; // no idea what this should be

const uint MCEheader_num_rows_reported_offset= 3; // 33 rows, 
const uint MCEheader_num_rows_reported_value = 33; 

const uint MCEheader_data_rate_offset =4; //use number of smurf frame averages (strange name)
const uint MCEheader_CC_ARZ_counter = 5; // no idea, use 528 smurf samples for now

const uint MCEheader_version_offset = 6;  // offset to header version. 
const uint MCEheader_version = 7;  // current header version

// addr 7,8 unused. 

const uint MCEheader_num_rows_offset = 9; // different from reported rows?
const uint MCEheader_num_rows_value = 33; 

const int MCEheader_syncbox_offset = 10;  // words offset to syncbox output


// smurf header byte offsets
const int h_version_offset = 0; // offset of version word
const int h_version_width = 1; // bytes of version word
const int h_ext_counter_offset = 68;  // resets with next MCE word
const int h_ext_counter_width = 4; // width
const int h_frame_counter_offset = 84;  // raw frame counter. 
const int h_frame_counter_width = 4; 
const int h_mce_syncword_offset = 96;  // 20 bit MCE sync workd
const int h_mce_syncword_width = 5;  // yes 40 bits, bletch. 


const char server_port_number[] = "5345";  // default unless overriddenw was 5433
//const char server_ip_addr[] = "127.0.0.1"; //default
const char server_ip_addr[] = "134.79.216.240"; //default - lcls-dev3
//const char server_ip_addr[] = "134.79.228.97"; // tid-pc93130
//const char server_ip_addr[] = "192.168.3.1"; // harvard bicep53
const char pipe_name[] = "/tmp/smurffifo"; // named pipe for MCE interface
//const char pipe_name[] = "/data/cryo/mas_data_pipe";

#endif
