#include "inc/cxx/json.hpp"
#include "test/catch.hpp"
#include <type_traits>
#include <memory>

TEST_CASE("cxx::get can use const cxx::json")
{
  cxx::json const json;
  REQUIRE(cxx::holds_alternative<cxx::document>(json));
  REQUIRE_THROWS_AS(cxx::get<cxx::array>(json), std::bad_variant_access);

  auto const& document = cxx::get<cxx::document>(json);
  REQUIRE(json == document);
}

TEST_CASE("cxx::get can use non-const cxx::json")
{
  cxx::json json;
  REQUIRE(cxx::holds_alternative<cxx::document>(json));
  REQUIRE_THROWS_AS(cxx::get<cxx::array>(json), std::bad_variant_access);

  auto& document = cxx::get<cxx::document>(json);
  document["lorem"] = 42l;
  REQUIRE(json == document);
}

TEST_CASE("const cxx::json allows item access for string keys")
{
  using namespace cxx::literals;
  SECTION("throws exception if underlying object is not a document")
  {
    cxx::json const json = 42;
    REQUIRE_THROWS_AS(json["sit"], std::bad_variant_access);
  }
  cxx::json const json = {
      // clang-format off
      {"lorem"_key, 42},
      {"ipsum"_key, cxx::null},
      {"dolor"_key, "sit"},
      {"amet"_key, 3.14}
      // clang-format on
  };
  SECTION("throws exception for nonexisting key")
  {
    REQUIRE_THROWS_AS(json["sit"], std::out_of_range);
  }
  REQUIRE(json["lorem"] == 42);
  REQUIRE(json["ipsum"] == cxx::null);
  REQUIRE(json["dolor"] == "sit");
  REQUIRE(json["amet"] == 3.14);
}

TEST_CASE("non-const cxx::json allows item access for string keys")
{
  using namespace cxx::literals;
  SECTION("throws exception if underlying object is not a document")
  {
    cxx::json json = 42;
    REQUIRE_THROWS_AS(json["sit"], std::bad_variant_access);
  }
  cxx::json json = {
      // clang-format off
      {"lorem"_key, 42},
      {"ipsum"_key, cxx::null},
      {"dolor"_key, "sit"},
      {"amet"_key, 3.14}
      // clang-format on
  };
  REQUIRE(json["lorem"] == 42);
  REQUIRE(json["ipsum"] == cxx::null);
  REQUIRE(json["dolor"] == "sit");
  REQUIRE(json["amet"] == 3.14);

  SECTION("returns new default created cxx::json object")
  {
    auto& nyu = json["sit"];
    REQUIRE(cxx::holds_alternative<cxx::document>(nyu));
    REQUIRE(std::size(nyu) == 0);
    REQUIRE(std::empty(nyu));
    nyu["consectetur"] = 7;
    REQUIRE(json["sit"]["consectetur"] == 7);
  }
}
