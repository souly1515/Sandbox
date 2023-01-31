#pragma once

#include "Includes/Defines.h"

const uint64_t Mask_64Bit = 0xFFFFFFFF;

template<uint32_t Size>
class Flag
{
  static_assert(Size % 32);
  const uint32_t arraySize = Size / 32;
  uint64_t data[arraySize];

public:
  Flag(uint64_t flag)
  {
    data[0] = flag;
  }
  Flag(const Flag& flag)
  {
    for(uint32_t i; i < arraySize; ++i)
    data[0] = flag;
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

  Flag& operator|=(uint32_t bit)
  {
    assert(bit < size);
    SetBit(bit);
    return *this;
  }
  Flag& operator|=(uint64_t flag)
  {
    return (*this) |= Flag(flag);
  }
  Flag& operator|=(const Flag& flag)
  {
    for(uint32_t i = 0; i < arraySize; ++i)
      data[i] |= flag.data[i];
    return *this;
  }
  
  Flag& operator&=(uint32_t bit)
  {
    Reset();
    SetBit(bit);
  }
  Flag& operator&=(uint64_t flag)
  {
    return (*this) &= Flag(flag);
  }
  Flag& operator&=(const Flag& flag)
  {
    for (uint32_t i = 0; i < arraySize; ++i)
      data[i] &= flag.data[i];
    return *this;
  }

  Flag& operator^=(uint32_t bit)
  {
    FlipBit(bit);
  }
  Flag& operator^=(uint64_t flag)
  {
    return (*this) ^= Flag(flag);
  }
  Flag& operator^=(const Flag& flag)
  {
    for (uint32_t i = 0; i < arraySize; ++i)
      data[i] ^= flag.data[i];
    return *this;
  }

  bool operator| (const Flag& flag) const
  {
    for (uint32_t i = 0; i < arraySize; ++i)
      if (data[i] |= flag.data[i])
        return true;
    return false;
  }
};
