#include "inc/cxx/cbor.hpp"
#include "test/catch.hpp"
#include "test/utils.hpp"

using namespace test::literals;

using cbor = cxx::cbor;

TEST_CASE("cbor transcoding of integers")
{
  auto const assert_transcoding = [](std::int64_t x) {
    REQUIRE(cbor::decode(cbor::encode(x)) == x);
  };
  assert_transcoding(0x00);
  assert_transcoding(0x17);
  assert_transcoding(0x18);
  assert_transcoding(0xff);
  assert_transcoding(0x100);
  assert_transcoding(0xffff);
  assert_transcoding(0x10000);
  assert_transcoding(0xffffffff);
  assert_transcoding(0x100000000);
  assert_transcoding(-0x01);
  assert_transcoding(-0x18);
  assert_transcoding(-0x19);
  assert_transcoding(-0x100);
  assert_transcoding(-0x101);
  assert_transcoding(-0x10000);
  assert_transcoding(-0x10001);
  assert_transcoding(-0x100000000);
  assert_transcoding(-0x100000001);
}

TEST_CASE("cbor transcoding of byte_stream")
{
  auto const assert_transcoding = [](cxx::json::byte_stream const& x) {
    REQUIRE(cbor::decode(cbor::encode(x)) == x);
  };
  assert_transcoding(""_hex);
  assert_transcoding("00"_hex);
  assert_transcoding("00112233445566778899aabbccddeeff00112233445566"_hex);
  assert_transcoding("00112233445566778899aabbccddeeff0011223344556677"_hex);
}

TEST_CASE("cbor transcoding of unicode strings")
{
  auto const assert_transcoding = [](char const* x) {
    REQUIRE(cbor::decode(cbor::encode(x)) == x);
  };
  assert_transcoding("");
  assert_transcoding("a");
  assert_transcoding("IETF");
  assert_transcoding("\"\\");
  assert_transcoding("ü");
  assert_transcoding("水");
  assert_transcoding("𐅑");
  assert_transcoding("lorem");
  assert_transcoding("ipsum dolor sit amet consectetur");
}
