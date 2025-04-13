#include <array>
#include <exception>
#include <iostream>
#include <print>

#include "datamining_lab1/one_rule.hpp"
#include "datamining_lab1/test/common.hpp"

namespace
{

  test_result_enum one_rule_test()
  {
    auto model = datamining_lab1::one_rule::create_model(data);
    auto main_table = model.getPosition();
    auto result = model.predict(std::to_array<int>({2, 1, 1, 1}));
    std::println("{}", main_table);
    return result == 0 && main_table == 3 ? test_result_enum::success
                                          : test_result_enum::failure;
  }

}  // namespace

int main(int /*argc*/, char* /*argv*/[])
{
  try
    {
      return static_cast<int>(one_rule_test());
    }
  catch (std::exception& e)
    {
      std::println(std::cerr, "Main, got exception: {}", e.what());
    }
}
