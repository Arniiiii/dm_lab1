#ifndef INCLUDE_DATAMINING_LAB1NAIVE_BAYESNAIVE_BAYES_HPP_
#define INCLUDE_DATAMINING_LAB1NAIVE_BAYESNAIVE_BAYES_HPP_

#include <algorithm>
#include <cstddef>
#include <format>
#include <map>
#include <ranges>
#include <stdexcept>
#include <vector>

#include <fmt/base.h>

#include "datamining_lab1/common.hpp"

namespace datamining_lab1::decision_trees
{
  template <typename data_type, typename result_type, typename float_type = float> class model
  {
    std::vector<std::map<data_type, std::map<result_type, float_type>>> tables_likelihoods_;
    std::map<result_type, float_type> main_probability_;

  public:
    [[nodiscard]] inline constexpr result_type predict(auto const& value_collection) const
      requires std::same_as<
                   typename std::iterator_traits<decltype(value_collection.cbegin())>::value_type,
                   data_type>
               && std::input_iterator<decltype(value_collection.cbegin())>
    {
      // here's a lot of edge cases in this function, when user does shit. Let's just check if sizes
      // are correct, because checking if all values at least could be possible from model point of
      // view is too much hassle and too much performance consuming.
      if (tables_likelihoods_.size() != value_collection.size())
        {
          throw std::logic_error(std::format(
              "User is inconsistent: first gave data for one stuff, now gives another data with "
              "different size. What are you doing? sizes expected and received:{} {}",
              tables_likelihoods_.size(), value_collection.size()));
        }

      std::map<result_type, float_type> result;
      for (auto const& [possible_result, main_probability] : main_probability_)
        {
          result[possible_result] = main_probability;

          for (std::size_t i = 0; i < value_collection.size(); ++i)
            {
              result[possible_result]
                  *= tables_likelihoods_.at(i).at(value_collection[i]).at(possible_result);
            }
          fmt::println("prediction: possibility of {} is {}", possible_result,
                       result[possible_result]);
        }
      std::pair<result_type, float_type> prediction = *std::ranges::max_element(
          result, [](const auto& pair1_result_probabilty, auto const& pair2_result_probability) {
            return pair1_result_probabilty.second < pair2_result_probability.second;
          });
      return prediction.first;
    }

    explicit model(
        std::vector<std::map<data_type, std::map<result_type, float_type>>> tables_likelihoods,
        std::map<result_type, float_type> main_probability)
        : tables_likelihoods_(std::move(tables_likelihoods)),
          main_probability_(std::move(main_probability))
    {
    }
  };

  template <std::ranges::bidirectional_range RangeT>
  [[nodiscard]] inline auto create_model(RangeT a_range) noexcept(false)
    requires Con_HasgetValueReturnsRange<std::iter_value_t<std::ranges::iterator_t<RangeT>>>
             and Con_HasgetExpectedResult<std::iter_value_t<std::ranges::iterator_t<RangeT>>>
  {
    using underneeth_sample_type = std::iter_value_t<std::ranges::iterator_t<RangeT>>;
    using underneeth_data_collection = decltype(std::declval<underneeth_sample_type>().getValue());
    using underneeth_data_type =
        typename std::remove_cvref_t<underneeth_data_collection>::value_type;
    using underneeth_expected_result_type
        = std::remove_cvref_t<decltype(std::declval<underneeth_sample_type>().getExpectedResult())>;

    if (a_range.size() == 0)
      {
        throw std::logic_error("U gave no data. What is wrong with you?");
      }

    // that's why it's not for input_iterator.
    std::map<underneeth_expected_result_type, std::size_t> unique_results{};
    std::ranges::for_each(a_range, [&unique_results](const auto& sample) mutable {
      ++unique_results[sample.getExpectedResult()];
    });

    // that's why it's not for input_iterator.
    const auto& values_exemplar = a_range[0].getValue();
    std::size_t amount_of_parameters = values_exemplar.size();

    std::vector<
        std::map<underneeth_data_type, std::map<underneeth_expected_result_type, std::size_t>>>
        tables(amount_of_parameters);

    std::ranges::for_each(a_range, [&tables, amount_of_parameters](const auto& sample) mutable {
      const auto& value = sample.getValue();
      const auto& expected_result = sample.getExpectedResult();
      for (std::size_t parameter_index = 0; parameter_index < amount_of_parameters;
           ++parameter_index)
        {
          // here's expected that map.operator [] , if not existed, creates size_t{}, which is 0
          // for size_t.
          ++tables[parameter_index][value[parameter_index]][expected_result];
        }
    });

    // fill with zeros, if they didn't exist
    for (std::size_t parameter_index = 0; parameter_index < amount_of_parameters; ++parameter_index)
      {
        for (auto& [row_key, row_map] : tables[parameter_index])
          {
            for (const auto& [expected_result_key, _] : unique_results)
              {
                /* auto [ iterator, is_new_inserted] = */ row_map.try_emplace(expected_result_key,
                                                                              std::size_t{});
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

    std::vector<std::map<underneeth_data_type, std::map<underneeth_expected_result_type, float>>>
        tables_likelihoods_(tables.size());
    std::map<underneeth_expected_result_type, float> main_probability_;

    for (const auto& [a_result, count] : unique_results)
      {
        main_probability_[a_result]
            = static_cast<float>(count) / static_cast<float>(a_range.size());
      }

    for (std::size_t param_index = 0; param_index < tables.size(); ++param_index)
      {
        for (const auto& [a_result, _] : unique_results)
          {
            std::map<underneeth_data_type, std::size_t> counts_state_for_param;
            std::size_t overall_count_of_the_state_for_the_param = 0;
            bool is_zero_exists = false;

            for (const auto& [row_key, row_map] : tables[param_index])
              {
                std::size_t a_count = row_map.at(a_result);
                counts_state_for_param[row_key] = a_count;
                overall_count_of_the_state_for_the_param += a_count;
                if (a_count == 0)
                  {
                    is_zero_exists = true;
                  }
              }

            std::map<underneeth_data_type, float> likelihoods;

            for (const auto& [param, count] : counts_state_for_param)
              {
                tables_likelihoods_[param_index][param][a_result]
                    = static_cast<float>(count)
                      / static_cast<float>(overall_count_of_the_state_for_the_param);
              }

            if (is_zero_exists)
              {
                // doing additive smoothing.

                // it's too much if try to do via std::algorihtms
                float min_probabilty = 1;
                for (const auto& [param, map_result_likelihood] : tables_likelihoods_[param_index])
                  {
                    float likelihood = map_result_likelihood.at(a_result);
                    if (likelihood != 0.0F && likelihood < min_probabilty)
                      {
                        min_probabilty = likelihood;
                      }
                  }

                // by a book: this has to be lesser than minimal probability
                float alpha_for_additive_smoothing = 0.25F * min_probabilty;

                for (const auto& [param, count] : counts_state_for_param)
                  {
                    tables_likelihoods_[param_index][param][a_result]
                        = (static_cast<float>(count) + alpha_for_additive_smoothing)
                          / (static_cast<float>(overall_count_of_the_state_for_the_param)
                             + alpha_for_additive_smoothing
                                   * static_cast<float>(counts_state_for_param.size()));
                  }
              }
          }
      }

    for (const auto& table : tables_likelihoods_)
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

    return model(std::move(tables_likelihoods_), std::move(main_probability_));
  }

}  // namespace datamining_lab1::naive_bayes

#endif  // INCLUDE_DATAMINING_LAB1NAIVE_BAYESNAIVE_BAYES_HPP_
