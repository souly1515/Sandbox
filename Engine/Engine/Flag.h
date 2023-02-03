#pragma once

#include "Includes/Defines.h"

const uint64_t Mask_64Bit = 0xFFFFFFFF;

template<uint32_t Size = 32>
class Flag
{
  static_assert(!(Size % 32));
public:
  static constexpr uint32_t arraySize = Size / 32;

private:
  uint64_t data[arraySize];

public:
  Flag(uint64_t flag = 0)
  {
    data[0] = flag;
  }
  Flag(uint64_t flags[arraySize])
  {
    for(uint32_t i = 0; i < arraySize; ++i)
      data[i] = flags[i];
  }
  Flag(const Flag& flag)
  {
    for(uint32_t i = 0; i < arraySize; ++i)
      data[i] = flag.data[i];
  }
  void SetBit(uint32_t bit)
  {
    data[bit / 64] |= (bit << (bit & 0x3F));
  }
  void FlipBit(uint32_t bit)
  {
    data[bit / 64] ^= (bit << (bit & 0x3F));
  }
  bool CheckBit(uint32_t bit) const
  {
    return data[bit / 64] & (bit << (bit & 0x3F));
  }
  void Reset()
  {
    for (uint32_t i = 0; i < arraySize; ++i)
      data[i] = 0;
  }

  operator bool()
  {
    for (uint32_t i = 0; i < arraySize; ++i)
      if (data[i])
        return true;
    return false;
  }


  // Sets selected bit
  Flag& operator|=(uint32_t bit)
  {
    assert(bit < Size);
    SetBit(bit);
    return *this;
  }
  // bitwise OR 2 flags
  Flag& operator|=(const Flag& flag)
  {
    for(uint32_t i = 0; i < arraySize; ++i)
      data[i] |= flag.data[i];
    return *this;
  }

  // reset all bits and bitwise AND selected bit
  Flag& operator&=(uint32_t bit)
  {
    Reset();
    SetBit(bit);
  }
  // bitwise AND 2 flags
  Flag& operator&=(const Flag& flag)
  {
    for (uint32_t i = 0; i < arraySize; ++i)
      data[i] &= flag.data[i];
    return *this;
  }

  // Toggles selected bit
  Flag& operator^=(uint32_t bit)
  {
    FlipBit(bit);
  }
  // bitwise XOR 2 flags
  Flag& operator^=(const Flag& flag)
  {
    for (uint32_t i = 0; i < arraySize; ++i)
      data[i] ^= flag.data[i];
    return *this;
  }


  Flag operator| (const Flag& flag) const
  {
    Flag temp(*this);
    return temp |= flag;
  }
  Flag operator& (const Flag& flag) const
  {
    Flag temp(*this);
    return temp &= flag;
  }
  Flag operator^ (const Flag& flag) const
  {
    Flag temp(*this);
    return temp ^= flag;
  }
};
