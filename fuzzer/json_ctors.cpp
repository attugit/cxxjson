#include "inc/cxx/json.hpp"
#include "inc/cxx/cbor.hpp"
#include <string_view>
#include <cstddef>
#include <cstdint>

using byte_view = std::basic_string_view<uint8_t>;

cxx::json factory(byte_view);

template <typename T>
cxx::json make(T, byte_view);

template <typename T>
cxx::json make(T, byte_view)
{
  return cxx::json{};
}

cxx::json make(std::int64_t n, byte_view bytes)
{
  std::copy(bytes.data(), bytes.data() + std::min(sizeof(n), std::size(bytes)),
            reinterpret_cast<uint8_t*>(&n));
  return cxx::json{n};
}

cxx::json make(double n, byte_view bytes)
{
  std::copy(bytes.data(), bytes.data() + std::min(sizeof(n), std::size(bytes)),
            reinterpret_cast<uint8_t*>(&n));
  return cxx::json{n};
}

cxx::json make(bool, byte_view bytes)
{
  if (bytes.empty()) return cxx::json{false};
  return cxx::json{!!(bytes.front() & 0x1)};
}

cxx::json make(cxx::json::null_t, byte_view)
{
  return cxx::json{cxx::json::null};
}

std::string do_make(byte_view bytes)
{
  if (bytes.size() < 3) return std::string{};
  auto const tag = static_cast<std::size_t>(bytes.front());
  bytes.remove_prefix(1);
  auto const size = std::min(tag, std::size(bytes));
  return std::string(reinterpret_cast<char const*>(bytes.data()), size);
}

cxx::json make(cxx::json::byte_stream stream, byte_view bytes)
{
  if (bytes.size() < 3) return cxx::json::byte_stream{};
  auto const tag = static_cast<std::size_t>(bytes.front());
  bytes.remove_prefix(1);
  auto const size = std::min(tag, std::size(bytes));
  auto const first = reinterpret_cast<cxx::json::byte_stream::const_pointer>(bytes.data());
  stream.insert(std::end(stream), first, first + size);
  return cxx::json(std::move(stream));
}

cxx::json make(std::string, byte_view bytes)
{
  return cxx::json{do_make(bytes)};
}

cxx::json make(cxx::json::array array, byte_view bytes)
{
  if (bytes.empty()) return cxx::json{std::move(array)};
  auto size = static_cast<std::size_t>(bytes.front());
  bytes.remove_prefix(1);
  while (size--)
  {
    if (bytes.empty()) break;
    array.emplace_back(factory(bytes));
    bytes.remove_prefix(1);
  }
  return cxx::json{std::move(array)};
}

cxx::json make(cxx::json::dictionary dict, byte_view bytes)
{
  if (bytes.empty()) return cxx::json{std::move(dict)};
  auto size = static_cast<std::size_t>(bytes.front());
  bytes.remove_prefix(1);
  while (size--)
  {
    if (bytes.empty()) break;
    auto key = do_make(bytes);
    bytes.remove_prefix(1);
    if (bytes.empty()) break;
    dict[std::move(key)] = factory(bytes);
    bytes.remove_prefix(1);
  }
  return cxx::json{std::move(dict)};
}

cxx::json factory(byte_view bytes)
{
  if (bytes.empty()) return cxx::json{};
  auto const tag = bytes.front();
  bytes.remove_prefix(1);
  switch (tag & 0x7)
  {
    case 0:
      return make(std::int64_t{}, bytes);
      break;
    case 1:
      return make(double{}, bytes);
      break;
    case 2:
      return make(bool{}, bytes);
      break;
    case 3:
      return make(cxx::json::null, bytes);
      break;
    case 4:
      return make(cxx::json::array{}, bytes);
      break;
    case 5:
      return make(cxx::json::dictionary{}, bytes);
      break;
    case 6:
      return make(cxx::json::byte_stream{}, bytes);
      break;
    default:
      return make(std::string{}, bytes);
  }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  byte_view bytes(data, size);
  auto const out = cxx::cbor::encode(factory(bytes));
  if (std::empty(out)) std::terminate();
  return 0;
}
