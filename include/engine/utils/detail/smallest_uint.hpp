#include <concepts>
#include <limits>

namespace engine::detail {
    // an unsigned integer type T is UintCanHold<X> if T can represent X
    template <std::unsigned_integral T, std::uintmax_t X> concept UintCanHold = X <= std::numeric_limits<T>::max();

    /* smallest_uint_t<X> is the smallest fixed width unsigned integer type which can represent X, or void if none can.
     * 
     * Note: if X cannot be represented by uint64_t, but uintmax_t can represent it then smallest_uint_t<X> 
     * is uintmax_t, but since the standard lacks definitions of >64 bit fixed width integer types it is not 
     * guaranteed that it will be the smallest type available, just the smallest fixed width type available
    */
    template <std::uintmax_t X>
    using smallest_uint_t =
            std::conditional_t<UintCanHold<uint_least8_t, X>, uint_least8_t,
                std::conditional_t<UintCanHold<uint_least16_t, X>, uint_least16_t,
                    std::conditional_t<UintCanHold<uint_least32_t, X>, uint_least32_t,
                        std::conditional_t<UintCanHold<uint_least64_t, X>, uint_least64_t,
			    std::conditional_t<UintCanHold<uintmax_t, X>, uintmax_t, void>
			>
                    >
                >
            >;

    // tests...
    static_assert(std::same_as<smallest_uint_t<0ull>, std::uint_least8_t>);
    static_assert(std::same_as<smallest_uint_t<0xffull>, std::uint_least8_t>);
    static_assert(std::same_as<smallest_uint_t<0x100ull>, std::uint_least16_t>);
    static_assert(std::same_as<smallest_uint_t<0xffffull>, std::uint_least16_t>);
    static_assert(std::same_as<smallest_uint_t<0x10000ull>, std::uint_least32_t>);
    static_assert(std::same_as<smallest_uint_t<0xffffffffull>, std::uint_least32_t>);
    static_assert(std::same_as<smallest_uint_t<0x100000000ull>, std::uint_least64_t>);
    static_assert(std::same_as<smallest_uint_t<~std::uint_least64_t(0)>, std::uint_least64_t>);
}
