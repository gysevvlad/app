#pragma once

#include <cstddef>
#include <utility>
#include <tuple>

#include "wire_detail.h"

/*****************************************************************************/
/*
 * Main class for unwind parameter pack
 */
namespace wire::detail
{
    /*
     * Base pack struct
     */
    template<class ... Entries>
    struct pack;

    /*
     * Pack for one entry
     */
    template<class Entry>
    struct pack<Entry>
    {
        using type = typename Entry::type;
        using is_const_entry_type = is_constant_size_entry_type<Entry>;

        template<class Arg>
        static std::byte * tobox(std::byte * buffer, const Arg & arg)
        {
            return Entry::tobox(buffer, arg);
        }

        template<class Arg>
        static std::byte * tobox(std::byte * buffer, const std::tuple<Arg> & arg)
        {
            return Entry::tobox(buffer, std::get<0>(arg));
        }

        template<class Arg>
        static const std::byte * unbox(const std::byte * buffer, Arg & arg)
        {
            return Entry::unbox(buffer, arg);
        }

        template<class Arg>
        static const std::byte * unbox(const std::byte * buffer, std::tuple<Arg> & arg)
        {
            return Entry::unbox(buffer, std::get<0>(arg));
        }

        template<class Arg>
        static std::size_t size(const Arg & arg)
        {
            return Entry::size(arg);
        }

        template<class Arg>
        static std::size_t size(const std::tuple<Arg> & arg)
        {
            return Entry::size(std::get<0>(arg));
        }

        static std::size_t size(const std::byte * buffer)
        {
            return Entry::size(buffer);
        }
    };

    /*
     * Pack for two entry
     */
    template<class FirstEntry, class SecondEntry>
    struct pack<FirstEntry, SecondEntry>
    {
        using type = std::pair<typename FirstEntry::type, typename SecondEntry::type>;
        using is_const_entry_type = is_constant_size_entry_type<FirstEntry, SecondEntry>;

        template<class FirstArg, class SecondArg>
        static std::byte * tobox(std::byte * buffer, const std::pair<FirstArg, SecondArg> & arg)
        {
            buffer = FirstEntry::tobox(buffer, arg.first);
            return SecondEntry::tobox(buffer, arg.second);
        }

        template<class FirstArg, class SecondArg>
        static std::byte * tobox(std::byte * buffer, const FirstArg & first_arg, const SecondArg & second_arg)
        {
            buffer = FirstEntry::tobox(buffer, first_arg);
            return SecondEntry::tobox(buffer, second_arg);
        }

        template<class FirstArg, class SecondArg>
        static std::byte * tobox(std::byte * buffer, const std::tuple<FirstArg, SecondArg> & arg)
        {
            buffer = FirstEntry::tobox(buffer, std::get<0>(arg));
            return SecondEntry::tobox(buffer, std::get<1>(arg));
        }

        template<class FirstArg, class SecondArg>
        static const std::byte * unbox(const std::byte * buffer, std::pair<FirstArg, SecondArg> & arg)
        {
            buffer = FirstEntry::unbox(buffer, arg.first);
            return SecondEntry::unbox(buffer, arg.second);
        }

        template<class FirstArg, class SecondArg>
        static const std::byte * unbox(const std::byte * buffer, FirstArg & first_arg, SecondArg & second_arg)
        {
            buffer = FirstEntry::unbox(buffer, first_arg);
            return SecondEntry::unbox(buffer, second_arg);
        }

        template<class FirstArg, class SecondArg>
        static const std::byte * unbox(const std::byte * buffer, std::tuple<FirstArg, SecondArg> & arg)
        {
            buffer = FirstEntry::unbox(buffer, std::get<0>(arg));
            return SecondEntry::unbox(buffer, std::get<1>(arg));
        }

        static std::size_t size(const std::byte * buffer)
        {
            auto element_size = FirstEntry::size(buffer);
            return element_size + SecondEntry::size(buffer + element_size);
        }

        template<class FirstArg, class SecondArg>
        static std::size_t size(const FirstArg & first_arg, const SecondArg & second_arg)
        {
            return FirstEntry::size(first_arg) + SecondEntry::size(second_arg);
        }

        template<class FirstArg, class SecondArg>
        static std::size_t size(const std::pair<FirstArg, SecondArg> & arg)
        {
            return FirstEntry::size(arg.first) + SecondEntry::size(arg.second);
        }

        template<class FirstArg, class SecondArg>
        static std::size_t size(const std::tuple<FirstArg, SecondArg> & arg)
        {
            return FirstEntry::size(std::get<0>(arg)) + SecondEntry::size(std::get<1>(arg));
        }
    };

    /*
     * Pack for many entry
     */
    template<class ... Entries>
    struct pack
    {
        using type = std::tuple<typename Entries::type...>;
        using is_const_entry_type = is_constant_size_entry_type<Entries...>;

        template<class ... Args>
        static std::byte * tobox(std::byte * buffer, const Args&... args)
        {
            return pack_helper<0>::tobox(buffer, args...);
        }

        template<class ... Args>
        static const std::byte * unbox(const std::byte * buffer, Args&... args)
        {
            return pack_helper<0>::unbox(buffer, args...);
        }

        template<class ... Args>
        static std::size_t size(const Args&... args)
        {
            return pack_helper<0>::unbox(args...);
        }

        static std::size_t size(const std::byte * buffer)
        {
            return pack_helper<0>::unbox(buffer);
        }

    private:
        using type_helper = std::tuple<Entries...>;

        template<int ArgNumber>
        struct pack_helper
        {
            template<class ... Args>
            static std::byte * tobox(std::byte * buffer, const std::tuple<Args...> & args)
            {
                buffer = std::tuple_element_t<ArgNumber, type_helper>::tobox(buffer, std::get<ArgNumber>(args));
                return pack_helper<ArgNumber + 1>::tobox(buffer, args);
            }

            template<class FirstArg, class ... OtherArgs>
            static std::byte * tobox(std::byte * buffer, const FirstArg & first_arg, const OtherArgs&... other_args)
            {
                buffer = std::tuple_element_t<ArgNumber, type_helper>::tobox(buffer, first_arg);
                return pack_helper<ArgNumber + 1>::tobox(buffer, other_args...);
            }

            template<class ... Args>
            static const std::byte * unbox(const std::byte * buffer, std::tuple<Args...> & args)
            {
                buffer = std::tuple_element_t<ArgNumber, type_helper>::unbox(buffer, std::get<ArgNumber>(args));
                return pack_helper<ArgNumber + 1>::unbox(buffer, args);
            }

            template<class FirstArg, class ... OtherArgs>
            static const std::byte * unbox(const std::byte * buffer, FirstArg & first_arg, OtherArgs&... other_args)
            {
                buffer = std::tuple_element_t<ArgNumber, type_helper>::unbox(buffer, first_arg);
                return pack_helper<ArgNumber + 1>::unbox(buffer, other_args...);
            }

            template<class ... Args>
            static std::size_t size(const std::tuple<Args...> & args)
            {
                return std::tuple_element_t<ArgNumber, type_helper>::size(std::get<ArgNumber>(args)) + pack_helper<ArgNumber + 1>::size(args);
            }

            template<class FirstArg, class ... OtherArgs>
            static std::size_t size(const FirstArg & first_arg, const OtherArgs&... other_args)
            {
                return std::tuple_element_t<ArgNumber, type_helper>::size(first_arg) + pack_helper<ArgNumber + 1>::size(other_args...);
            }

            static std::size_t size(const std::byte * buffer)
            {
                auto element_size = std::tuple_element_t<ArgNumber, type_helper>::size(buffer);
                return element_size + pack_helper<ArgNumber + 1>::size(buffer + element_size);
            }
        };

        template<>
        struct pack_helper<sizeof...(Entries)>
        {
            template<class ... Args>
            static std::byte * tobox(std::byte * buffer, const Args&...)
            {
                return buffer;
            }

            template<class ... Args>
            static const std::byte * unbox(const std::byte * buffer, Args&...)
            {
                return buffer;
            }

            template<class ... Args>
            static std::size_t size(const Args&...)
            {
                return 0;
            }

            static std::size_t size(const std::byte *)
            {
                return 0;
            }
        };
    };
}
/*****************************************************************************/