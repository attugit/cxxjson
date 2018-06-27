#include "inc/cxx/cbor.hpp"
#include <algorithm>
#include <arpa/inet.h>

namespace
{
  template <typename T>
  constexpr auto base_type_impl()
  {
    if
      constexpr(std::is_enum_v<T>) { return std::underlying_type_t<T>{}; }
    else
    {
      return T{};
    }
  }

  template <typename T>
  using base_type = std::decay_t<decltype(base_type_impl<T>())>;

  struct initial_byte {
    static inline constexpr base_type<cxx::byte> max_insitu = 23;
    struct type {
      // static inline constexpr base_type<cxx::byte> positive = 0;
      static inline constexpr base_type<cxx::byte> negative = 1;
      // static inline constexpr base_type<cxx::byte> string = 2;
      static inline constexpr base_type<cxx::byte> unicode = 3;
      static inline constexpr base_type<cxx::byte> array = 4;
      static inline constexpr base_type<cxx::byte> document = 5;
      // static inline constexpr base_type<cxx::byte> tag = 6;
      // static inline constexpr base_type<cxx::byte> special = 7;
    };
    struct value {
      static inline constexpr base_type<cxx::byte> False = 0xf4;
      static inline constexpr base_type<cxx::byte> True = 0xf5;
      static inline constexpr base_type<cxx::byte> Null = 0xf6;
      static inline constexpr base_type<cxx::byte> ieee_754_double = 0xfb;
    };

    base_type<cxx::byte> additional : 5; // lower
    base_type<cxx::byte> major : 3;      // higher
  };
  static_assert(sizeof(initial_byte) == sizeof(cxx::byte));

  constexpr auto const initial = cxx::overload(
      [](cxx::json::byte_stream::reference byte) -> initial_byte* {
        return reinterpret_cast<initial_byte*>(&byte);
      },
      [](cxx::json::byte_stream::const_reference byte) -> initial_byte const* {
        return reinterpret_cast<initial_byte const*>(&byte);
      });

  auto const sum = [](auto const&... x) -> decltype(auto) { return (x + ...); };

  std::uint64_t htonll(std::uint64_t x) noexcept
  {
    return (static_cast<std::uint64_t>(htonl(static_cast<std::uint32_t>(x & 0xffffffff))) << 32) |
           htonl(static_cast<std::uint32_t>(x >> 32));
  }

  auto const available = [](cxx::json::byte_stream const& stream) {
    return stream.capacity() - std::size(stream);
  };

  auto const append = [](cxx::json::byte_stream& stream,
                         cxx::json::byte_stream::size_type const space) {
    return stream.reserve(stream.capacity() + space);
  };

  auto const assure = [](cxx::json::byte_stream& stream,
                         cxx::json::byte_stream::size_type const needed) {
    if (available(stream) < needed) append(stream, needed);
  };
}

namespace detail
{
  void encode(cxx::json const&, cxx::json::byte_stream&) noexcept;

  cxx::byte& encode_positive_integer(std::uint64_t x, cxx::json::byte_stream& stream) noexcept
  {
    auto& init = stream.emplace_back(cxx::byte(x));
    if (x <= initial_byte::max_insitu) return init;
    auto const code = sum((x >> 32) != 0, (x >> 16) != 0, (x >> 8) != 0);
    init = cxx::byte(initial_byte::max_insitu + code + 1);
    auto const space = (1u << code);
    auto it = stream.insert(std::end(stream), space, {});
    switch (code)
    {
      case 0:
        *it = cxx::byte(0xff & x);
        break;
      case 1:
        reinterpret_cast<std::uint16_t&>(*it) = htons(static_cast<std::uint16_t>(0xffff & x));
        break;
      case 2:
        reinterpret_cast<std::uint32_t&>(*it) = htonl(static_cast<std::uint32_t>(0xffffffff & x));
        break;
      case 3:
        reinterpret_cast<std::uint64_t&>(*it) = htonll(x);
        break;
    }
    return *(--it);
  }

  void encode_negative_integer(std::int64_t x, cxx::json::byte_stream& stream) noexcept
  {
    initial(encode_positive_integer(static_cast<std::uint64_t>(-(x + 1)), stream))->major =
        initial_byte::type::negative;
  }

  void encode(std::int64_t x, cxx::json::byte_stream& stream) noexcept
  {
    assure(stream, sizeof(std::int64_t) + 1);
    if (x < 0) return encode_negative_integer(x, stream);
    encode_positive_integer(static_cast<std::uint64_t>(x), stream);
  }

  void encode(std::string const& x, cxx::json::byte_stream& stream) noexcept
  {
    assure(stream, std::size(x) + sizeof(std::uint64_t) + 1);
    initial(encode_positive_integer(std::size(x), stream))->major = initial_byte::type::unicode;
    auto first = reinterpret_cast<cxx::byte const*>(x.data());
    stream.insert(std::end(stream), first, first + std::size(x));
  }

  void encode(cxx::json::array const& x, cxx::json::byte_stream& stream) noexcept
  {
    assure(stream, sizeof(std::uint64_t) + 1 + std::size(x) * sizeof(cxx::json));
    initial(encode_positive_integer(std::size(x), stream))->major = initial_byte::type::array;
    for (auto const& item : x) ::detail::encode(item, stream);
  }

  void encode(cxx::json::document const& x, cxx::json::byte_stream& stream) noexcept
  {
    assure(stream, sizeof(std::uint64_t) + 1 + 2 * std::size(x) * sizeof(cxx::json));
    initial(encode_positive_integer(std::size(x), stream))->major = initial_byte::type::document;
    for (auto const & [ key, value ] : x) {
      ::detail::encode(key, stream);
      ::detail::encode(value, stream);
    }
  }

  void encode(bool b, cxx::json::byte_stream& stream) noexcept
  {
    stream.emplace_back(cxx::byte(b ? initial_byte::value::True : initial_byte::value::False));
  }

  void encode(cxx::json::null_t, cxx::json::byte_stream& stream) noexcept
  {
    stream.emplace_back(cxx::byte(initial_byte::value::Null));
  }

  void encode(double d, cxx::json::byte_stream& stream) noexcept
  {
    assure(stream, sizeof(double) + 1);
    stream.emplace_back(cxx::byte(initial_byte::value::ieee_754_double));
    auto dest = stream.insert(std::end(stream), sizeof(double), cxx::byte());
    auto const* first = reinterpret_cast<cxx::byte const*>(&d);
    std::reverse_copy(first, first + sizeof(double), dest);
  }

  void encode(cxx::json const& json, cxx::json::byte_stream& stream) noexcept
  {
    cxx::visit([&stream](auto const& x) { ::detail::encode(x, stream); }, json);
  }
}

auto ::cxx::cbor::encode(json const& j) noexcept -> json::byte_stream
{
  json::byte_stream stream;
  ::detail::encode(j, stream);
  return stream;
}
