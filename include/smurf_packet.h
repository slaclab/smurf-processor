#ifndef _SMURF_PACKET_H_
#define _SMURF_PACKET_H_

#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/timeb.h>
#include <vector>

#include "common.h"

#include "smurf2mce.h"

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


// SmurfHeader class
// This class contains methods to access the different elements
// in the SmurfHeader.
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
  SmurfHeader(uint8_t *buffer); // creates header and set pointer

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

// SmurfPakcet Class
// This class handler SMuRF packets.
class SmurfPacket
{
public:
  // Default constructor
  SmurfPacket();

  // Constructor using a raw array for the header data
  SmurfPacket(uint8_t* h);

  // Constructor using a raw array for the header and payload data
  SmurfPacket(uint8_t* h, avgdata_t* d);

  // Destructor
  ~SmurfPacket();

  // Get the length of the header in number of bytes
  const std::size_t getHeaderLength()  const;

  // Get the length of the payload in number of avgdata_t words
  const std::size_t getPayloadLength() const;

  // Get the total length of the packet in number of bytes
  const std::size_t getPacketLength()  const;

  // Header function //
  const uint8_t  getVersion()             const;  // Get protocol version
  const uint8_t  getCrateID()             const;  // Get ATCA crate ID
  const uint8_t  getSlotNumber()          const;  // Get ATCA slot number
  const uint8_t  getTimingConfiguration() const;  // Get timing configuration
  const uint32_t getNumberChannels()      const;  // Get number of channel in this packet

  // Copy an array of bytes into the header
  void copyHeader(uint8_t* h);

  // Copy an array of avgdata_t's into the payload
  void copyData(avgdata_t* d);

  // Write the packet into a file
  void writeToFile(uint fd) const;

  // Get a pointer to a SmurfHeader object, to access the header information
  SmurfHeader* getHeaderPtr();

  // Get a data value, at a specified index
  const avgdata_t getValue(std::size_t index) const;

  // Set a data value, at a specific index
  void setValue(std::size_t index, avgdata_t value);

  // Get a byte from the header, at a specified index
  const uint8_t getHeaderByte(std::size_t index) const;

  // Set a byte on the header, at a specific index
  void setHeaderByte(std::size_t index, uint8_t value);

  // Get a copy of the header bufefr as an array of bytes
  void getHeaderArray(uint8_t* h) const;

  // Get a copy of the data buffer as an array of avgdata_t
  void getDataArray(avgdata_t* d) const;

private:
  std::size_t            headerLength;  // Header length (number of bytes)
  std::size_t            payloadLength; // Payload size (number of avgdata_t)
  std::size_t            packetLength;  // Total packet length (number of bytes)
  std::vector<uint8_t>   headerBuffer;  // Header buffer
  std::vector<avgdata_t> payloadBuffer; // Payload buffer
  SmurfHeader            header;        // Packet header object
};

#endif