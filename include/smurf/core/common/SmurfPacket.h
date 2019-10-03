#ifndef _SMURF_CORE_COMMON_SMURFPACKET_H_
#define _SMURF_CORE_COMMON_SMURFPACKET_H_

/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Packet
 * ----------------------------------------------------------------------------
 * File          : SmurfPacket.h
 * Created       : 2019-10-01
 *-----------------------------------------------------------------------------
 * Description :
 *    SMuRF Packet Class.
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
#include <stdexcept>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameLock.h>
#include <rogue/interfaces/stream/FrameIterator.h>
#include "smurf/core/common/SmurfHeader.h"

namespace ris = rogue::interfaces::stream;

class SmurfPacketRawRO;
class SmurfPacketRaw;
typedef std::shared_ptr<SmurfPacketRawRO> SmurfPacketRawROPtr;
typedef std::shared_ptr<SmurfPacketRaw> SmurfPacketRawPtr;


// SMuRF packet class.
// This class handle raw packet, as they come from FW.
// This class give read-only access.
class SmurfPacketRawRO
{
public:
    SmurfPacketRawRO(ris::FramePtr frame);
    ~SmurfPacketRawRO() {};

    // Factory method, to create a smart pointer.
    static SmurfPacketRawROPtr create(ris::FramePtr frame);

    // Data type
    typedef int16_t data_t;

    // Size in bytes of a data word
    static const std::size_t DataWordSize  = sizeof(data_t);

    // Get a data point
    const data_t getDataWord(std::size_t offset) const;

private:
    // Prevent construction using the default or copy constructor.
    // Prevent an SmurfPacketRawRO object to be assigned as well.
    SmurfPacketRawRO();
    SmurfPacketRawRO(const SmurfPacketRawRO&);
    SmurfPacketRawRO& operator=(const SmurfPacketRawRO&);

    // Private data members
    ris::FrameIterator dataIt;  // Iterator to the start of the header in a Frame};
};

// SMuRF packet class.
// This class handle raw packet, as they come from FW.
// This class give read-write access.
class SmurfPacketRaw : public SmurfPacketRawRO
{
public:
    SmurfPacketRaw(ris::FramePtr frame);
    ~SmurfPacketRaw() {};

    // Factory method, to create a smart pointer.
    static SmurfPacketRawPtr create(ris::FramePtr frame);

    // Get a data point
    void setDataWord(std::size_t offset, data_t value) const;

private:
    // Prevent construction using the default or copy constructor.
    // Prevent an SmurfPacketRawRO object to be assigned as well.
    SmurfPacketRaw();
    SmurfPacketRaw(const SmurfPacketRaw&);
    SmurfPacketRaw& operator=(const SmurfPacketRaw&);

    // Private data members
    ris::FrameIterator dataIt;  // Iterator to the start of the header in a Frame};
};



// SMuRF packet class.
// This class handle SW processed packets
class SmurfPacket
{
public:
    // Smurf data type
    typedef int32_t smurf_data_t;

    // Size in bytes of a SMuRF data word
    static const std::size_t SmurfDataWordSize  = sizeof(smurf_data_t);

    // The length of the smurf packet payload (in number of channels)
    // It has public access.
    static const std::size_t SmurfPacketPayloadLength = 528;

    // The size of the SMuRF packet payload in bytes
    static const std::size_t SmurfPacketPayloadSize = SmurfPacketPayloadLength * SmurfDataWordSize;

    // The total size, in bytes, of the SMuRF packet (i.e., including the header)
    static const std::size_t SmurfPacketSize = SmurfHeader::SmurfHeaderSize + SmurfPacketPayloadSize;
};

#endif