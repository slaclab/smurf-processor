#ifndef _SMURF_CORE_COMMON_SMURFHEADER_H_
#define _SMURF_CORE_COMMON_SMURFHEADER_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Header
 * ----------------------------------------------------------------------------
 * File          : SmurfHeader.h
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

#include <memory>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameLock.h>
#include <rogue/interfaces/stream/FrameIterator.h>

namespace ris = rogue::interfaces::stream;

class SmurfHeaderRO;
typedef std::shared_ptr<SmurfHeaderRO> SmurfHeaderROPtr;

class SmurfHeaderRO
{
public:
    SmurfHeaderRO(ris::FrameIterator it);
    ~SmurfHeaderRO() {};

    static SmurfHeaderROPtr create(ris::FrameIterator it);

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

protected:
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
    // Prevent construction using the default or copy constructor.
    // Prevent an SmurfHeaderRO object to be assigned as well.
    SmurfHeaderRO();
    SmurfHeaderRO(const SmurfHeaderRO&);
    SmurfHeaderRO& operator=(const SmurfHeaderRO&);

    // helper functions
    const uint16_t getU32Word(std::size_t offset) const;
    const uint32_t getU32Word(std::size_t offset) const;
    const uint64_t getU32Word(std::size_t offset) const;

    // Private variables
    ris::FrameIterator headerIt;  // Iterator to the start of the header in a Frame
};

#endif