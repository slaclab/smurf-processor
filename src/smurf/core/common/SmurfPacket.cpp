/**
 *-----------------------------------------------------------------------------
 * Title         : SMuRF Packet
 * ----------------------------------------------------------------------------
 * File          : SmurfPacket.cpp
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
#include "smurf/core/common/SmurfPacket.h"

/////////////////////////////////////////////
////// + SmurfPacketRawRO definitions ///////
/////////////////////////////////////////////

SmurfPacketRawRO::SmurfPacketRawRO(ris::FramePtr frame)
:
    dataIt(frame->beginRead() + SmurfHeader::SmurfHeaderSize)
{
}

SmurfPacketRawROPtr SmurfPacketRawRO::create(ris::FramePtr frame)
{
    return std::make_shared<SmurfPacketRawRO>(frame);
}

const SmurfPacketRaw::data_t SmurfPacketRawRO::getDataWord(std::size_t offset) const
{
    // Unfortunately this code, is not parametric with the user-define
    // data type 'data_t'. So, it will need to be update if the definition
    // of 'data_t' changes.
    union
    {
        uint16_t w;
        uint8_t  b[DataWordSize];
    } aux;

    for (std::size_t i{0}; i < DataWordSize; ++i)
        aux.b[i] = *(dataIt + offset * DataWordSize + i);

    return static_cast<int16_t>(aux.w);
}

/////////////////////////////////////////////
////// - SmurfPacketRawRO definitions ///////
/////////////////////////////////////////////

///////////////////////////////////////////
////// + SmurfPacketRaw definitions ///////
///////////////////////////////////////////

SmurfPacketRaw::SmurfPacketRaw(ris::FramePtr frame)
:
    SmurfPacketRawRO(frame),
    dataIt(frame->beginWrite() + SmurfHeader::SmurfHeaderSize)
{
}

SmurfPacketRawPtr SmurfPacketRaw::create(ris::FramePtr frame)
{
    return std::make_shared<SmurfPacketRaw>(frame);
}

void SmurfPacketRaw::setDataWord(std::size_t offset, data_t value) const
{
    union
    {
        uint16_t w;
        uint8_t  b[DataWordSize];
    } aux;

    aux.w = static_cast<uint16_t>(value);

    for (std::size_t i{0}; i < DataWordSize; ++i)
        *(dataIt + offset * DataWordSize + i) = aux.b[i];
}

///////////////////////////////////////////
////// - SmurfPacketRaw definitions ///////
///////////////////////////////////////////

//////////////////////////////////////////
////// + SmurfPacketRO definitions ///////
//////////////////////////////////////////

SmurfPacketRO::SmurfPacketRO(ris::FramePtr frame)
:
    dataIt(frame->beginRead() + SmurfHeader::SmurfHeaderSize)
{
}

SmurfPacketROPtr SmurfPacketRO::create(ris::FramePtr frame)
{
    return std::make_shared<SmurfPacketRO>(frame);
}

const SmurfPacket::data_t SmurfPacketRO::getDataWord(std::size_t offset) const
{
    union
    {
        uint16_t w;
        uint8_t  b[DataWordSize];
    } aux;

    for (std::size_t i{0}; i < DataWordSize; ++i)
        aux.b[i] = *(dataIt + offset * DataWordSize + i);

    return static_cast<int16_t>(aux.w);
}

//////////////////////////////////////////
////// - SmurfPacketRO definitions ///////
//////////////////////////////////////////

////////////////////////////////////////
////// + SmurfPacket definitions ///////
////////////////////////////////////////

SmurfPacket::SmurfPacket(ris::FramePtr frame)
:
    SmurfPacketRO(frame),
    dataIt(frame->beginWrite() + SmurfHeader::SmurfHeaderSize)
{
}

SmurfPacketPtr SmurfPacket::create(ris::FramePtr frame)
{
    return std::make_shared<SmurfPacket>(frame);
}

void SmurfPacket::setDataWord(std::size_t offset, data_t value) const
{
    union
    {
        uint16_t w;
        uint8_t  b[DataWordSize];
    } aux;

    aux.w = static_cast<uint16_t>(value);

    for (std::size_t i{0}; i < DataWordSize; ++i)
        *(dataIt + offset * DataWordSize + i) = aux.b[i];
}

////////////////////////////////////////
////// - SmurfPacket definitions ///////
////////////////////////////////////////