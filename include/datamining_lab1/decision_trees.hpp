#ifndef INCLUDE_DATAMINING_LAB1NAIVE_BAYESNAIVE_BAYES_HPP_
#define INCLUDE_DATAMINING_LAB1NAIVE_BAYESNAIVE_BAYES_HPP_

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <print>
#include <ranges>
#include <set>
#include <stdexcept>
#include <vector>
#include <type_traits>

#include <fmt/base.h>

#include "boost/dynamic_bitset.hpp"
#include "datamining_lab1/common.hpp"

namespace datamining_lab1::decision_trees
{

  template <std::ranges::bidirectional_range SampleT, typename ResultType>
  class INode
  {
  public:
    INode(const INode&) = delete;
    INode(INode&&) = delete;
    INode& operator=(const INode&) = delete;
    INode& operator=(INode&&) = delete;
    INode() = default;
    virtual ~INode() = default;
    virtual ResultType predict(SampleT const&) = 0;
    virtual void print() = 0;
  };

  template <std::ranges::bidirectional_range SampleT, typename ResultType>
  class JustResult : public INode<SampleT, ResultType>
  {
    ResultType result;

  public:
    JustResult(const JustResult&) = default;
    JustResult(JustResult&&) = default;
    JustResult& operator=(const JustResult&) = default;
    JustResult& operator=(JustResult&&) = default;
    ~JustResult() = default;

    explicit JustResult(ResultType result_arg) : result(result_arg) {};
    ResultType predict(SampleT const& /*unused*/) override { return result; }
    void print() override { std::print("res {} ", result); }
  };

  template <std::ranges::bidirectional_range SampleT, typename ResultType>
  class Tree : public INode<SampleT, ResultType>
  {
    using underneeth_data_collection = SampleT;
    using underneeth_data_type =
        typename std::remove_cvref_t<underneeth_data_collection>::value_type;

    std::map<underneeth_data_type, std::unique_ptr<INode<SampleT, ResultType>>>
        tree;
    std::size_t position;

  public:
    Tree(const Tree&) = default;
    Tree(Tree&&) = default;
    Tree& operator=(const Tree&) = default;
    Tree& operator=(Tree&&) = default;
    ~Tree() = default;

    explicit Tree(std::map<underneeth_data_type,
                           std::unique_ptr<INode<SampleT, ResultType>>>
                      a_tree_map,
                  std::size_t a_position)
        : tree(std::move(a_tree_map)), position(a_position) {};
    ResultType predict(SampleT const& sample) override
    {
      return tree.at(sample.at(position))->predict(sample);
    }
    void print() override
    {
      std::print("tree param index: {} ", position);
      for (auto const& [key, val] : tree)
        {
          std::print("param_index {} key_val {} ", position, key);
          val->print();
        }
    }
  };

  template <typename map_key_type> double informational_entropy(
      std::map<map_key_type, std::size_t> const& count_of_results,
      std::size_t amount_of_samples)
  {
    double result = 0;

    for (auto const& [a_result, count] : count_of_results)
      {
        double tmp = static_cast<double>(count)
                     / static_cast<double>(amount_of_samples);
        result -= tmp * std::log2(tmp);
      }

    return result;
  }

  template <typename map_key_type> double informational_entropy(
      std::map<map_key_type, std::size_t> const& count_of_results)
  {
    std::size_t amount_of_samples
        = std::accumulate(count_of_results.cbegin(), count_of_results.cend(), 0,
                          [](std::size_t init, auto const& pair_result_count) {
                            return init + pair_result_count.second;
                          });

    return informational_entropy(count_of_results, amount_of_samples);
  }

  template <std::ranges::bidirectional_range RangeT>
  double informational_entropy(RangeT const& samples,
                               boost::dynamic_bitset<> const& bitmask_samples)
  {
    using underneeth_sample_type
        = std::iter_value_t<std::ranges::iterator_t<RangeT>>;
    using underneeth_expected_result_type = std::remove_cvref_t<
        decltype(std::declval<underneeth_sample_type>().getExpectedResult())>;

    std::size_t amount_of_samples = 0;
    std::map<underneeth_expected_result_type, std::size_t> count_of_results;

    for (std::size_t i = 0; i < samples.size(); ++i)
      {
        if (bitmask_samples[i])
          {
            ++amount_of_samples;
            ++count_of_results[samples[i].getExpectedResult()];
          }
      }

    return informational_entropy(count_of_results, amount_of_samples);
  };

  template <typename key_type> struct map_and_counts
  {
    std::map<key_type, std::size_t> map{};
    std::size_t sum_of_counts{};
  };

  template <std::ranges::bidirectional_range RangeT>
  auto informational_criteria(RangeT const& samples,
                              boost::dynamic_bitset<> const& bitmask_samples,
                              boost::dynamic_bitset<> const& bitmask_params)
  {
    using underneeth_sample_type
        = std::iter_value_t<std::ranges::iterator_t<RangeT>>;
    using underneeth_data_collection
        = decltype(std::declval<underneeth_sample_type>().getValue());
    using underneeth_data_type =
        typename std::remove_cvref_t<underneeth_data_collection>::value_type;
    using underneeth_expected_result_type = std::remove_cvref_t<
        decltype(std::declval<underneeth_sample_type>().getExpectedResult())>;

    // calculate entropy of all acquired samples
    double general_entropy = informational_entropy(samples, bitmask_samples);

    // for(std::size_t i = 0; i < bitmask_params.size();++i) {
    //   if(bi)
    // }

    std::vector<double> gain_for_param_index(bitmask_params.size());

    std::size_t amount_of_samples = bitmask_samples.count();

    std::vector<std::map<underneeth_data_type,
                         map_and_counts<underneeth_expected_result_type>>>
        vector_count_for_param(bitmask_params.size());
    {
      std::size_t index_param = bitmask_params.find_first();
      while (index_param != boost::dynamic_bitset<>::npos)
        {
          gain_for_param_index[index_param] = general_entropy;

          std::map<underneeth_data_type,
                   map_and_counts<underneeth_expected_result_type>>&
              count_for_param
              = vector_count_for_param[index_param];

          std::size_t index_sample = bitmask_samples.find_first();
          while (index_sample != boost::dynamic_bitset<>::npos)
            {
              ++count_for_param
                    [samples.at(index_sample).getValue().at(index_param)]
                        .map[samples.at(index_sample).getExpectedResult()];
              ++count_for_param
                    [samples.at(index_sample).getValue().at(index_param)]
                        .sum_of_counts;

              index_sample = bitmask_samples.find_next(index_sample);
            }

          for (auto const& [param_value,
                            struct_map_result_counts_and_sum_of_all_counts] :
               count_for_param)
            {
              gain_for_param_index[index_param]
                  -= (static_cast<double>(
                          struct_map_result_counts_and_sum_of_all_counts
                              .sum_of_counts)
                      / static_cast<double>(amount_of_samples))
                     * informational_entropy(
                         struct_map_result_counts_and_sum_of_all_counts.map,
                         struct_map_result_counts_and_sum_of_all_counts
                             .sum_of_counts);
            }

          index_param = bitmask_params.find_next(index_param);
        }
    }

    std::size_t index_param = bitmask_params.find_first();
    // we know that at least one param is not masked at this moment.
    double max_gain = gain_for_param_index[index_param];
    std::size_t best_index = index_param;

    while (index_param != boost::dynamic_bitset<>::npos)
      {
        double a_gain = gain_for_param_index[index_param];
        if (a_gain > max_gain)
          {
            max_gain = a_gain;
            best_index = index_param;
          }

        index_param = bitmask_params.find_next(index_param);
      }

    return std::make_tuple(best_index, vector_count_for_param[best_index]);
  }

  template <std::ranges::bidirectional_range RangeT>
  /* std::unique_ptr<INode<RangeT, ResultType>> */ auto id3(
      RangeT const& samples, const boost::dynamic_bitset<>& bitmask_samples,
      const boost::dynamic_bitset<>& bitmask_params,
      std::vector<
          std::map<typename std::remove_cvref_t<
                       decltype(std::declval<std::iter_value_t<
                                    std::ranges::iterator_t<RangeT>>>()
                                    .getValue())>::value_type,
                   std::map<std::remove_cvref_t<
                                decltype(std::declval<std::iter_value_t<
                                             std::ranges::iterator_t<RangeT>>>()
                                             .getExpectedResult())>,
                            std::size_t>>> const& tables)
      -> std::unique_ptr<INode<
          decltype(std::declval<
                       std::iter_value_t<std::ranges::iterator_t<RangeT>>>()
                       .getValue()),
          std::remove_cvref_t<decltype(std::declval<std::iter_value_t<
                                           std::ranges::iterator_t<RangeT>>>()
                                           .getExpectedResult())>>>

  {
    using underneeth_sample_type
        = std::iter_value_t<std::ranges::iterator_t<RangeT>>;
    using underneeth_data_collection
        = decltype(std::declval<underneeth_sample_type>().getValue());
    using underneeth_data_type =
        typename std::remove_cvref_t<underneeth_data_collection>::value_type;
    using underneeth_expected_result_type = std::remove_cvref_t<
        decltype(std::declval<underneeth_sample_type>().getExpectedResult())>;

    // case 1

    std::multiset<underneeth_expected_result_type> all_uniqueS;

    {
      std::size_t index = bitmask_samples.find_first();
      while (index != boost::dynamic_bitset<>::npos)
        {
          all_uniqueS.insert(samples.at(index).getExpectedResult());
          index = bitmask_samples.find_next(index);
        }
    }
    std::println("is case 1 ?: {} , size: {} count_first: {}",
                 *all_uniqueS.cbegin(), all_uniqueS.size(),
                 all_uniqueS.count(*all_uniqueS.cbegin()));

    if (all_uniqueS.size() == all_uniqueS.count(*all_uniqueS.cbegin()))
      {
        std::println("case 1: {} , size: {} count_first: {}",
                     *all_uniqueS.cbegin(), all_uniqueS.size(),
                     all_uniqueS.count(*all_uniqueS.cbegin()));
        return static_cast<std::unique_ptr<INode<
            underneeth_data_collection, underneeth_expected_result_type>>>(
            std::make_unique<JustResult<underneeth_data_collection,
                                        underneeth_expected_result_type>>(
                *all_uniqueS.cbegin()));
      }

    // case 2

    {
      std::size_t index = bitmask_params.find_first();
      if (index == boost::dynamic_bitset<>::npos)
        {
          auto it_element_with_max_count = std::ranges::max_element(
              all_uniqueS,
              [&all_uniqueS](auto const& a_result_1, auto const& a_result_2) {
                return std::max(all_uniqueS.count(a_result_1),
                                all_uniqueS.count(a_result_2));
              });
          std::println("case 2: {}", *it_element_with_max_count);
          return static_cast<std::unique_ptr<INode<
              underneeth_data_collection, underneeth_expected_result_type>>>(
              std::make_unique<JustResult<underneeth_data_collection,
                                          underneeth_expected_result_type>>(
                  *it_element_with_max_count));
        }
    }

    // case 3
    std::tuple<std::size_t,
               std::map<underneeth_data_type,
                        map_and_counts<underneeth_expected_result_type>>>
        selected_shit
        = informational_criteria(samples, bitmask_samples, bitmask_params);

    auto
        [selected_param,
         map_param_val_to_struct_map_result_val_to_counts_and_sum_of_all_counts]
        = selected_shit;

    std::println("case 3: param index: {}", selected_param);

    std::map<underneeth_data_type,
             std::unique_ptr<INode<underneeth_data_collection,
                                   underneeth_expected_result_type>>>
        tree;

    for (auto const& [param_val, map_result_count] : tables[selected_param])
      {
        if (map_param_val_to_struct_map_result_val_to_counts_and_sum_of_all_counts
                .contains(param_val))
          {
            boost::dynamic_bitset<> filtered_bitmask_params(bitmask_params);
            filtered_bitmask_params[selected_param] = false;

            // because of the if statement, we can be sure that here will be at
            // least one sample unmasked.
            boost::dynamic_bitset<> filtered_bitmask_samples(
                bitmask_samples.size());

            std::size_t index = bitmask_samples.find_first();
            while (index != boost::dynamic_bitset<>::npos)
              {
                // std::println("index: {} , param_val: {} , sample_param_val:
                // {}",index,param_val,samples.at(index).getValue().at(selected_param));
                if (samples.at(index).getValue().at(selected_param)
                    == param_val)
                  {
                    // std::println("setting at index {} true",index);
                    filtered_bitmask_samples[index] = true;
                  }

                index = bitmask_samples.find_next(index);
              }

            std::string bitmask_params_str;
            std::string bitmask_samples_str;
            boost::to_string(filtered_bitmask_params, bitmask_params_str);
            boost::to_string(filtered_bitmask_samples, bitmask_samples_str);

            std::println(
                "case 3.2: param_index {} , param value: {} , bitmask_samples: "
                "{} , "
                "bitmask_params: {}",
                selected_param, param_val, bitmask_samples_str,
                bitmask_params_str);

            tree[param_val] = id3(samples, filtered_bitmask_samples,
                                  filtered_bitmask_params, tables);
          }
        else
          {
            auto it_element_with_max_count = std::ranges::max_element(
                all_uniqueS,
                [&all_uniqueS](auto const& a_result1, auto const& a_result2) {
                  return std::max(all_uniqueS.count(a_result1),
                                  all_uniqueS.count(a_result2));
                });
            std::println(
                "case 3.1: param_index {} , param value: {} element: {}",
                selected_param, param_val, *it_element_with_max_count);
            tree[param_val] = static_cast<std::unique_ptr<INode<
                underneeth_data_collection, underneeth_expected_result_type>>>(
                std::make_unique<JustResult<underneeth_data_collection,
                                            underneeth_expected_result_type>>(
                    *it_element_with_max_count));
          }
      }

    return static_cast<std::unique_ptr<
        INode<underneeth_data_collection, underneeth_expected_result_type>>>(
        std::make_unique<
            Tree<underneeth_data_collection, underneeth_expected_result_type>>(
            std::move(tree), selected_param));
  }

  template <std::ranges::bidirectional_range RangeT> [[nodiscard]] inline auto
  create_model(RangeT a_range) noexcept(false) -> std::unique_ptr<
      INode<decltype(std::declval<
                         std::iter_value_t<std::ranges::iterator_t<RangeT>>>()
                         .getValue()),
            std::remove_cvref_t<decltype(std::declval<std::iter_value_t<
                                             std::ranges::iterator_t<RangeT>>>()
                                             .getExpectedResult())>>>
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
    std::map<underneeth_expected_result_type, std::size_t> unique_results{};
    std::ranges::for_each(a_range,
                          [&unique_results](const auto& sample) mutable {
                            ++unique_results[sample.getExpectedResult()];
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
                /* auto [ iterator, is_new_inserted] = */ row_map.try_emplace(
                    expected_result_key, std::size_t{});
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

    boost::dynamic_bitset<> bitmask_samples(a_range.size());
    bitmask_samples.set();

    boost::dynamic_bitset<> bitmask_params(amount_of_parameters);
    bitmask_params.set();

    std::cout << "bitmask_samples: " << bitmask_samples << '\n';
    std::cout << "bitmask_params: " << bitmask_params << '\n';

    return id3(a_range, bitmask_samples, bitmask_params, tables);
  }

}  // namespace datamining_lab1::decision_trees

#endif  // INCLUDE_DATAMINING_LAB1NAIVE_BAYESNAIVE_BAYES_HPP_
