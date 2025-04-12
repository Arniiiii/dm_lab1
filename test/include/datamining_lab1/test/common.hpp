#pragma once

#include <array>
enum class test_result_enum : bool
{
  success = 0,
  failure = 1,
};

struct test_sample
{
  std::array<int, 4> value{};
  int result;

  [[nodiscard]] constexpr inline const auto& getValue() const { return value; }
  [[nodiscard]] constexpr inline const auto& getExpectedResult() const
  {
    return result;
  }
};

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
