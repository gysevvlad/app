#pragma once

#include <cstddef>
#include <type_traits>

/*****************************************************************************/
/*
 * Type trait for const size entry
 */
namespace wire::detail
{
    template<class FirstArg, class ...OtherArgs>
    struct is_constant_size_entry_type
    {
        using T1 = typename FirstArg::is_const_entry_type;
        using T2 = typename is_constant_size_entry_type<OtherArgs...>;
        static constexpr bool value = T1::value && T2::value;
    };

    template<class Arg>
    struct is_constant_size_entry_type<Arg>
    {
        using T1 = typename Arg::is_const_entry_type;
        static constexpr bool value = T1::value;
    };
}
/*****************************************************************************/
/*
 * Codec classes for common types
 */
namespace wire::detail
{
    template<typename ArithmeticType>
    struct ArithmeticBase
    {
        using type = ArithmeticType;
        using is_const_entry_type = std::true_type;

        template<class ArgType>
        static std::byte * tobox(std::byte * buffer, const ArgType & value)
        {
            *static_cast<ArithmeticType*>(static_cast<void*>(buffer)) = value;
            return buffer + sizeof(ArithmeticType);
        }

        template<class ArgType>
        static const std::byte * unbox(const std::byte * buffer, ArgType & value)
        {
            value = *static_cast<const ArithmeticType*>(static_cast<const void*>(buffer));
            return buffer + sizeof(ArithmeticType);
        }

        static std::size_t size(const std::byte *)
        {
            return sizeof(ArithmeticType);
        }

        static std::size_t size(const ArithmeticType &)
        {
            return sizeof(ArithmeticType);
        }
    };
}
/*****************************************************************************/
