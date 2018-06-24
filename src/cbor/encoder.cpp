#include "inc/cxx/cbor.hpp"
#include <arpa/inet.h>

namespace
{
  struct initial_byte {
    static constexpr cxx::byte max_insitu = 23;
    struct type {
      static constexpr cxx::byte positive = 0;
      static constexpr cxx::byte negative = 1;
      static constexpr cxx::byte string = 2;
    };
    cxx::byte additional : 5; // lower
    cxx::byte major : 3;      // higher
  };
  static_assert(sizeof(initial_byte) == sizeof(cxx::byte));

  constexpr auto const initial = cxx::overload(
      [](cxx::cbor::byte_stream::reference byte) -> initial_byte* {
        return reinterpret_cast<initial_byte*>(&byte);
      },
      [](cxx::cbor::byte_stream::const_reference byte) -> initial_byte const* {
        return reinterpret_cast<initial_byte const*>(&byte);
      });

  auto const sum = [](auto const&... x) -> decltype(auto) { return (x + ...); };

  std::uint64_t htonll(std::uint64_t x) noexcept
  {
    return (static_cast<std::uint64_t>(htonl(static_cast<std::uint32_t>(x & 0xffffffff))) << 32) |
           htonl(static_cast<std::uint32_t>(x >> 32));
  }
}

namespace detail
{
  cxx::byte& encode_positive_integer(std::int64_t x, cxx::cbor::byte_stream& stream) noexcept
  {
    auto& init = stream.emplace_back(cxx::byte(x));
    if (x <= initial_byte::max_insitu) return init;
    auto const code = sum((x >> 32) != 0, (x >> 16) != 0, (x >> 8) != 0);
    init = cxx::byte(initial_byte::max_insitu + code + 1);
    auto const space = (1 << code);
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

  cxx::byte& encode_negative_integer(std::int64_t x, cxx::cbor::byte_stream& stream) noexcept
  {
    auto& data = encode_positive_integer(-x - 1, stream);
    initial(data)->major = initial_byte::type::negative;
    return data;
  }

  cxx::byte& encode(std::int64_t x, cxx::cbor::byte_stream& stream) noexcept
  {
    if (x < 0) return encode_negative_integer(x, stream);
    return encode_positive_integer(x, stream);
  }

  cxx::byte& encode(std::string const& x, cxx::cbor::byte_stream& stream) noexcept
  {
    auto& data = encode_positive_integer(std::size(x), stream);
    initial(data)->major = initial_byte::type::string;
    auto first = reinterpret_cast<cxx::byte const*>(x.data());
    stream.insert(std::end(stream), first, first + std::size(x));
    return data;
  }

  template <typename T>
  cxx::byte encode(T const&, cxx::cbor::byte_stream&) noexcept
  {
    return {};
  }
}

auto ::cxx::cbor::encode(json const& j) noexcept -> byte_stream
{
  byte_stream stream;
  auto const alloc = cxx::overload(
      [](std::string const& s) -> std::size_t { return std::size(s) * sizeof(std::int64_t) + 1; },
      [](cxx::array const& array) -> std::size_t {
        return std::size(array) * sizeof(cxx::array::value_type);
      },
      [](cxx::document const& doc) -> std::size_t {
        return std::size(doc) * sizeof(cxx::document::value_type);
      },
      [](auto const&) -> std::size_t { return sizeof(cxx::json); });
  stream.reserve(cxx::visit(alloc, j));
  auto const func = [&stream](auto const& x) { detail::encode(x, stream); };
  cxx::visit(func, j);
  return stream;
}
