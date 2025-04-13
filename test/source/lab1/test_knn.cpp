#include <array>
#include <exception>
#include <iostream>

#include "fmt/base.h"
#include "fmt/ostream.h"

#include "datamining_lab1/knn.hpp"
#include "datamining_lab1/test/common.hpp"

namespace
{

  test_result_enum test_knn()
  {
    // auto pointer
    //     = std::unique_ptr<std::remove_pointer_t<decltype(&data)>>(&data);
    auto model = datamining_lab1::knn::create_model(data, 3);
    auto result = model.predict(std::to_array<int>({2, 1, 1, 1}));
    return result == 1 ? test_result_enum::success : test_result_enum::failure;
  }

}  // namespace

int main(int /*argc*/, char* /*argv*/[])
{
  try
    {
      return static_cast<int>(test_knn());
    }
  catch (std::exception& e)
    {
      fmt::println(std::cerr, "Main, got exception: {}", e.what());
    }
}
