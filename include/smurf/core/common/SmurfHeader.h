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

class SmurfHeaderRO
typedef std::shared_ptr<SmurfHeaderRO> SmurfHeaderROPtr;

class SmurfHeaderRO
{
public:
    SmurfHeaderRO(ris::FrameIterator it);
    ~SmurfHeaderRO() {};

    static SmurfHeaderROPtr create(ris::FrameIterator it);

    const uint32_t getFrameCounter()              const;  // Get locally generate frame counter 32 bit

protected:
    static const std::size_t headerFrameCounterOffset         = 84;

private:
    // Prevent construction using the default or copy constructor.
    // Prevent an SmurfHeaderRO object to be assigned as well.
    SmurfHeaderRO();
    SmurfHeaderRO(const SmurfHeaderRO&);
    SmurfHeaderRO& operator=(const SmurfHeaderRO&);

    // Helper unions definitions
    union U32
    {
        uint32_t w;
        uint8_t  b[4];
    };

    ris::FrameIterator headerIt;  // Iterator to the start of the header in a Frame
};

#endif