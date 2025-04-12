#pragma once

#include <array>
#include <type_traits>
enum class test_result_enum : bool
{
  success = false,
  failure = true,
};

struct test_sample
{
  std::array<int, 4> value{};
  int result;

  [[nodiscard]] constexpr const auto& getValue() const { return value; }
  [[nodiscard]] constexpr const auto& getExpectedResult() const
  {
    return result;
  }
};

// jff
static_assert(std::is_standard_layout_v<test_sample>, "It's not POD ?");

constexpr auto data = std::to_array<test_sample>({
    {.value = {{0, 0, 1, 0}}, .result = 0},
    {.value = {{1, 0, 0, 1}}, .result = 1},
    {.value = {{2, 0, 1, 0}}, .result = 1},
    {.value = {{0, 1, 0, 0}}, .result = 1},
    {.value = {{0, 1, 1, 0}}, .result = 1},
    {.value = {{0, 1, 1, 1}}, .result = 0},
    {.value = {{1, 0, 0, 1}}, .result = 0},
    {.value = {{2, 0, 0, 0}}, .result = 1},
    {.value = {{2, 1, 1, 0}}, .result = 1},
    {.value = {{0, 1, 1, 1}}, .result = 0},
});
