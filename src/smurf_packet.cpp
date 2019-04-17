#include "smurf_packet.h"

////////////////////////////////////////
////// + SmurfHeader definitions ///////
////////////////////////////////////////

// Decodes information in the header part of the data from smurf
SmurfHeader::SmurfHeader()
{

  last_frame_count = 0;
  first_cycle = 1;
  average_counter = 0;  // number of frames avearaged so far
  data_ok = true;  // start of assuming data is OK, invalidate later.
  average_ok = true;
  last_ext_counter = 0;  // this tracks the counter rolling over
  last_syncword = 0;
  delta_syncword = 0;
  bigtimems=0;
  lastbigtime = 0;
  unix_dtime = 0;
}

SmurfHeader::SmurfHeader(uint8_t *buffer)
:
  SmurfHeader()
{
  copy_header(buffer);
}

void SmurfHeader::copy_header(uint8_t *buffer)
{
  header = buffer;  // just move the pointer
  data_ok = true;  // This is where we first get new data so star with header OK.
}


uint SmurfHeader::get_version(void)
{
  return(pull_bit_field(header, h_version_offset, h_version_width));
}


uint SmurfHeader::get_frame_counter(void)
{
  return(pull_bit_field(header, h_frame_counter_offset, h_frame_counter_width));
}

uint SmurfHeader::get_1hz_counter(void)
{
  return(pull_bit_field(header, h_1hz_counter_offset, h_1hz_counter_width));
}

uint SmurfHeader::get_ext_counter(void)
{
  return(pull_bit_field(header, h_ext_counter_offset, h_ext_counter_width));
}

uint SmurfHeader::get_syncword(void)
{
  uint64_t x;
  x = pull_bit_field(header, h_mce_syncword_offset, h_mce_syncword_width);
  return(x & 0xFFFFFFFF);  // pull out the counter.
}

uint SmurfHeader::get_epics_seconds(void)
{
  uint64_t x;
  x = pull_bit_field(header, h_epics_s_offset, h_epics_s_width);
  return(x & 0xFFFFFFFF);  // pull out the counter.
}

uint SmurfHeader::get_epics_nanoseconds(void)
{
  uint64_t x;
  x = pull_bit_field(header, h_epics_ns_offset, h_epics_ns_width);
  return(x & 0xFFFFFFFF);  // pull out the counter.
}

uint SmurfHeader::get_clear_bit(void)
{
  uint64_t x;
  x = pull_bit_field(header,  h_user0a_ctrl_offset, h_user0a_ctrl_width);
  return((x & (1 << h_ctrl_bit_clear))? 1 : 0);
}

uint SmurfHeader::disable_file_write(void)
{
  uint64_t x;
  x = pull_bit_field(header,  h_user0a_ctrl_offset, h_user0a_ctrl_width);
  return((x & (1 << h_ctrl_bit_disable_file))? 1 : 0);
}

uint SmurfHeader::disable_stream(void) //
{
  uint64_t x;
  x = pull_bit_field(header,  h_user0a_ctrl_offset, h_user0a_ctrl_width);
  return((x & (1 <<  h_ctrl_bit_disable_stream))? 1 :0);
}


uint SmurfHeader::get_test_mode(void)
{
  uint x;
  x = pull_bit_field(header, h_user0a_ctrl_offset, h_user0a_ctrl_width);
  return((x & (0xF <<  h_ctrl_nibble_test_modes)) >>  4);
}

uint SmurfHeader::get_num_rows(void)
{
  uint x;
  x = pull_bit_field(header, h_num_rows_offset, h_num_rows_width);
  // return(x ? x: MCEheader_num_rows_value);  // if zero go to default
  return(x ? x: 33); // Not sure what to do here.
}

uint SmurfHeader::get_num_rows_reported(void)
{
  uint x;
  x = pull_bit_field(header, h_num_rows_reported_offset, h_num_rows_reported_width);
  // return(x ? x: MCEheader_num_rows_reported_value);
  return(x ? x: 33); // Not sure what to do here.

}

uint SmurfHeader::get_row_len(void)
{
  uint x;
  x = pull_bit_field(header, h_row_len_offset, h_row_len_width);
  // return(x ? x: MCEheader_row_len_value);
  return(x ? x: 60); // Not sure what to do here.
}

uint SmurfHeader::get_data_rate(void)
{
  uint x;
  x = pull_bit_field(header, h_data_rate_offset, h_data_rate_width);
  // return(x ? x: MCEheader_data_rate_value);
  return(x ? x: 140); // Not sure what to do here.
}


uint SmurfHeader::get_test_parameter(void)
{
  uint x;
  x = pull_bit_field(header, h_user0b_ctrl_offset , h_user0b_ctrl_width);
  return(x);
}

void SmurfHeader::set_num_channels(uint32_t num_ch)
{
  put_field(h_num_channels_offset, h_num_channels_offset, &num_ch);
}

uint32_t SmurfHeader::get_num_channels()
{
  return(pull_bit_field(header, h_num_channels_offset, h_num_channels_width));
}

void SmurfHeader::put_field(int offset, int width, void *data)
{
  memcpy(header+offset, data, width); // not protected, proabably  a bad idea.
}

uint SmurfHeader::read_config_file(void)
{
  uint64_t x;
  x = pull_bit_field(header,  h_user0a_ctrl_offset, h_user0a_ctrl_width);
  return((x & (1 <<  h_ctrl_bit_read_config))? 1 :0);
}


void SmurfHeader::clear_average(void)
{
  average_counter=0;
}

uint SmurfHeader::average_control(int num_averages) // returns num averages when avearaging is done.
{
  uint x=0, y;
  timeb tm;  // wil lhold time imnformation
  ftime(&tm); // get time
  bigtimems = tm.millitm  + 1000 * tm.time; // make64 bit milliecond clock
  unix_dtime = bigtimems - lastbigtime;
  lastbigtime = bigtimems;


  y = get_syncword();
  delta_syncword = y - last_syncword;
  last_syncword = y;

  if (average_counter ==0)
    average_ok = data_ok;  // reset avearge ok bit.

  average_counter++; // increment number of frames averaged.

  if (num_averages)
  {
    if (average_counter == num_averages)
    {
      average_counter = 0;
      return (num_averages); // end averaging with local control
    }
  }
  else
  {
    y = get_ext_counter();

    if (delta_syncword)  //
    {
      x = average_counter; // number of averages
      average_counter = 0; // reset average
    }

    last_ext_counter = y; // copy over counter
    return(x);  // return, 0 to kep averaging, otehr to zero average.
  }

  return(0);
}

////////////////////////////////////////
////// - SmurfHeader definitions ///////
////////////////////////////////////////

////////////////////////////////////////
////// + SmurfPacket definitions ///////
////////////////////////////////////////

// Default constructor
SmurfPacket::SmurfPacket()
:
  headerLength(smurfheaderlength),
  payloadLength(smurfsamples),
  packetLength(smurfheaderlength + smurfsamples * sizeof(avgdata_t)),
  headerBuffer(smurfheaderlength),
  payloadBuffer(smurfsamples),
  header(headerBuffer.data())
{
  std::cout << "SmurfPacket object created:" << std::endl;
  std::cout << "Header length       = " << headerLength  << " bytes" << std::endl;
  std::cout << "Payload length      = " << payloadLength << " words" << std::endl;
  std::cout << "Total packet length = " << packetLength  << " bytes" << std::endl;
}

SmurfPacket::SmurfPacket(uint8_t* h)
:
  SmurfPacket()
{
  copyHeader(h);
}

SmurfPacket::SmurfPacket(uint8_t* h, avgdata_t* d)
:
  SmurfPacket(h)
{
  copyData(d);
}

SmurfPacket::~SmurfPacket()
{
  std::cout << "SmurfPacket object destroyed" << std::endl;
}

const std::size_t SmurfPacket::getHeaderLength()  const
{
  return headerLength;
}

const std::size_t SmurfPacket::getPayloadLength() const
{
  return payloadLength;
}

const std::size_t SmurfPacket::getPacketLength()  const
{
  return packetLength;
}

void SmurfPacket::copyHeader(uint8_t* h)
{
  memcpy(headerBuffer.data(), h, headerLength);
}

void SmurfPacket::copyData(avgdata_t* d)
{
  memcpy(payloadBuffer.data(), d, payloadLength * sizeof(avgdata_t));
}

void SmurfPacket::writeToFile(uint fd) const
{
  write(fd, headerBuffer.data(), headerLength);
  write(fd, payloadBuffer.data(), payloadLength * sizeof(avgdata_t));
}

SmurfHeader* SmurfPacket::getHeaderPtr()
{
  return &header;
}

const avgdata_t SmurfPacket::getValue(std::size_t index) const
{
  return payloadBuffer.at(index);
}

void SmurfPacket::setValue(std::size_t index, avgdata_t value)
{
  payloadBuffer.at(index) = value;
}

const uint8_t SmurfPacket::getHeaderByte(std::size_t index) const
{
  return headerBuffer.at(index);
}

void SmurfPacket::setHeaderByte(std::size_t index, uint8_t value)
{
  headerBuffer.at(index) = value;
}

void SmurfPacket::getHeaderArray(uint8_t* h) const
{
  memcpy(h, headerBuffer.data(), headerLength);
}

void SmurfPacket::getDataArray(avgdata_t* d) const
{
  memcpy(d, payloadBuffer.data(), payloadLength * sizeof(avgdata_t));
}

////////////////////////////////////////
////// - SmurfPacket definitions ///////
////////////////////////////////////////

uint64_t pull_bit_field(uint8_t *ptr, uint offset, uint width)
{
  uint64_t x;  // will hold version number
  uint64_t r;
  uint64_t tmp;

  if(width > sizeof(uint64_t))
    error("field width too big");

  r = (1UL << (width*8)) -1;
  memcpy(&x, ptr+offset, width); // move the bytes over
  tmp = r & (uint64_t)x;
  return(r & tmp);
}