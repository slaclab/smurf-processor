#include "tes_bias_array.h"

TesBiasArray::TesBiasArray(uint8_t *p)
:
  pData(p)
{

}

TesBiasArray::~TesBiasArray()
{

}

void TesBiasArray::setPtr(uint8_t *p)
{
  pData = p;
}

void TesBiasArray::setWord(const WordIndex& index, int32_t value) const
{
  if (index >= TesBiasCount)
    throw std::runtime_error("Trying to write a TES bias value in an address of out the buffer range.");

  if (index.Word() == 0)
  {
    // Create an union pointing to the block
    uint8_t *b = (pData + 5*index.Block());

    // Create an union with the passed value
    U v { value };
    *b++ = v.byte[0];
    *b++ = v.byte[1];
    uint8_t temp = *b;
    temp &= 0xf0;
    temp |= (v.byte[2] & 0x0f);
    *b = temp;
  }
  else
  {
    // Create an union pointing to the block
    uint8_t *b = (pData + 5*index.Block() + 2);

    // Create an union with the passed value
    U v { value << 4 };
    uint8_t temp = *b;
    temp &= 0x0f;
    temp |= (v.byte[0] & 0xf0);
    *b++ = temp;
    *b++ = v.byte[1];
    *b = v.byte[2];
  }
};

const int32_t TesBiasArray::getWord(const WordIndex& index) const
{
  if (index >= TesBiasCount)
    throw std::runtime_error("Trying to read a TES bias value in an address of out the buffer range.");

  std::size_t offset(0), shift(0);

  if (index.Word())
  {
    offset = 2;
    shift = 4;
  }

  return S { static_cast<int>( *( reinterpret_cast<uint32_t*>( pData + 5*index.Block() + offset ) ) >> shift )  }.word;
}

std::mutex* TesBiasArray::getMutex()
{
  return &mut;
};