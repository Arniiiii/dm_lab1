#include <array>

#include <fmt/base.h>

#include "datamining_lab1/decision_trees.hpp"

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

test_result_enum test_decision_trees()
{
  auto model = datamining_lab1::decision_trees::create_model(data);
  model->print();
  // auto main_table = model.getPosition();
  auto result = model->predict(std::to_array<int>({2, 1, 1, 1}));
  // fmt::println("{}",main_table);
  return result == 1 ? test_result_enum::success : test_result_enum::failure;
}

int main(int  /*argc*/, char*  /*argv*/[])
{
  return static_cast<int>(test_decision_trees());
}
