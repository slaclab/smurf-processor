#ifndef __TES_BIAS_ARRAY_H__
#define __TES_BIAS_ARRAY_H__

#include <stdexcept>
#include <mutex>

static const std::size_t TesBiasCount = 16;  // 16 Tes Bias values
static const std::size_t TesBiasBufferSize = TesBiasCount * 20 / 8; // 16x 20-bit bytes

// Class to handle TES bias array of values.
class TesBiasArray
{
private:
  // Pointer to the data buffer
  uint8_t *pData;

  // Mutex, to safetly access the data from different threads
  std::mutex mut;

  // Helper class to handler indexes of TES bias words
  // TES bias are 20-bit words = 2.5 bytes
  // 16x TES bias occupy 40 bytes, which are divided into 8 blocks of 2 words (5 bytes) each
  // From each word index (0-15) a block number (0-7), and a word sub-index inside that block (0-1)
  // is generated. For example, TES bias 6 and 7 are both located on block 3; with 6 at the first word
  // and 7 on the second word inside that block.
  class WordIndex
  {
  public:
    WordIndex(std::size_t i) : index( i ), block( i / 2 ), word( i % 2 ) {};

    std::size_t Index() const { return index; }; // 20-bit word index
    std::size_t Block() const { return block; }; // 2-word block number
    std::size_t Word()  const { return word;  }; // Word index inside the block

    bool operator >= (std::size_t rhs) const { return index >= rhs; };

  private:
    std::size_t index; // 20-bit word index (0-15)
    std::size_t block; // 2-word block index (0-7)
    std::size_t word;  // Word index in the word block (0-1)
  };

  // Helper Union to access individual bytes of a word
  typedef union
  {
    unsigned int word;
    uint8_t byte[4];
  } U;

  // Helper Struct to sign extend 20-bit values
  typedef struct
  {
    signed int word:20;
  } S;

public:
 TesBiasArray(uint8_t *p);
  ~TesBiasArray();

  // Change the data pointer
  void setPtr(uint8_t *p);

  // Write a TES bias value
  void setWord(const WordIndex& index, int value) const;

  // Read a TES bias value
  int getWord(const WordIndex& index) const;

  // Method to the mutex
  std::mutex* getMutex();
};

#endif