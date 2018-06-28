#include "inc/cxx/cbor.hpp"
#include "src/cbor/initial_byte.hpp"
#include <arpa/inet.h>

namespace
{
  using initial_byte = cxx::codec::cbor::initial_byte;
  template <cxx::codec::cbor::base_type<cxx::byte> t>
  using tag_t = std::integral_constant<cxx::codec::cbor::base_type<cxx::byte>, t>;
  template <cxx::codec::cbor::base_type<cxx::byte> t>
  constexpr tag_t<t> tag{};

  constexpr auto const ntoh = cxx::overload(
      [](std::uint8_t, cxx::cbor::byte_view bytes) -> std::int64_t {
        return static_cast<std::int64_t>(bytes.front());
      },
      [](std::uint16_t x, cxx::cbor::byte_view bytes) -> std::int64_t {
        x = *reinterpret_cast<std::uint16_t const*>(bytes.data());
        return ntohs(x);
      },
      [](std::uint32_t, cxx::cbor::byte_view bytes) -> std::int64_t {
        return ntohl(*reinterpret_cast<std::uint32_t const*>(bytes.data()));
      });

  template <typename Sink>
  cxx::cbor::byte_view parse(cxx::cbor::byte_view, Sink);

  template <typename Int, typename Sink>
  cxx::cbor::byte_view parse(Int, cxx::cbor::byte_view bytes, Sink sink)
  {
    if (std::size(bytes) < sizeof(Int))
      throw cxx::cbor::buffer_error("not enough data to decode json");
    sink(ntoh(Int{}, bytes));
    bytes.remove_prefix(sizeof(Int));
    return bytes;
  }

  template <typename Sink>
  cxx::cbor::byte_view parse(tag_t<initial_byte::type::positive>,
                             cxx::byte byte,
                             cxx::cbor::byte_view bytes,
                             Sink sink)
  {
    auto const additional = cxx::codec::cbor::initial(byte)->additional;
    switch (additional)
    {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x06:
      case 0x07:
      case 0x08:
      case 0x09:
      case 0x0a:
      case 0x0b:
      case 0x0c:
      case 0x0d:
      case 0x0e:
      case 0x0f:
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x17:
        sink(static_cast<std::int64_t>(additional));
        return bytes;
      case 0x18:
        return parse(std::uint8_t{0}, bytes, sink);
      case 0x19:
        return parse(std::uint16_t{0}, bytes, sink);
      case 0x1a:
        return parse(std::uint32_t{0}, bytes, sink);
      case 0x1b:
        if (std::size(bytes) < (1u << (additional - 0x17)))
          throw cxx::cbor::buffer_error("not enough data to decode json");
        throw cxx::cbor::unsupported("integer format not yet supported");
      default:
        throw cxx::cbor::data_error("meaningless additional data in initial byte");
    }
  }

  template <typename Sink>
  cxx::cbor::byte_view parse(cxx::cbor::byte_view bytes, Sink sink)
  {
    if (bytes.empty()) throw cxx::cbor::buffer_error("not enough data to decode json");
    auto const byte = bytes.front();
    bytes.remove_prefix(1);
    switch (cxx::codec::cbor::initial(byte)->major)
    {
      case initial_byte::type::positive:
        return parse(tag<initial_byte::type::positive>, byte, bytes, sink);
      default:
        throw cxx::cbor::unsupported("decoding given type is not yet supported");
    }
  }
}

auto ::cxx::cbor::decode(json::byte_stream const& stream) -> std::pair<json, byte_view>
{
  return decode(byte_view(stream.data(), std::size(stream)));
}

auto ::cxx::cbor::decode(byte_view bytes) -> std::pair<json, byte_view>
{
  std::pair<json, byte_view> ret;
  auto sink = cxx::overload([&ret](std::int64_t x) { ret.first = x; }, [](auto const&) {});
  ret.second = parse(bytes, sink);
  return ret;
}
