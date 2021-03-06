#include "inc/cxx/json.hpp"
#include "inc/cxx/cbor.hpp"
#include <string_view>
#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  cxx::json::byte_view bytes(reinterpret_cast<cxx::byte const*>(data), size);
  try
  {
    while (!std::empty(bytes))
    {
      auto const json = cxx::cbor::decode(cxx::by_ref(bytes));
      cxx::cbor::encode(json);
    }
  } catch (cxx::cbor::error const&)
  {
  };
  return 0;
}
