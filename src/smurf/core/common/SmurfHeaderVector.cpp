/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Header (on a std::vector)
 * ----------------------------------------------------------------------------
 * File          : SmurfHeaderVector.cpp
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
 *-----
**/

 #include "smurf/core/common/SmurfHeaderVector.h"

////////////////////////////////////////////////
////// + SmurfHeaderVectorRO definitions ///////
////////////////////////////////////////////////

SmurfHeaderVectorRO::SmurfHeaderVectorRO(std::vector<uint8_t>& buffer)
:
    headerIt(buffer.begin())
{
}

SmurfHeaderVectorROPtr SmurfHeaderVectorRO::create(std::vector<uint8_t>& buffer)
{
    return std::make_shared<SmurfHeaderVectorRO>(buffer);
}

// Function to get header words
const uint8_t SmurfHeaderVectorRO::getVersion() const
{
    return getU8Word(headerVersionOffset);
}

const uint8_t SmurfHeaderVectorRO::getCrateID() const
{
    return getU8Word(headerCrateIDOffset);
}

const uint8_t SmurfHeaderVectorRO::getSlotNumber() const
{
    return getU8Word(headerSlotNumberOffset);
}

const uint8_t SmurfHeaderVectorRO::getTimingConfiguration() const
{
    return getU8Word(headerTimingConfigurationOffset);
}

const uint32_t SmurfHeaderVectorRO::getNumberChannels() const
{
    return getU32Word(headerNumberChannelOffset);
}

const int32_t SmurfHeaderVectorRO::getTESBias(std::size_t index) const
{
    // return tba->getWord(index);
}

void SmurfHeaderVectorRO::copyTESBiasArrayTo(std::vector<uint8_t>::iterator it) const
{
    std::copy(it + headerTESDACOffset,
        it + headerTESDACOffset + TesBiasArray::TesBiasBufferSize,
        headerIt);
}

const uint64_t SmurfHeaderVectorRO::getUnixTime() const
{
    return getU64Word(headerUnixTimeOffset);
}

const uint32_t SmurfHeaderVectorRO::getFluxRampIncrement() const
{
    return getU32Word(headerFluxRampIncrementOffset);
}

const uint32_t SmurfHeaderVectorRO::getFluxRampOffset() const
{
    return getU32Word(headerFluxRampOffsetOffset);
}

const uint32_t SmurfHeaderVectorRO::getCounter0() const
{
    return getU32Word(headerCounter0Offset);
}

const uint32_t SmurfHeaderVectorRO::getCounter1() const
{
    return getU32Word(headerCounter1Offset);
}

const uint64_t SmurfHeaderVectorRO::getCounter2() const
{
    return getU64Word(headerCounter2Offset);
}

const uint32_t SmurfHeaderVectorRO::getAveragingResetBits() const
{
    return getU32Word(headerAveragingResetBitsOffset);
}

const uint32_t SmurfHeaderVectorRO::getFrameCounter() const
{
    return getU32Word(headerFrameCounterOffset);
}

const uint32_t SmurfHeaderVectorRO::getTESRelaySetting() const
{
    return getU32Word(headerTESRelaySettingOffset);
}

const uint64_t SmurfHeaderVectorRO::getExternalTimeClock() const
{
    return getU64Word(headerExternalTimeClockOffset);
}

const uint8_t SmurfHeaderVectorRO::getControlField() const
{
    return getU8Word(headerControlFieldOffset);
}

const bool SmurfHeaderVectorRO::getClearAverageBit() const
{
    return getWordBit(headerControlFieldOffset, clearAvergaveBitOffset);
}

const bool SmurfHeaderVectorRO::getDisableStreamBit() const
{
    return getWordBit(headerControlFieldOffset, disableStreamBitOffset);
}

const bool SmurfHeaderVectorRO::getDisableFileWriteBit() const
{
    return getWordBit(headerControlFieldOffset, disableFileWriteBitOffset);
}

const bool SmurfHeaderVectorRO::getReadConfigEachCycleBit() const
{
    return getWordBit(headerControlFieldOffset, readConfigEachCycleBitOffset);
}

const uint8_t SmurfHeaderVectorRO::getTestMode() const
{
    return ( ( getU8Word(headerControlFieldOffset) >> 4 ) & 0x0f );
}

const uint8_t SmurfHeaderVectorRO::getTestParameters() const
{
    return getU8Word(headerTestParametersOffset);
}

const uint16_t SmurfHeaderVectorRO::getNumberRows() const
{
    return getU16Word(headerNumberRowsOffset);
}

const uint16_t SmurfHeaderVectorRO::getNumberRowsReported() const
{
    return getU16Word(headerNumberRowsReportedOffset);
}

const uint16_t SmurfHeaderVectorRO::getRowLength() const
{
    return getU16Word(headerRowLengthOffset);
}

const uint16_t SmurfHeaderVectorRO::getDataRate() const
{
    return getU16Word(headerDataRateOffset);
}

// Helper functions
const uint8_t SmurfHeaderVectorRO::getU8Word(std::size_t offset) const
{
    return *(headerIt+offset);
}

const uint16_t SmurfHeaderVectorRO::getU16Word(std::size_t offset) const
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

const uint32_t SmurfHeaderVectorRO::getU32Word(std::size_t offset) const
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

const uint64_t SmurfHeaderVectorRO::getU64Word(std::size_t offset) const
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

const bool SmurfHeaderVectorRO::getWordBit(std::size_t offset, std::size_t index) const
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

SmurfHeaderVector::SmurfHeaderVector(std::vector<uint8_t>& buffer)
:
    SmurfHeaderVectorRO(buffer),
    headerIt(buffer.begin())
{
}

SmurfHeaderVectorPtr SmurfHeaderVector::create(std::vector<uint8_t>& buffer)
{
    return std::make_shared<SmurfHeaderVector>(buffer);
}

// Function to get header words
void SmurfHeaderVector::setVersion(uint8_t value) const
{
    setU8Word(headerVersionOffset, value);
}

void SmurfHeaderVector::setCrateID(uint8_t value) const
{
    setU8Word(headerCrateIDOffset, value);
}

void SmurfHeaderVector::setSlotNumber(uint8_t value) const
{
    setU8Word(headerSlotNumberOffset, value);
}

void SmurfHeaderVector::setTimingConfiguration(uint8_t value) const
{
    setU8Word(headerTimingConfigurationOffset, value);
}

void SmurfHeaderVector::setNumberChannels(uint32_t value) const
{
    setU32Word(headerNumberChannelOffset, value);
}

void SmurfHeaderVector::setTESBias(std::size_t index, int32_t value) const
{
    // tba->setWord(index, value);
}

void SmurfHeaderVector::copyTESBiasArrayFrom(ris::FrameIterator it) const
{
    std::copy(headerIt + headerTESDACOffset,
        headerIt + headerTESDACOffset + TesBiasArray::TesBiasBufferSize,
        it);
}

void SmurfHeaderVector::setUnixTime(uint64_t value) const
{
    setU64Word(headerUnixTimeOffset, value);
}

void SmurfHeaderVector::setFluxRampIncrement(uint32_t value) const
{
    setU32Word(headerFluxRampIncrementOffset, value);
}

void SmurfHeaderVector::setFluxRampOffset(uint32_t value) const
{
    setU32Word(headerFluxRampOffsetOffset, value);
}

void SmurfHeaderVector::setCounter0(uint32_t value) const
{
    setU32Word(headerCounter0Offset, value);
}

void SmurfHeaderVector::setCounter1(uint32_t value) const
{
    setU32Word(headerCounter1Offset, value);
}

void SmurfHeaderVector::setCounter2(uint64_t value) const
{
    setU64Word(headerCounter2Offset, value);
}

void SmurfHeaderVector::setAveragingResetBits(uint32_t value) const
{
    setU32Word(headerAveragingResetBitsOffset, value);
}

void SmurfHeaderVector::setFrameCounter(uint32_t value) const
{
    setU32Word(headerFrameCounterOffset, value);
}

void SmurfHeaderVector::setTESRelaySetting(uint32_t value) const
{
    setU32Word(headerTESRelaySettingOffset, value);
}

void SmurfHeaderVector::setExternalTimeClock(uint64_t value) const
{
    setU64Word(headerExternalTimeClockOffset, value);
}

void SmurfHeaderVector::setControlField(uint8_t value) const
{
    setU8Word(headerControlFieldOffset, value);
}

void SmurfHeaderVector::setClearAverageBit(bool value) const
{
    setWordBit(headerControlFieldOffset, value, clearAvergaveBitOffset);
}

void SmurfHeaderVector::setDisableStreamBit(bool value) const
{
    setWordBit(headerControlFieldOffset, value, disableStreamBitOffset);
}

void SmurfHeaderVector::setDisableFileWriteBit(bool value) const
{
    setWordBit(headerControlFieldOffset, value, disableFileWriteBitOffset);
}

void SmurfHeaderVector::setReadConfigEachCycleBit(bool value) const
{
    setWordBit(headerControlFieldOffset, value, readConfigEachCycleBitOffset);
}

void SmurfHeaderVector::setTestMode(uint8_t value) const
{
    uint8_t u8 = getControlField();

    u8 &= 0x0f;
    u8 |= ( (value << 4 ) & 0xf0 );

    setU8Word(headerControlFieldOffset, u8);
}

void SmurfHeaderVector::setTestParameters(uint8_t value) const
{
    setU8Word(headerTestParametersOffset, value);
}

void SmurfHeaderVector::setNumberRows(uint16_t value) const
{
    setU16Word(headerNumberRowsOffset, value);
}

void SmurfHeaderVector::setNumberRowsReported(uint16_t value) const
{
    setU16Word(headerNumberRowsReportedOffset, value);
}

void SmurfHeaderVector::setRowLength(uint16_t value) const
{
    setU16Word(headerRowLengthOffset, value);
}

void SmurfHeaderVector::setDataRate(uint16_t value) const
{
    setU16Word(headerDataRateOffset, value);
}

// Helper functions
void SmurfHeaderVector::setU8Word(std::size_t offset, uint8_t value) const
{
    *(headerIt+offset) = value;
}

void SmurfHeaderVector::setU16Word(std::size_t offset, uint16_t value) const
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

void SmurfHeaderVector::setU32Word(std::size_t offset, uint32_t value) const
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

void SmurfHeaderVector::setU64Word(std::size_t offset, uint64_t value) const
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

void SmurfHeaderVector::setWordBit(std::size_t offset, std::size_t index, bool value) const
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
