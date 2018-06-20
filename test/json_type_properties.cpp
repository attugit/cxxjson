#include "inc/cxx/json.hpp"
#include <type_traits>
#include <string_view>

namespace cxx::traits
{
  template <typename T, typename U, typename = void>
  struct is_equality_comparable : std::false_type {
  };

  template <typename T, typename U>
  struct is_equality_comparable<
      T,
      U,
      std::void_t<
          decltype(std::declval<bool&>() = (std::declval<T const&>() == std::declval<U const&>())),
          decltype(std::declval<bool&>() = (std::declval<T const&>() != std::declval<U const&>())),
          decltype(std::declval<bool&>() = (std::declval<U const&>() == std::declval<T const&>())),
          decltype(std::declval<bool&>() = (std::declval<U const&>() != std::declval<T const&>()))>>
      : std::true_type {
  };

  template <typename T, typename U>
  constexpr bool is_equality_comparable_v = is_equality_comparable<T, U>::value;
}

static_assert(cxx::null == cxx::null);
static_assert(!(cxx::null != cxx::null));

static_assert(std::is_nothrow_default_constructible_v<cxx::json>);
static_assert(std::is_nothrow_move_constructible_v<cxx::json>);
static_assert(std::is_copy_constructible_v<cxx::json>);
static_assert(std::is_nothrow_move_assignable_v<cxx::json>);
static_assert(std::is_copy_assignable_v<cxx::json>);

static_assert(cxx::is_alternative<cxx::document>);
static_assert(cxx::is_alternative<cxx::array>);
static_assert(cxx::is_alternative<std::string>);
static_assert(cxx::is_alternative<std::int64_t>);
static_assert(cxx::is_alternative<double>);
static_assert(cxx::is_alternative<bool>);
static_assert(cxx::is_alternative<cxx::null_t>);

static_assert(std::is_constructible_v<cxx::json, cxx::document const&>);
static_assert(std::is_nothrow_constructible_v<cxx::json, cxx::document&&>);
static_assert(std::is_constructible_v<cxx::json, cxx::array const&>);
static_assert(std::is_nothrow_constructible_v<cxx::json, cxx::array&&>);
static_assert(std::is_constructible_v<cxx::json, std::string const&>);
static_assert(std::is_nothrow_constructible_v<cxx::json, std::string&&>);
static_assert(std::is_nothrow_constructible_v<cxx::json, std::int64_t>);
static_assert(std::is_nothrow_constructible_v<cxx::json, double>);
static_assert(std::is_nothrow_constructible_v<cxx::json, bool>);
static_assert(std::is_nothrow_constructible_v<cxx::json, cxx::null_t>);

static_assert(std::is_assignable_v<cxx::json, cxx::document const&>);
static_assert(std::is_nothrow_assignable_v<cxx::json, cxx::document&&>);
static_assert(std::is_assignable_v<cxx::json, cxx::array const&>);
static_assert(std::is_nothrow_assignable_v<cxx::json, cxx::array&&>);
static_assert(std::is_assignable_v<cxx::json, std::string const&>);
static_assert(std::is_nothrow_assignable_v<cxx::json, std::string&&>);
static_assert(std::is_nothrow_assignable_v<cxx::json, std::int64_t>);
static_assert(std::is_nothrow_assignable_v<cxx::json, double>);
static_assert(std::is_nothrow_assignable_v<cxx::json, bool>);
static_assert(std::is_nothrow_assignable_v<cxx::json, cxx::null_t>);

static_assert(std::is_nothrow_constructible_v<cxx::json, std::int32_t>);
static_assert(std::is_nothrow_constructible_v<cxx::json, std::int16_t>);
static_assert(std::is_nothrow_constructible_v<cxx::json, std::int8_t>);
static_assert(std::is_nothrow_constructible_v<cxx::json, std::uint32_t>);
static_assert(std::is_nothrow_constructible_v<cxx::json, std::uint16_t>);
static_assert(std::is_nothrow_constructible_v<cxx::json, std::uint8_t>);

static_assert(std::is_same_v<std::int64_t, cxx::alternatives::find<std::is_integral>>);
static_assert(!cxx::is_alternative<std::int32_t>);
static_assert(!std::is_same_v<cxx::json, std::int32_t>);
static_assert(cxx::is_compatibile<std::int32_t>);
static_assert(std::is_same_v<cxx::compatibile_alternative<std::int32_t>, std::int64_t>);
static_assert(std::is_same_v<cxx::compatibile_alternative<std::uint32_t>, std::int64_t>);
static_assert(std::is_same_v<cxx::compatibile_alternative<std::int16_t>, std::int64_t>);
static_assert(std::is_same_v<cxx::compatibile_alternative<std::uint16_t>, std::int64_t>);
static_assert(std::is_same_v<cxx::compatibile_alternative<std::int8_t>, std::int64_t>);
static_assert(std::is_same_v<cxx::compatibile_alternative<std::uint8_t>, std::int64_t>);

static_assert(std::is_constructible_v<cxx::json, std::string_view>);

static_assert(!cxx::is_alternative<std::string_view>);
static_assert(cxx::is_compatibile<std::string_view>);
static_assert(std::is_same_v<cxx::compatibile_alternative<std::string_view>, std::string>);

static_assert(std::is_constructible_v<cxx::json, char const*>);
static_assert(std::is_same_v<cxx::compatibile_alternative<char const*>, std::string>);

static_assert(std::is_constructible_v<cxx::json, char const (&)[6]>);
static_assert(std::is_same_v<cxx::compatibile_alternative<char const (&)[6]>, std::string>);

static_assert(std::is_nothrow_assignable_v<cxx::json, std::int32_t>);
static_assert(std::is_assignable_v<cxx::json, std::string_view>);
static_assert(std::is_assignable_v<cxx::json, char const*>);
static_assert(std::is_assignable_v<cxx::json, char const (&)[6]>);

static_assert(cxx::traits::is_equality_comparable_v<cxx::json, cxx::json>);
static_assert(cxx::traits::is_equality_comparable_v<cxx::json, cxx::document>);
static_assert(cxx::traits::is_equality_comparable_v<cxx::json, std::int64_t>);
static_assert(cxx::traits::is_equality_comparable_v<cxx::json, cxx::array>);
static_assert(cxx::traits::is_equality_comparable_v<cxx::json, std::string>);
static_assert(cxx::traits::is_equality_comparable_v<cxx::json, double>);
static_assert(cxx::traits::is_equality_comparable_v<cxx::json, bool>);
static_assert(cxx::traits::is_equality_comparable_v<cxx::json, cxx::null_t>);

static_assert(cxx::traits::is_equality_comparable_v<cxx::json, std::int32_t>);
static_assert(cxx::traits::is_equality_comparable_v<cxx::json, std::string_view>);
static_assert(cxx::traits::is_equality_comparable_v<cxx::json, char const*>);
static_assert(cxx::traits::is_equality_comparable_v<cxx::json, char const (&)[6]>);
