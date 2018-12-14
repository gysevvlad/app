#pragma once

#include "wire_extension.h"

/*****************************************************************************/
/*
 * Base wire types
 */
namespace wire
{
    using  i64 = detail::ArithmeticBase<std::int64_t>;
    using  u64 = detail::ArithmeticBase<std::uint64_t>;
    using  i32 = detail::ArithmeticBase<std::int32_t>;
    using  u32 = detail::ArithmeticBase<std::uint32_t>;
    using  i16 = detail::ArithmeticBase<std::int16_t>;
    using  u16 = detail::ArithmeticBase<std::uint16_t>;
    using   i8 = detail::ArithmeticBase<std::int8_t>;
    using   u8 = detail::ArithmeticBase<std::uint8_t>;
    using  f32 = detail::ArithmeticBase<float>;
    using  f64 = detail::ArithmeticBase<double>;
    using size = detail::size_codec;
    using  str = detail::sequence<i8, std::string>;
    using wstr = detail::sequence<i16, std::wstring>;

    template<class ...WireTypes>
    using pack = detail::pack<WireTypes...>;

    template<class WireType> 
    using sequence = detail::sequence<WireType, std::vector<typename WireType::type>>;
}
/*****************************************************************************/
/*
 * Examples
 *
/*****************************************************************************/
/*
        using codec = wire::sequence<wire::pack<wire::i32, wire::i32, wire::i32>>;
        codec::type v1 = {
                {1, 2, 3}, {4,5,6}, {7,8,9}
        };
        codec::tobox(buffer1, v1);

        codec::type v2;
        codec::unbox(buffer1, v2);
 */
/*****************************************************************************/
/*
    using codec = wire::sequence<wire::sequence<wire::pack<wire::size, wire::size, wire::size>>>;
    codec::type v1 = {
            {{1, 2, 3}, {4,5,6}, {7,8,9}},
            {{1, 2, 3}, {4,5,6}, {7,8,9}},
            {{1, 2, 3}, {4,5,6}, {7,8,9}}
    };
    codec::tobox(buffer1, std::make_pair(v1.cbegin(), v1.size()));
 */
/*****************************************************************************/
/*
    using codec = wire::sequence<wire::sequence<wire::sequence<wire::pack<wire::i32, wire::i32, wire::i32>>>>;
    codec::type v1 = {
        {
            {{1,2,3}, {4,5,6}, {7,8,9}},
            {{1,2,3}, {4,5,6}, {7,8,9}},
            {{1,2,3}, {4,5,6}, {7,8,9}}
        },
        {
            {{1,2,3}, {4,5,6}, {7,8,9}},
            {{1,2,3}, {4,5,6}, {7,8,9}},
            {{1,2,3}, {4,5,6}, {7,8,9}}
        },
        {
            {{1,2,3}, {4,5,6}, {7,8,9}},
            {{1,2,3}, {4,5,6}, {7,8,9}},
            {{1,2,3}, {4,5,6}, {7,8,9}}
        }
    };
    codec::tobox(buffer1, v1);
 */
/*****************************************************************************/
/*
    using codec = wire::sequence<wire::str>;
    codec::type v1 = {
        "one",
        "two",
        "1111!!!1!!1!!!1!",
    };
    codec::tobox(buffer1, v1);
*/
/*****************************************************************************/