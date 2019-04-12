#ifndef _SMURF_PACKET_H_
#define _SMURF_PACKET_H_

#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/timeb.h>

#include "common.h"

uint64_t pull_bit_field(uint8_t *ptr, uint offset, uint width);

// smurf header byte offsets
const int h_version_offset = 0; // offset of version word
const int h_version_width = 1; // bytes of version word
const int h_num_channels_offset = 4; // normally 528 channels
const int h_num_channels_width = 4;  // 32 bit number

const int h_unix_time_offset = 48; // offset to 64 bit unix time
const int h_unix_time_width = 8;   // 64 bit timing word

const int h_1hz_counter_offset = 64;  // resets with next MCE word
const int h_1hz_counter_width = 4; // width
const int h_ext_counter_offset = 68;  // resets with next MCE word
const int h_ext_counter_width = 4; // width
const int h_epics_ns_offset = 72;  // from timing system, epics time nanoseconds
const int h_epics_ns_width = 4;
const int h_epics_s_offset = 76;  // timing system epics time seconds
const int h_epics_s_width = 4;
const int h_frame_counter_offset = 84;  // raw frame counter.
const int h_frame_counter_width = 4;
const int h_mce_syncword_offset = 96;  // 20 bit MCE sync workd
const int h_mce_syncword_width = 5;  // yes 40 bits, bletch.

const int h_user0a_ctrl_offset = 104; // first byte first user word, control smurfd
const int h_user0a_ctrl_width = 2;
// bit fields
const int h_ctrl_bit_clear = 0;  // 1 to clear average and unwrap
const int h_ctrl_bit_disable_stream = 1;  // 1 to disable streming to mce
const int h_ctrl_bit_disable_file  = 2;  // 1 to disable writing to files
const int h_ctrl_bit_read_config = 3;  // set to read config file each cycle
const int h_ctrl_nibble_test_modes = 4; // used to enable various test modes

const int h_user0b_ctrl_offset = 105;
const int h_user0b_ctrl_width = 1;

const int h_num_rows_offset = 112;
const int h_num_rows_width = 2;
const int h_num_rows_reported_offset = 114;
const int h_num_rows_reported_width = 2;
const int h_row_len_offset = 120;
const int h_row_len_width = 2;
const int h_data_rate_offset = 122;
const int h_data_rate_width = 2;


class SmurfHeader //generates and decodes SMURF data header
{
public:
  uint8_t *header; // full header bytes
  uint last_frame_count;
  bool first_cycle;
  bool data_ok; // set to indicate taht data has passed internal checks.
  bool average_ok;  // set for a bad header somewhere in average
  uint average_counter;
  uint last_ext_counter;
  uint last_syncword;
  uint delta_syncword;
  uint64_t bigtimems; // time in millliseconds
  uint64_t lastbigtime;
  uint64_t unix_dtime;
  uint32_t epics_seconds;
  uint32_t epics_nanoseconds;

  SmurfHeader(void); // creates header with num samples
  void copy_header(uint8_t *buffer);
  uint get_version(void);
  uint get_ext_counter(void);
  uint get_1hz_counter(void);
  uint get_frame_counter(void);
  uint get_average_bit(void) { return(0);}; // place holder
  uint get_syncword(void); // returns 20 bit MCE sync word
  uint get_epics_nanoseconds(void);
  uint get_epics_seconds(void);
  uint get_clear_bit(void);  // 1 means clear averaging and unwrap
  uint disable_file_write(void); // 1 means don't write a local output file
  uint disable_stream(void); // 1 means don't stream to MCE
  uint read_config_file(void); // 1 means read config file
  uint average_control(int num); // num=0 means use external average,
  uint get_num_rows(void);  // num rows from header
  uint get_num_rows_reported(void);
  uint get_row_len(void);
  uint get_data_rate(void);
  uint get_test_parameter(void);
  uint get_test_mode(void); //  0 = normal, 1 -> all zeros, 2 -> by channnel
  void set_num_channels(uint32_t num_ch); // Set the number of channels in the header
  uint32_t get_num_channels();            // Get the number of channels from the header

  void put_field(int offset, int width, void *data);  // for adding to smurf header

  void clear_average(); // clears aveage counters
};

#endif