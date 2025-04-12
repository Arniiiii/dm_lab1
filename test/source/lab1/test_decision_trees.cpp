#include "datamining_lab1/decision_trees.hpp"
#include "datamining_lab1/test/common.hpp"

namespace
{
  test_result_enum test_decision_trees()
  {
    auto model = datamining_lab1::decision_trees::create_model(data);
    model->print();
    // auto main_table = model.getPosition();
    auto result = model->predict(std::to_array<int>({2, 1, 1, 1}));
    // fmt::println("{}",main_table);
    return result == 1 ? test_result_enum::success : test_result_enum::failure;
  }
}  // namespace

int main(int /*argc*/, char* /*argv*/[])
{
  return static_cast<int>(test_decision_trees());
}
