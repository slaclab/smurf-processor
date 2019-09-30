#ifndef _SMURF_PACKET_H_
#define _SMURF_PACKET_H_

#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/timeb.h>
#include <vector>
#include <stdexcept>
#include <memory>

#include "common.h"
#include "tes_bias_array.h"

#include "smurf2mce.h"

uint64_t pull_bit_field(uint8_t *ptr, uint offset, uint width);

// smurf header byte offsets
const int h_version_offset = 0; // offset of version word
const int h_version_width = 1; // bytes of version word
const int h_num_channels_offset = 4; // normally 528 channels
const int h_num_channels_width = 4;  // 32 bit number

const int h_tes_dac_offset = 8; // TES DAC 0-15
const int h_tes_dac_width = 40; // 16x 20-bit

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


class ISmurfPacket_RO;
class ISmurfPacket;
typedef std::shared_ptr<ISmurfPacket_RO>  SmurfPacket_RO;
typedef std::shared_ptr<ISmurfPacket>     SmurfPacket;

// SmurfPakcet Class
// This class handler SMuRF packets.
// This class gives a read-only interface
class ISmurfPacket_RO
{
public:
  // Get the length of the header in number of bytes
  const std::size_t getHeaderLength()  const;

  // Get the length of the payload in number of avgdata_t words
  const std::size_t getPayloadLength() const;

  // Get the total length of the packet in number of bytes
  const std::size_t getPacketLength()  const;

  // Get a copy of the header bufefr as an array of bytes
  void getHeaderArray(uint8_t* h) const;

  // Get a copy of the data buffer as an array of avgdata_t
  void getDataArray(avgdata_t* d) const;

  // Header functions //
  const uint8_t  getVersion()                   const;  // Get protocol version
  const uint8_t  getCrateID()                   const;  // Get ATCA crate ID
  const uint8_t  getSlotNumber()                const;  // Get ATCA slot number
  const uint8_t  getTimingConfiguration()       const;  // Get timing configuration
  const uint32_t getNumberChannels()            const;  // Get number of channel in this packet
  const int32_t  getTESBias(std::size_t index)  const;  // Get TES DAC values 16X 20 bit
  const uint64_t getUnixTime()                  const;  // Get 64 bit unix time nanoseconds
  const uint32_t getFluxRampIncrement()         const;  // Get signed 32 bit integer for increment
  const uint32_t getFluxRampOffset()            const;  // Get signed 32 it integer for offset
  const uint32_t getCounter0()                  const;  // Get 32 bit counter since last 1Hz marker
  const uint32_t getCounter1()                  const;  // Get 32 bit counter since last external input
  const uint64_t getCounter2()                  const;  // Get 64 bit timestamp
  const uint32_t getAveragingResetBits()        const;  // Get up to 32 bits of average reset from timing system
  const uint32_t getFrameCounter()              const;  // Get locally genreate frame counter 32 bit
  const uint32_t getTESRelaySetting()           const;  // Get TES and flux ramp relays, 17bits in use now
  const uint64_t getExternalTimeClock()         const;  // Get Syncword from mce for mce based systems (40 bit including header)
  const uint8_t  getControlField()              const;  // Get control field word
  const bool     getClearAverageBit()           const;  // Get control field's clear average and unwrap bit (bit 0)
  const bool     getDisableStreamBit()          const;  // Get control field's disable stream to MCE bit (bit 1)
  const bool     getDisableFileWriteBit()       const;  // Get control field's disable file write (bit 2)
  const bool     getReadConfigEachCycleBit()    const;  // Get control field's set to read configuration file each cycle bit (bit 3)
  const uint8_t  getTestMode()                  const;  // Get control field's test mode (bits 4-7)
  const uint8_t  getTestParameters()            const;  // Get test parameters
  const uint16_t getNumberRows()                const;  // Get MCE header value (max 255) (defaluts to 33 if 0)
  const uint16_t getNumberRowsReported()        const;  // Get MCE header value (defaults to numb rows if 0)
  const uint16_t getRowLength()                 const;  // Get MCE header value
  const uint16_t getDataRate()                  const;  // Get MCE header value

  // Get a data value, at a specified index
  const avgdata_t getValue(std::size_t index) const;

  // Get a raw byte from the header, at a specified index
  const uint8_t getHeaderByte(std::size_t index) const;

  // Write the packet into a file
  void writeToFile(uint fd) const;

  // Factory method, which return a smart pointer to a SmurfPacket object
  static SmurfPacket_RO create(const SmurfPacket& sp);

protected:
  // Prevent construction of ISmurfPacket with RO interface.
  // ISmurfPacket must be created with a RW interface, and this class
  // can be used to give read only access to the data.
  ISmurfPacket_RO();
  ISmurfPacket_RO(const ISmurfPacket_RO&);
  ISmurfPacket_RO& operator=(const ISmurfPacket_RO&);
  virtual ~ISmurfPacket_RO();

  std::size_t            headerLength;  // Header length (number of bytes)
  std::size_t            payloadLength; // Payload size (number of avgdata_t)
  std::size_t            packetLength;  // Total packet length (number of bytes)
  std::vector<uint8_t>   headerBuffer;  // Header buffer
  std::vector<avgdata_t> payloadBuffer; // Payload buffer
  SmurfHeader            header;        // Packet header object
  TesBiasArray           tba;           // Tes Bias array object

  // Header word offsets (in bytes)
  static const std::size_t headerVersionOffset              = 0;
  static const std::size_t headerCrateIDOffset              = 1;
  static const std::size_t headerSlotNumberOffset           = 2;
  static const std::size_t headerTimingConfigurationOffset  = 3;
  static const std::size_t headerNumberChannelOffset        = 4;
  static const std::size_t headerTESDACOffset               = 8;
  static const std::size_t headerUnixTimeOffset             = 48;
  static const std::size_t headerFluxRampIncrementOffset    = 56;
  static const std::size_t headerFluxRampOffsetOffset       = 60;
  static const std::size_t headerCounter0Offset             = 64;
  static const std::size_t headerCounter1Offset             = 68;
  static const std::size_t headerCounter2Offset             = 72;
  static const std::size_t headerAveragingResetBitsOffset   = 80;
  static const std::size_t headerFrameCounterOffset         = 84;
  static const std::size_t headerTESRelaySettingOffset      = 88;
  static const std::size_t headerExternalTimeClockOffset    = 96;
  static const std::size_t headerControlFieldOffset         = 104;
  static const std::size_t headerTestParametersOffset       = 105;
  static const std::size_t headerNumberRowsOffset           = 112;
  static const std::size_t headerNumberRowsReportedOffset   = 114;
  static const std::size_t headerRowLengthOffset            = 120;
  static const std::size_t headerDataRateOffset             = 122;

  // Header's control field bit offset
  static const std::size_t clearAvergaveBitOffset           = 0;
  static const std::size_t disableStreamBitOffset           = 1;
  static const std::size_t disableFileWriteBitOffset        = 2;
  static const std::size_t readConfigEachCycleBitOffset     = 3;

private:
  // Get a word from the header
  template<typename T>
  const T getHeaderWord(std::size_t offset) const;

  // Get the bit number 'index' of the word 'byte'
  const bool getWordBit(uint8_t byte, std::size_t index) const;
};

// SmurfPakcet Class
// This class handler SMuRF packets.
// This class gives a full read-write interface
class ISmurfPacket : public ISmurfPacket_RO
{
public:
  // Default constructor
  ISmurfPacket();

  // Constructor using a raw array for the header data
  ISmurfPacket(uint8_t* h);

  // Constructor using a raw array for the header and payload data
  ISmurfPacket(uint8_t* h, avgdata_t* d);

  // Destructor
  virtual ~ISmurfPacket();

  // Copy an array of bytes into the header
  void copyHeader(uint8_t* h);

  // Copy an array of avgdata_t's into the payload
  void copyData(avgdata_t* d);

  // Header functions //
  void setVersion(uint8_t value);                     // Get protocol version
  void setCrateID(uint8_t value);                     // Get ATCA crate ID
  void setSlotNumber(uint8_t value);                  // Get ATCA slot number
  void setTimingConfiguration(uint8_t value);         // Get timing configuration
  void setNumberChannels(uint32_t value);             // Get number of channel in this packet
  void setTESBias(std::size_t index, int32_t value);  // Get TES DAC values 16X 20 bit
  void setUnixTime(uint64_t value);                   // Get 64 bit unix time nanoseconds
  void setFluxRampIncrement(uint32_t value);          // Get signed 32 bit integer for increment
  void setFluxRampOffset(uint32_t value);             // Get signed 32 it integer for offset
  void setCounter0(uint32_t value);                   // Get 32 bit counter since last 1Hz marker
  void setCounter1(uint32_t value);                   // Get 32 bit counter since last external input
  void setCounter2(uint64_t value);                   // Get 64 bit timestamp
  void setAveragingResetBits(uint32_t value);         // Get up to 32 bits of average reset from timing system
  void setFrameCounter(uint32_t value);               // Get locally genreate frame counter 32 bit
  void setTESRelaySetting(uint32_t value);            // Get TES and flux ramp relays, 17bits in use now
  void setExternalTimeClock(uint64_t value);          // Get Syncword from mce for mce based systems (40 bit including header)
  void setControlField(uint8_t value);                // Get control field word
  void setClearAverageBit(bool value);                // Get control field's clear average and unwrap bit (bit 0)
  void setDisableStreamBit(bool value);               // Get control field's disable stream to MCE bit (bit 1)
  void setDisableFileWriteBit(bool value);            // Get control field's disable file write (bit 2)
  void setReadConfigEachCycleBit(bool value);         // Get control field's set to read configuration file each cycle bit (bit 3)
  void setTestMode(uint8_t value);                    // Get control field's test mode (bits 4-7)
  void setTestParameters(uint8_t value);              // Get test parameters
  void setNumberRows(uint16_t value);                 // Get MCE header value (max 255) (defaluts to 33 if 0)
  void setNumberRowsReported(uint16_t value);         // Get MCE header value (defaults to numb rows if 0)
  void setRowLength(uint16_t value);                  // Get MCE header value
  void setDataRate(uint16_t value);                   // Get MCE header value

  // Set a raw byte in the header, at a specific index
  void setHeaderByte(std::size_t index, uint8_t value);

  // Set a data value, at a specific index
  void setValue(std::size_t index, avgdata_t value);

  // Factory methods, which return smart pointer
  static SmurfPacket create();
  static SmurfPacket create(uint8_t* h);
  static SmurfPacket create(uint8_t* h, avgdata_t* d);

private:
  // Get a word from the header
  template<typename T>
  void setHeaderWord(std::size_t offset, const T& value);

  // Set bit number 'index' to 'value' in the word 'byte'
  uint8_t setWordBit(uint8_t byte, std::size_t index, bool value);
};

#endif