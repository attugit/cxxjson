#include "inc/cxx/msgpack.hpp"
#include "test/catch.hpp"
#include "test/utils.hpp"

using namespace std::string_literals;
using namespace cxx::literals;
using namespace test::literals;

auto const transcode = [](auto const& x) -> cxx::json {
  using msgpack = cxx::msgpack;
  return msgpack::decode(msgpack::encode(x));
};

auto const assert_transcoding = [](auto const& x) { REQUIRE(transcode(x) == x); };

TEST_CASE("msgpack can transcode integers")
{
  assert_transcoding(0x00);
  assert_transcoding(0x7f);
  assert_transcoding(0x80);
  assert_transcoding(0xffffffff);
  assert_transcoding(std::numeric_limits<std::int64_t>::max());

  assert_transcoding(-0x01);
  assert_transcoding(-0x11);
  assert_transcoding(-0x20);
  assert_transcoding(-0x7001);
  assert_transcoding(-0x8000);
  assert_transcoding(std::numeric_limits<std::int64_t>::min());
}

TEST_CASE("msgpack can transcode simple values")
{
  assert_transcoding(cxx::json::null);
  assert_transcoding(true);
  assert_transcoding(false);
}
