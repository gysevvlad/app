#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "wire_base.h"

/*****************************************************************************/
/*
 * Simple class for size boxing
 */
namespace wire::detail
{
    class size_codec
    {
    private:
        using LongCodec = pack<ArithmeticBase<std::uint8_t>, ArithmeticBase<std::uint64_t>>;
        using ShortCodec = pack<ArithmeticBase<std::uint8_t>>;

    public:
        using type = std::size_t;
        using is_const_entry_type = std::false_type;

        static std::byte * tobox(std::byte * dst, const std::size_t & arg)
        {
            if (arg >= 0xFF) {
                return LongCodec::tobox(dst, static_cast<std::uint8_t>(0xFF), arg);
            }
            else {
                return ShortCodec::tobox(dst, static_cast<std::uint8_t>(arg));
            }
        }

        static const std::byte * unbox(const std::byte * src, std::size_t & arg)
        {
            std::uint8_t temp;
            src = ArithmeticBase<std::uint8_t>::unbox(src, temp);
            if (temp == 0xFF) {
                std::uint64_t temp;
                src = ArithmeticBase<std::uint64_t>::unbox(src, temp);
                arg = temp;
            }
            else {
                arg = temp;
            }
            return src;
        }

        static std::size_t size(const std::size_t & arg)
        {
            if (arg >= 0xFF) {
                std::uint8_t stud1;
                std::uint64_t stud2;
                return LongCodec::size(stud1, stud2);
            }
            else {
                std::uint8_t stud;
                return ShortCodec::size(stud);
            }
        }

        static std::size_t size(const std::byte * src)
        {
            std::uint8_t temp;
            ArithmeticBase<std::uint8_t>::unbox(src, temp);
            if (temp == 0xFF) {
                return LongCodec::size(src);
            }
            else {
                return ShortCodec::size(src);
            }
        }
    };
}
/*****************************************************************************/
/*
 * Codec for working with sequences
 */
namespace wire::detail
{
    template<class Codec>
    struct sequence_base_action
    {
        template<class InputIterator>
        static std::byte * tobox(std::byte * buffer, std::pair<InputIterator, std::size_t> seq)
        {
            auto[it, size] = seq;
            buffer = size_codec::tobox(buffer, size);
            while (size) {
                buffer = Codec::tobox(buffer, *it);
                it++;
                size--;
            }
            return buffer;
        }

        template<class OutputIterator>
        static const std::byte * unbox(const std::byte * buffer, OutputIterator it)
        {
            std::size_t size;
            buffer = size_codec::unbox(buffer, size);
            while (size) {
                typename Codec::type temp;
                buffer = Codec::unbox(buffer, temp);
                *it++ = temp;
                size--;
            }
            return buffer;
        }
    };

    template<class Codec, bool is_constant_size_entry = detail::is_constant_size_entry_type<Codec>::value>
    struct sequnce_base_size;

    template<class Codec>
    struct sequnce_base_size<Codec, true>
    {
        static const std::size_t size(const std::byte * buffer)
        {
            std::size_t size;
            size_codec::unbox(buffer, size);
            return size * sizeof(typename Codec::type);
        }

        template <class InputIterator>
        static const std::size_t size(std::pair<InputIterator, std::size_t> seq)
        {
            auto[_, element_count] = seq;
            return size_codec::size(element_count) + element_count * sizeof(typename Codec::type);
        }
    };

    template<class Codec>
    struct sequnce_base_size<Codec, false>
    {
        static std::size_t size(const std::byte * buffer)
        {
            std::size_t element_count;
            buffer = size_codec::unbox(buffer, element_count);
            std::size_t summary_size = wire::size::size(element_count);
            for (std::size_t i = 0; i < element_count; i++) {
                std::size_t size = Codec::size(buffer);
                summary_size += size;
                buffer += size;
            }
            return summary_size;
        }

        template <class InputIterator>
        static std::size_t size(std::pair<InputIterator, std::size_t> seq)
        {
            auto[it, element_number] = seq;
            auto summary_size = size_codec::size(element_number);
            for (std::size_t i = 0; i < element_number; i++) {
                summary_size += Codec::size(*it);
                it++;
            }
            return summary_size;
        }
    };

    template<class Codec, class Type, bool is_constant_size_entry = detail::is_constant_size_entry_type<Codec>::value>
    struct sequence
    {
        using is_const_entry_type = std::false_type;
        using type = Type;

        template<class Entry>
        static std::byte * tobox(std::byte * buffer, const std::vector<Entry> & seq)
        {
            return sequence_base_action<Codec>::tobox(buffer, std::make_pair(seq.cbegin(), seq.size()));
        }

        template<class InputIterator>
        static std::byte * tobox(std::byte * buffer, std::pair<InputIterator, std::size_t> seq)
        {
            return sequence_base_action<Codec>::tobox(buffer, seq);
        }

        static std::byte * tobox(std::byte * buffer, const std::string & seq)
        {
            return sequence_base_action<Codec>::tobox(buffer, std::make_pair(seq.cbegin(), seq.size()));
        }

        static std::byte * tobox(std::byte * buffer, const std::wstring & seq)
        {
            return sequence_base_action<Codec>::tobox(buffer, std::make_pair(seq.cbegin(), seq.size()));
        }

        template<class Entry>
        static const std::byte * unbox(const std::byte * buffer, std::vector<Entry> & it)
        {
            return sequence_base_action<Codec>::unbox(buffer, std::back_inserter(it));
        }

        static const std::byte * unbox(const std::byte * buffer, std::string & it)
        {
            return sequence_base_action<Codec>::unbox(buffer, std::back_inserter(it));
        }

        static const std::byte * unbox(const std::byte * buffer, std::wstring & it)
        {
            return sequence_base_action<Codec>::unbox(buffer, std::back_inserter(it));
        }

        template<class OutputIterator>
        static const std::byte * unbox(const std::byte * buffer, OutputIterator it)
        {
            return sequence_base_action<Codec>::unbox(buffer, it);
        }
    };
}
/*****************************************************************************/