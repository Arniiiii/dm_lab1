#ifndef INCLUDE_DATAMINING_LAB1ONE_RULEONE_RULE_HPP_
#define INCLUDE_DATAMINING_LAB1ONE_RULEONE_RULE_HPP_

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <map>
#include <ranges>
#include <vector>
#include <type_traits>

#include <fmt/base.h>
#include <fmt/ranges.h>

#include "datamining_lab1/common.hpp"

namespace datamining_lab1::one_rule
{

  template <typename data_type, typename result_type> class model
  {
    std::size_t position;
    std::map<data_type, result_type> valueable_data_;

  public:
    [[nodiscard]] inline constexpr result_type predict(
        auto const& value_collection) const
      requires std::same_as<
                   typename std::iterator_traits<
                       decltype(value_collection.cbegin())>::value_type,
                   data_type>
               && std::input_iterator<decltype(value_collection.cbegin())>
    {
      // I can do this check, but this check is going to be the same as in any
      // hardened c++ library for bounds checking. if (position < value.size())
      // [[unlikely]]
      //   {
      //     return {};
      //   };
      return valueable_data_.at(
          *std::next(value_collection.cbegin(), position));
    }

    model(std::size_t position, std::map<data_type, result_type> valueable_data)
        : position(position), valueable_data_(std::move(valueable_data))
    {
    }

    [[nodiscard]] constexpr auto getPosition() const { return position; }
    [[nodiscard]] constexpr const auto& getValuableData() const
    {
      return valueable_data_;
    }
  };

  // additional deduction guide...
  template <std::ranges::bidirectional_range RangeT> model(RangeT range)
      -> model<typename decltype(std::declval<std::iter_value_t<
                                     std::ranges::iterator_t<RangeT>>>()
                                     .getValue())::value_type,
               decltype(std::declval<std::iter_value_t<
                            std::ranges::iterator_t<RangeT>>>()
                            .getExpectedResult())>;

  template <std::ranges::bidirectional_range RangeT>
  [[nodiscard]] inline auto create_model(RangeT a_range) noexcept(false)
    requires Con_HasgetValueReturnsRange<
                 std::iter_value_t<std::ranges::iterator_t<RangeT>>>
             and Con_HasgetExpectedResult<
                 std::iter_value_t<std::ranges::iterator_t<RangeT>>>
  {
    using underneeth_sample_type
        = std::iter_value_t<std::ranges::iterator_t<RangeT>>;
    using underneeth_data_collection
        = decltype(std::declval<underneeth_sample_type>().getValue());
    using underneeth_data_type =
        typename std::remove_cvref_t<underneeth_data_collection>::value_type;
    using underneeth_expected_result_type = std::remove_cvref_t<
        decltype(std::declval<underneeth_sample_type>().getExpectedResult())>;

    if (a_range.size() == 0)
      {
        throw std::logic_error("U gave no data. What is wrong with you?");
      }

    // that's why it's not for input_iterator.
    std::map<underneeth_expected_result_type, char> unique_results{};
    std::ranges::for_each(
        a_range, [&unique_results](const auto& sample) mutable {
          unique_results.insert({sample.getExpectedResult(), {}});
        });

    // that's why it's not for input_iterator.
    const auto& values_exemplar = a_range[0].getValue();
    std::size_t amount_of_parameters = values_exemplar.size();

    std::vector<
        std::map<underneeth_data_type,
                 std::map<underneeth_expected_result_type, std::size_t>>>
        tables(amount_of_parameters);

    std::ranges::for_each(a_range, [&tables, amount_of_parameters](
                                       const auto& sample) mutable {
      const auto& value = sample.getValue();
      const auto& expected_result = sample.getExpectedResult();
      for (std::size_t parameter_index = 0;
           parameter_index < amount_of_parameters; ++parameter_index)
        {
          // here's expected that map.operator [] , if not existed, creates
          // size_t{}, which is 0 for size_t.
          ++tables[parameter_index][value[parameter_index]][expected_result];
        }
    });

    // fill with zeros, if they didn't exist
    for (std::size_t parameter_index = 0;
         parameter_index < amount_of_parameters; ++parameter_index)
      {
        for (auto& [row_key, row_map] : tables[parameter_index])
          {
            for (const auto& [expected_result_key, _] : unique_results)
              {
                row_map.try_emplace(expected_result_key, std::size_t{});
              }
          }
      }

    for (const auto& table : tables)
      {
        for (const auto& [row_key, row_map] : table)
          {
            for (const auto& [column_key, count] : row_map)
              {
                fmt::print("{} ", count);
              }
            fmt::println("");
          }
        fmt::println("");
      }

    std::vector<std::size_t> error_of_table(amount_of_parameters, 0);

    for (std::size_t parameter_index = 0;
         parameter_index < amount_of_parameters; ++parameter_index)
      {
        for (const auto& [row_key, row_map] : tables[parameter_index])
          {
            std::pair<underneeth_expected_result_type, std::size_t>
                pair_with_max = *std::ranges::max_element(
                    row_map, [](auto const& pair1, auto const& pair2) {
                      return pair1.second < pair2.second;
                    });
            for (const auto& [column_key, count] : row_map)
              {
                if (column_key != pair_with_max.first)
                  {
                    error_of_table[parameter_index] += count;
                  }
              }
          }
      }

    std::size_t best_table_index = std::distance(
        error_of_table.cbegin(),
        std::min_element(error_of_table.cbegin(), error_of_table.cend()));

    fmt::println(
        "error_of_table: {} , best_table_index: {} , amount_of_parameters {}",
        error_of_table, best_table_index, amount_of_parameters);

    std::size_t position = best_table_index;
    std::map<underneeth_data_type, underneeth_expected_result_type>
        valueable_data;

    for (const auto& [row_key, row_map] : tables[best_table_index])
      {
        std::pair<underneeth_expected_result_type, std::size_t> pair_with_max
            = *std::ranges::max_element(
                row_map, [](auto const& pair1, auto const& pair2) {
                  return pair1.second < pair2.second;
                });
        valueable_data.insert({row_key, pair_with_max.first});
      }

    return model<underneeth_data_type, underneeth_expected_result_type>(
        std::move(position), std::move(valueable_data));
  };

}  // namespace datamining_lab1::one_rule

#endif  // INCLUDE_DATAMINING_LAB1ONE_RULEONE_RULE_HPP_
