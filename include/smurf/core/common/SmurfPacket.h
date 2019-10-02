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

#include "smurf/core/common/SmurfHeader.h"

class SmurfPacket
{
public:
	// Smurf data type
    typedef uint32_t smurf_data_t;

    // Size in bytes of a SMuRF data word
    static const std::size_t SmurfDataWordSize  = sizeof(smurf_data_t);

	// The length of the smurf packet payload (in number of channels)
    // It has public access.
    static const std::size_t SmurfPacketPayloadLength = 528;

    // The size of the SMuRF packet payload in bytes
	static const std::size_t SmurfPacketPayloadSize = SmurfPacketPayloadLength * SmurfDataWordSize;

	// The total size, in bytes, of the SMuRF packet (i.e., including the header)
	static const std::size_t SmurfPacketSize = SmurfHeader::SmurfHeaderLength + SmurfPacketPayloadSize;
};

#endif