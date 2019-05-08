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

///////////////////////////////////////////
////// + ISmurfPacket_RO definitions ///////
///////////////////////////////////////////

// Default constructor
ISmurfPacket_RO::ISmurfPacket_RO()
:
  headerLength(smurfheaderlength),
  payloadLength(smurfsamples),
  packetLength(smurfheaderlength + smurfsamples * sizeof(avgdata_t)),
  headerBuffer(smurfheaderlength),
  payloadBuffer(smurfsamples),
  header(headerBuffer.data()),
  tba(&headerBuffer.at(headerTESDACOffset))
{
  std::cout << "ISmurfPacket_RO object created:" << std::endl;
  std::cout << "Header length       = " << headerLength  << " bytes" << std::endl;
  std::cout << "Payload length      = " << payloadLength << " words" << std::endl;
  std::cout << "Total packet length = " << packetLength  << " bytes" << std::endl;
}

ISmurfPacket_RO::~ISmurfPacket_RO()
{
  std::cout << "ISmurfPacket_RO object destroyed" << std::endl;
}

const std::size_t ISmurfPacket_RO::getHeaderLength()  const
{
  return headerLength;
}

const std::size_t ISmurfPacket_RO::getPayloadLength() const
{
  return payloadLength;
}

const std::size_t ISmurfPacket_RO::getPacketLength()  const
{
  return packetLength;
}

const uint8_t ISmurfPacket_RO::getVersion() const
{
  return getHeaderWord<uint8_t>(headerVersionOffset);
}

const uint8_t ISmurfPacket_RO::getCrateID() const
{
  return getHeaderWord<uint8_t>(headerCrateIDOffset);
}

const uint8_t ISmurfPacket_RO::getSlotNumber() const
{
  return getHeaderWord<uint8_t>(headerSlotNumberOffset);
}

const uint8_t ISmurfPacket_RO::getTimingConfiguration() const
{
  return getHeaderWord<uint8_t>(headerTimingConfigurationOffset);
}

const uint32_t ISmurfPacket_RO::getNumberChannels() const
{
  return getHeaderWord<uint32_t>(headerNumberChannelOffset);
}

const int32_t ISmurfPacket_RO::getTESBias(std::size_t index) const
{
  return tba.getWord(index);
}

const uint64_t ISmurfPacket_RO::getUnixTime() const
{
  return getHeaderWord<uint64_t>(headerUnixTimeOffset);
}

const uint32_t ISmurfPacket_RO::getFluxRampIncrement() const
{
  return getHeaderWord<uint32_t>(headerFluxRampIncrementOffset);
}

const uint32_t ISmurfPacket_RO::getFluxRampOffset() const
{
  return getHeaderWord<uint32_t>(headerFluxRampOffsetOffset);
}

const uint32_t ISmurfPacket_RO::getCounter0() const
{
  return getHeaderWord<uint32_t>(headerCounter0Offset);
}

const uint32_t ISmurfPacket_RO::getCounter1() const
{
  return getHeaderWord<uint32_t>(headerCounter1Offset);
}

const uint64_t ISmurfPacket_RO::getCounter2() const
{
  return getHeaderWord<uint64_t>(headerCounter2Offset);
}

const uint32_t ISmurfPacket_RO::getAveragingResetBits() const
{
  return getHeaderWord<uint32_t>(headerAveragingResetBitsOffset);
}

const uint32_t ISmurfPacket_RO::getFrameCounter() const
{
  return getHeaderWord<uint32_t>(headerFrameCounterOffset);
}

const uint32_t ISmurfPacket_RO::getTESRelaySetting() const
{
  return getHeaderWord<uint32_t>(headerTESRelaySettingOffset);
}

const uint64_t ISmurfPacket_RO::getExternalTimeClock() const
{
  return getHeaderWord<uint64_t>(headerExternalTimeClockOffset);
}

const uint8_t ISmurfPacket_RO::getControlField() const
{
  return getHeaderWord<uint8_t>(headerControlFieldOffset);
}

const bool ISmurfPacket_RO::getClearAverageBit() const
{
  return getWordBit(getHeaderWord<uint8_t>(headerControlFieldOffset), clearAvergaveBitOffset);
}

const bool ISmurfPacket_RO::getDisableStreamBit() const
{
  return getWordBit(getHeaderWord<uint8_t>(headerControlFieldOffset), disableStreamBitOffset);
}

const bool ISmurfPacket_RO::getDisableFileWriteBit() const
{
  return getWordBit(getHeaderWord<uint8_t>(headerControlFieldOffset), disableFileWriteBitOffset);
}

const bool ISmurfPacket_RO::getReadConfigEachCycleBit() const
{
  return getWordBit(getHeaderWord<uint8_t>(headerControlFieldOffset), readConfigEachCycleBitOffset);
}

const uint8_t ISmurfPacket_RO::getTestMode() const
{
  return ((getHeaderWord<uint8_t>(headerControlFieldOffset) >> 4) & 0x0f);
}

const uint8_t ISmurfPacket_RO::getTestParameters() const
{
  return getHeaderWord<uint8_t>(headerTestParametersOffset);
}

const uint16_t ISmurfPacket_RO::getNumberRows() const
{
  return getHeaderWord<uint16_t>(headerNumberRowsOffset);
}

const uint16_t ISmurfPacket_RO::getNumberRowsReported() const
{
  return getHeaderWord<uint16_t>(headerNumberRowsReportedOffset);
}

const uint16_t ISmurfPacket_RO::getRowLength() const
{
  return getHeaderWord<uint16_t>(headerRowLengthOffset);
}

const uint16_t ISmurfPacket_RO::getDataRate() const
{
  return getHeaderWord<uint16_t>(headerDataRateOffset);
}

template <typename T>
const T ISmurfPacket_RO::getHeaderWord(std::size_t offset) const
{
  return *(reinterpret_cast<const T*>(&headerBuffer.at(offset)));
}

const bool ISmurfPacket_RO::getWordBit(uint8_t byte, std::size_t index) const
{
  if (index >= 8)
    throw std::runtime_error("Trying to get a bit with index > 8 from a byte");

  return (byte & (0x01 << index));
}

void ISmurfPacket_RO::writeToFile(uint fd) const
{
  write(fd, headerBuffer.data(), headerLength);
  write(fd, payloadBuffer.data(), payloadLength * sizeof(avgdata_t));
}

const avgdata_t ISmurfPacket_RO::getValue(std::size_t index) const
{
  return payloadBuffer.at(index);
}

const uint8_t ISmurfPacket_RO::getHeaderByte(std::size_t index) const
{
  return headerBuffer.at(index);
}

void ISmurfPacket_RO::getHeaderArray(uint8_t* h) const
{
  memcpy(h, headerBuffer.data(), headerLength);
}

void ISmurfPacket_RO::getDataArray(avgdata_t* d) const
{
  memcpy(d, payloadBuffer.data(), payloadLength * sizeof(avgdata_t));
}

SmurfPacket_RO ISmurfPacket_RO::create(const SmurfPacket& sp)
{
  return sp;
}
///////////////////////////////////////////
////// - ISmurfPacket_RO definitions ///////
///////////////////////////////////////////


////////////////////////////////////////
////// - ISmurfPacket definitions ///////
////////////////////////////////////////
ISmurfPacket::ISmurfPacket()
:
  ISmurfPacket_RO()
{
  std::cout << "ISmurfPacket() object created" << std::endl;
}

ISmurfPacket::ISmurfPacket(uint8_t* h)
:
  ISmurfPacket()
{
  std::cout << "ISmurfPacket(uint8_t* h) object created" << std::endl;
  copyHeader(h);
}

ISmurfPacket::ISmurfPacket(uint8_t* h, avgdata_t* d)
:
  ISmurfPacket(h)
{
  std::cout << "ISmurfPacket(uint8_t* h, avgdata_t* d) object created" << std::endl;
  copyData(d);
}

ISmurfPacket::~ISmurfPacket()
{
  std::cout << "ISmurfPacket object destroyed" << std::endl;
}

void ISmurfPacket::copyHeader(uint8_t* h)
{
  memcpy(headerBuffer.data(), h, headerLength);
}

void ISmurfPacket::copyData(avgdata_t* d)
{
  memcpy(payloadBuffer.data(), d, payloadLength * sizeof(avgdata_t));
}

void ISmurfPacket::setVersion(uint8_t value)
{
  setHeaderWord<uint8_t>(headerVersionOffset, value);
}

void ISmurfPacket::setCrateID(uint8_t value)
{
  setHeaderWord<uint8_t>(headerCrateIDOffset, value);
}

void ISmurfPacket::setSlotNumber(uint8_t value)
{
  setHeaderWord<uint8_t>(headerSlotNumberOffset, value);
}

void  ISmurfPacket::setTimingConfiguration(uint8_t value)
{
  setHeaderWord<uint8_t>(headerTimingConfigurationOffset, value);
}

void ISmurfPacket::setNumberChannels(uint32_t value)
{
  setHeaderWord<uint32_t>(headerNumberChannelOffset, value);
}

void  ISmurfPacket::setTESBias(std::size_t index, int32_t value)
{
  tba.setWord(index, value);
}

void ISmurfPacket::setUnixTime(uint64_t value)
{
  setHeaderWord<uint64_t>(headerUnixTimeOffset, value);
}

void ISmurfPacket::setFluxRampIncrement(uint32_t value)
{
  setHeaderWord<uint32_t>(headerFluxRampIncrementOffset, value);
}

void ISmurfPacket::setFluxRampOffset(uint32_t value)
{
  setHeaderWord<uint32_t>(headerFluxRampOffsetOffset, value);
}

void ISmurfPacket::setCounter0(uint32_t value)
{
  setHeaderWord<uint32_t>(headerCounter0Offset, value);
}

void ISmurfPacket::setCounter1(uint32_t value)
{
  setHeaderWord<uint32_t>(headerCounter1Offset, value);
}

void ISmurfPacket::setCounter2(uint64_t value)
{
  setHeaderWord<uint64_t>(headerCounter2Offset, value);
}

void ISmurfPacket::setAveragingResetBits(uint32_t value)
{
  setHeaderWord<uint32_t>(headerAveragingResetBitsOffset, value);
}

void ISmurfPacket::setFrameCounter(uint32_t value)
{
  setHeaderWord<uint32_t>(headerFrameCounterOffset, value);
}

void ISmurfPacket::setTESRelaySetting(uint32_t value)
{
  setHeaderWord<uint32_t>(headerTESRelaySettingOffset, value);
}

void ISmurfPacket::setExternalTimeClock(uint64_t value)
{
  setHeaderWord<uint64_t>(headerExternalTimeClockOffset, value);
}

void ISmurfPacket::setControlField(uint8_t value)
{
  setHeaderWord<uint8_t>(headerControlFieldOffset, value);
}

void ISmurfPacket::setClearAverageBit(bool value)
{
  setHeaderWord<uint8_t>(headerControlFieldOffset, \
    setWordBit(getControlField(), clearAvergaveBitOffset, value));
}

void ISmurfPacket::setDisableStreamBit(bool value)
{
  setHeaderWord<uint8_t>(headerControlFieldOffset, \
    setWordBit(getControlField(), disableStreamBitOffset, value));
}

void ISmurfPacket::setDisableFileWriteBit(bool value)
{
  setHeaderWord<uint8_t>(headerControlFieldOffset, \
    setWordBit(getControlField(), disableFileWriteBitOffset, value));
}

void ISmurfPacket::setReadConfigEachCycleBit(bool value)
{
  setHeaderWord<uint8_t>(headerControlFieldOffset, \
    setWordBit(getControlField(), readConfigEachCycleBitOffset, value));
}

void ISmurfPacket::setTestMode(uint8_t value)
{
  uint8_t u8 = getControlField();

  u8 &= 0x0f;
  u8 |= ( (value << 4 ) & 0xf0 );

  setHeaderWord<uint8_t>(headerControlFieldOffset, u8);
}

void ISmurfPacket::setTestParameters(uint8_t value)
{
  setHeaderWord<uint8_t>(headerTestParametersOffset, value);
}

void ISmurfPacket::setNumberRows(uint16_t value)
{
  setHeaderWord<uint16_t>(headerNumberRowsOffset, value);
}

void ISmurfPacket::setNumberRowsReported(uint16_t value)
{
  setHeaderWord<uint16_t>(headerNumberRowsReportedOffset, value);
}

void ISmurfPacket::setRowLength(uint16_t value)
{
  setHeaderWord<uint16_t>(headerRowLengthOffset, value);
}

void ISmurfPacket::setDataRate(uint16_t value)
{
  setHeaderWord<uint16_t>(headerDataRateOffset, value);
}


void ISmurfPacket::setHeaderByte(std::size_t index, uint8_t value)
{
  headerBuffer.at(index) = value;
}

void ISmurfPacket::setValue(std::size_t index, avgdata_t value)
{
  payloadBuffer.at(index) = value;
}

template <typename T>
void ISmurfPacket::setHeaderWord(std::size_t offset, const T& value)
{
  *(reinterpret_cast<T*>(&headerBuffer.at(offset))) = value;
}

uint8_t ISmurfPacket::setWordBit(uint8_t byte, std::size_t index, bool value)
{
  if (index >= 8)
    throw std::runtime_error("Trying to set a byte bit out range.");

  if (value)
    byte |= (0x01 << index);
  else
    byte &= ~(0x01 << index);

  return byte;
}

SmurfPacket ISmurfPacket::create()
{
  return std::make_shared<ISmurfPacket>();
}

SmurfPacket ISmurfPacket::create(uint8_t* h)
{
  return std::make_shared<ISmurfPacket>(h);
}

SmurfPacket ISmurfPacket::create(uint8_t* h, avgdata_t* d)
{
  return std::make_shared<ISmurfPacket>(h,d);
}

////////////////////////////////////////
////// - ISmurfPacket definitions ///////
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