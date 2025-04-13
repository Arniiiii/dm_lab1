#include <array>
#include <exception>
#include <iostream>
#include <print>

#include "datamining_lab1/naive_bayes.hpp"
#include "datamining_lab1/test/common.hpp"

namespace
{
  test_result_enum naive_bayes_test() noexcept(false)
  {
    auto model = datamining_lab1::naive_bayes::create_model(data);
    // auto main_table = model.getPosition();
    auto result = model.predict(std::to_array<int>({2, 1, 1, 1}));
    // std::println("{}",main_table);
    return result == 1 ? test_result_enum::success : test_result_enum::failure;
  }
}  // namespace

int main(int /*argc*/, char* /*argv*/[])
{
  try
    {
      return static_cast<int>(naive_bayes_test());
    }
  catch (std::exception& e)
    {
      std::println(std::cerr, "Main, got exception: {}", e.what());
    }
}
