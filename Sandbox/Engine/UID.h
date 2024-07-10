#pragma once
#include <cstdint>
#include <vector>

consteval uint32_t ConstExprStrHash(std::string_view str)
{
    // random 7 digit primes i got from a prime number generator
    uint32_t primeArr[] =
    {
      5903789,
      8646581,
      2189081,
      8145971,
      9693413,
      5365751,
      7874981,
      5825207,
      1738423,
      2790649,
    };

    uint32_t mulTotal = 1;
    uint32_t primeIndex = 0;
    for (auto& c : str)
    {
        mulTotal = mulTotal * c % primeArr[primeIndex];
        // There should be no way the above expression results in 0 as there is no way
        // to get a prime number from multiplication

        primeIndex = (primeIndex + 1) % (sizeof(primeArr) / sizeof(primeArr[0]));
    }

    return mulTotal % primeArr[primeIndex];
}

#define RegisterResource_Macro(BaseType, TypeName)\
    struct Resource_##BaseType_##TypeName\
    {};\
    template<>\
    struct uid_base<BaseType, Resource_##BaseType_##TypeName>\
    {\
        static constexpr uint32_t internalID = ConstExprStrHash(#TypeName);\
        static constexpr std::string_view resourceName = {#TypeName};\
    };

#define GetResource_Macro(BaseType, TypeName) uid_base<BaseType, Resource_##BaseType_##TypeName>::internalID

#define DeclareIV(TypeName) RegisterResource_Macro(GfxImageView, TypeName)
#define GetIV(TypeName) GetResource_Macro(GfxImageView, TypeName)

#define GetResourceType(BaseType, TypeName) uid_base<BaseType, Resource_##BaseType_##TypeName>

template <typename BaseType, typename ResourceName>
struct uid_base
{
    constexpr uid_base() = default;
private:
};

