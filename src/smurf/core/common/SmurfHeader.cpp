/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Header
 * ----------------------------------------------------------------------------
 * File          : SmurfHeader.cpp
 * Created       : 2019-10-01
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Header Class.
 *-----------------------------------------------------------------------------
 * This file is part of the smurf software platform. It is subject to
 * the license terms in the LICENSE.txt file found in the top-level directory
 * of this distribution and at:
    * https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
 * No part of the smurf software platform, including this file, may be
 * copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE.txt file.
 *-----------------------------------------------------------------------------
**/

#include "smurf/core/common/SmurfHeader.h"

//////////////////////////////////////////
////// + SmurfHeaderRO definitions ///////
//////////////////////////////////////////

SmurfHeaderRO::SmurfHeaderRO(ris::FramePtr frame)
:
    headerIt(frame->beginRead()),
    tba(TesBiasArray::create(headerIt + headerTESDACOffset))
{
}

SmurfHeaderROPtr SmurfHeaderRO::create(ris::FramePtr frame)
{
    return std::make_shared<SmurfHeaderRO>(frame);
}

// Function to get header words
const uint8_t SmurfHeaderRO::getVersion() const
{
    return getU8Word(headerVersionOffset);
}

const uint8_t SmurfHeaderRO::getCrateID() const
{
    return getU8Word(headerCrateIDOffset);
}

const uint8_t SmurfHeaderRO::getSlotNumber() const
{
    return getU8Word(headerSlotNumberOffset);
}

const uint8_t SmurfHeaderRO::getTimingConfiguration() const
{
    return getU8Word(headerTimingConfigurationOffset);
}

const uint32_t SmurfHeaderRO::getNumberChannels() const
{
    return getU32Word(headerNumberChannelOffset);
}

const int32_t SmurfHeaderRO::getTESBias(std::size_t index) const
{
    return tba->getWord(index);
}

const uint64_t SmurfHeaderRO::getUnixTime() const
{
    return getU64Word(headerUnixTimeOffset);
}

const uint32_t SmurfHeaderRO::getFluxRampIncrement() const
{
    return getU32Word(headerFluxRampIncrementOffset);
}

const uint32_t SmurfHeaderRO::getFluxRampOffset() const
{
    return getU32Word(headerFluxRampOffsetOffset);
}

const uint32_t SmurfHeaderRO::getCounter0() const
{
    return getU32Word(headerCounter0Offset);
}

const uint32_t SmurfHeaderRO::getCounter1() const
{
    return getU32Word(headerCounter1Offset);
}

const uint64_t SmurfHeaderRO::getCounter2() const
{
    return getU64Word(headerCounter2Offset);
}

const uint32_t SmurfHeaderRO::getAveragingResetBits() const
{
    return getU32Word(headerAveragingResetBitsOffset);
}

const uint32_t SmurfHeaderRO::getFrameCounter() const
{
    return getU32Word(headerFrameCounterOffset);
}

const uint32_t SmurfHeaderRO::getTESRelaySetting() const
{
    return getU32Word(headerTESRelaySettingOffset);
}

const uint64_t SmurfHeaderRO::getExternalTimeClock() const
{
    return getU64Word(headerExternalTimeClockOffset);
}

const uint8_t SmurfHeaderRO::getControlField() const
{
    return getU8Word(headerControlFieldOffset);
}

const bool SmurfHeaderRO::getClearAverageBit() const
{
    return getWordBit(headerControlFieldOffset, clearAvergaveBitOffset);
}

const bool SmurfHeaderRO::getDisableStreamBit() const
{
    return getWordBit(headerControlFieldOffset, disableStreamBitOffset);
}

const bool SmurfHeaderRO::getDisableFileWriteBit() const
{
    return getWordBit(headerControlFieldOffset, disableFileWriteBitOffset);
}

const bool SmurfHeaderRO::getReadConfigEachCycleBit() const
{
    return getWordBit(headerControlFieldOffset, readConfigEachCycleBitOffset);
}

const uint8_t SmurfHeaderRO::getTestMode() const
{
    return ( ( getU8Word(headerControlFieldOffset) >> 4 ) & 0x0f );
}

const uint8_t SmurfHeaderRO::getTestParameters() const
{
    return getU8Word(headerTestParametersOffset);
}

const uint16_t SmurfHeaderRO::getNumberRows() const
{
    return getU16Word(headerNumberRowsOffset);
}

const uint16_t SmurfHeaderRO::getNumberRowsReported() const
{
    return getU16Word(headerNumberRowsReportedOffset);
}

const uint16_t SmurfHeaderRO::getRowLength() const
{
    return getU16Word(headerRowLengthOffset);
}

const uint16_t SmurfHeaderRO::getDataRate() const
{
    return getU16Word(headerDataRateOffset);
}

// Helper functions
const uint8_t SmurfHeaderRO::getU8Word(std::size_t offset) const
{
    return *(headerIt+offset);
}

const uint16_t SmurfHeaderRO::getU16Word(std::size_t offset) const
{
    union
    {
        uint16_t w;
        uint8_t  b[2];
    } aux;

    for (std::size_t i{0}; i < 2; ++i)
        aux.b[i] = *(headerIt+offset+i);

    return aux.w;
}

const uint32_t SmurfHeaderRO::getU32Word(std::size_t offset) const
{
    union
    {
        uint32_t w;
        uint8_t  b[4];
    } aux;

    for (std::size_t i{0}; i < 4; ++i)
        aux.b[i] = *(headerIt+offset+i);

    return aux.w;
}

const uint64_t SmurfHeaderRO::getU64Word(std::size_t offset) const
{
    union
    {
        uint64_t w;
        uint8_t  b[8];
    } aux;

    for (std::size_t i{0}; i < 8; ++i)
        aux.b[i] = *(headerIt+offset+i);

    return aux.w;
}

const bool SmurfHeaderRO::getWordBit(std::size_t offset, std::size_t index) const
{
    if (index >= 8)
        throw std::runtime_error("Trying to get a byte's bit with index > 8");

    return ( (*(headerIt+offset) >> index ) & 0x01 );
}


//////////////////////////////////////////
////// - SmurfHeaderRO definitions ///////
//////////////////////////////////////////


////////////////////////////////////////
////// + SmurfHeader definitions ///////
////////////////////////////////////////

SmurfHeader::SmurfHeader(ris::FramePtr frame)
:
    SmurfHeaderRO(frame),
    headerIt(frame->beginWrite()),
    tba(TesBiasArray::create(headerIt + headerTESDACOffset))
{

}

SmurfHeaderPtr SmurfHeader::create(ris::FramePtr frame)
{
    return std::make_shared<SmurfHeader>(frame);
}

// Function to get header words
void SmurfHeader::setVersion(uint8_t value) const
{
    setU8Word(headerVersionOffset, value);
}

void SmurfHeader::setCrateID(uint8_t value) const
{
    setU8Word(headerCrateIDOffset, value);
}

void SmurfHeader::setSlotNumber(uint8_t value) const
{
    setU8Word(headerSlotNumberOffset, value);
}

void SmurfHeader::setTimingConfiguration(uint8_t value) const
{
    setU8Word(headerTimingConfigurationOffset, value);
}

void SmurfHeader::setNumberChannels(uint32_t value) const
{
    setU32Word(headerNumberChannelOffset, value);
}

void SmurfHeader::setTESBias(std::size_t index, int32_t value) const
{
    tba->setWord(index, value);
}

void SmurfHeader::setUnixTime(uint64_t value) const
{
    setU64Word(headerUnixTimeOffset, value);
}

void SmurfHeader::setFluxRampIncrement(uint32_t value) const
{
    setU32Word(headerFluxRampIncrementOffset, value);
}

void SmurfHeader::setFluxRampOffset(uint32_t value) const
{
    setU32Word(headerFluxRampOffsetOffset, value);
}

void SmurfHeader::setCounter0(uint32_t value) const
{
    setU32Word(headerCounter0Offset, value);
}

void SmurfHeader::setCounter1(uint32_t value) const
{
    setU32Word(headerCounter1Offset, value);
}

void SmurfHeader::setCounter2(uint64_t value) const
{
    setU64Word(headerCounter2Offset, value);
}

void SmurfHeader::setAveragingResetBits(uint32_t value) const
{
    setU32Word(headerAveragingResetBitsOffset, value);
}

void SmurfHeader::setFrameCounter(uint32_t value) const
{
    setU32Word(headerFrameCounterOffset, value);
}

void SmurfHeader::setTESRelaySetting(uint32_t value) const
{
    setU32Word(headerTESRelaySettingOffset, value);
}

void SmurfHeader::setExternalTimeClock(uint64_t value) const
{
    setU64Word(headerExternalTimeClockOffset, value);
}

void SmurfHeader::setControlField(uint8_t value) const
{
    setU8Word(headerControlFieldOffset, value);
}

void SmurfHeader::setClearAverageBit(bool value) const
{
    setWordBit(headerControlFieldOffset, value, clearAvergaveBitOffset);
}

void SmurfHeader::setDisableStreamBit(bool value) const
{
    setWordBit(headerControlFieldOffset, value, disableStreamBitOffset);
}

void SmurfHeader::setDisableFileWriteBit(bool value) const
{
    setWordBit(headerControlFieldOffset, value, disableFileWriteBitOffset);
}

void SmurfHeader::setReadConfigEachCycleBit(bool value) const
{
    setWordBit(headerControlFieldOffset, value, readConfigEachCycleBitOffset);
}

void SmurfHeader::setTestMode(uint8_t value) const
{
    uint8_t u8 = getControlField();

    u8 &= 0x0f;
    u8 |= ( (value << 4 ) & 0xf0 );

    setU8Word(headerControlFieldOffset, u8);
}

void SmurfHeader::setTestParameters(uint8_t value) const
{
    setU8Word(headerTestParametersOffset, value);
}

void SmurfHeader::setNumberRows(uint16_t value) const
{
    setU16Word(headerNumberRowsOffset, value);
}

void SmurfHeader::setNumberRowsReported(uint16_t value) const
{
    setU16Word(headerNumberRowsReportedOffset, value);
}

void SmurfHeader::setRowLength(uint16_t value) const
{
    setU16Word(headerRowLengthOffset, value);
}

void SmurfHeader::setDataRate(uint16_t value) const
{
    setU16Word(headerDataRateOffset, value);
}

// Helper functions
void SmurfHeader::setU8Word(std::size_t offset, uint8_t value) const
{
    *(headerIt+offset) = value;
}

void SmurfHeader::setU16Word(std::size_t offset, uint16_t value) const
{
    union
    {
        uint16_t w;
        uint8_t  b[2];
    } aux;

    aux.w  = value;

    for (std::size_t i{0}; i < 2; ++i)
        *(headerIt+offset+i) = aux.b[i];
}

void SmurfHeader::setU32Word(std::size_t offset, uint32_t value) const
{
    union
    {
        uint32_t w;
        uint8_t  b[4];
    } aux;

    aux.w = value;

    for (std::size_t i{0}; i < 4; ++i)
        *(headerIt+offset+i) = aux.b[i];
}

void SmurfHeader::setU64Word(std::size_t offset, uint64_t value) const
{
    union
    {
        uint64_t w;
        uint8_t  b[8];
    } aux;

    aux.w = value;

    for (std::size_t i{0}; i < 8; ++i)
        *(headerIt+offset+i) = aux.b[i];
}

void SmurfHeader::setWordBit(std::size_t offset, std::size_t index, bool value) const
{
    if (index >= 8)
        throw std::runtime_error("Trying to set a byte's bit with index > 8");

    uint8_t aux = *(headerIt+offset);

    if (value)
        aux |= (0x01 << index);
    else
        aux &= ~(0x01 << index);

    *(headerIt+offset) = aux;
}

////////////////////////////////////////
////// - SmurfHeader definitions ///////
////////////////////////////////////////