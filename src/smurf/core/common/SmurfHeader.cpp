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


SmurfHeaderRO::SmurfHeaderRO(ris::FrameIterator it)
:
    headerIt(it)
{
}

SmurfHeaderROPtr SmurfHeaderRO::create(ris::FrameIterator it)
{
    return std::make_shared<SmurfHeaderRO>(it);
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


const uint32_t SmurfHeaderRO::getFrameCounter() const
{
    return getU32Word(headerFrameCounterOffset);
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

const const bool SmurfHeaderRO::getWordBit(std::size_t offset, std::size_t index) const
{
    if (index >= 8)
        throw std::runtime_error("Trying to get a bit with index > 8 from a byte");

    return ( (*(headerIt+offset) >> index ) & 0x01 );
}