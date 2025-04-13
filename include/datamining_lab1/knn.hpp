#ifndef INCLUDE_DATAMINING_LAB1KNNKNN_HPP_
#define INCLUDE_DATAMINING_LAB1KNNKNN_HPP_

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <map>
#include <queue>
#include <ranges>
#include <stdexcept>
#include <vector>
#include <type_traits>

#include "datamining_lab1/common.hpp"
#include <boost/math/tools/norms.hpp>

namespace datamining_lab1::knn
{
  template <std::ranges::bidirectional_range RangeSamplesDataT>
    requires Con_HasgetValueReturnsRange<
                 std::iter_value_t<std::ranges::iterator_t<RangeSamplesDataT>>>
             and Con_HasgetExpectedResult<
                 std::iter_value_t<std::ranges::iterator_t<RangeSamplesDataT>>>
  class model
  {
    RangeSamplesDataT m_data;
    std::size_t m_k;

    using underneeth_sample_type
        = std::iter_value_t<std::ranges::iterator_t<RangeSamplesDataT>>;
    using underneeth_data_collection
        = decltype(std::declval<underneeth_sample_type>().getValue());
    using underneeth_data_type =
        typename std::remove_cvref_t<underneeth_data_collection>::value_type;
    using underneeth_expected_result_type = std::remove_cvref_t<
        decltype(std::declval<underneeth_sample_type>().getExpectedResult())>;

  public:
    explicit model(RangeSamplesDataT data, std::size_t a_k)
        : m_data(std::move(data)), m_k(a_k)
    {
      if (m_k > m_data.size())
        {
          throw(
              std::logic_error("Parameter K cannot be less than size of data "
                               "in discrete KNN algorithm."));
        }
    }

    underneeth_expected_result_type predict(
        underneeth_data_collection const& sample_to_predict) noexcept
    {
      struct pos_distance
      {
        double distance;
        std::size_t pos;
      };

      auto cmp = [](pos_distance const& pos_dis1,
                    pos_distance const& pos_dis2) -> bool {
        return pos_dis1.distance > pos_dis2.distance;
      };
      std::priority_queue<pos_distance, std::vector<pos_distance>,
                          decltype(cmp)>
          k_min_elements(cmp);

      for (std::size_t pos = 0; pos < m_k; ++pos)
        {
          pos_distance tmp;
          tmp.distance = boost::math::tools::l2_distance(m_data[pos].getValue(),
                                                         sample_to_predict);
          tmp.pos = pos;
          k_min_elements.push(tmp);
        }
      for (std::size_t pos = m_k; pos < m_data.size(); ++pos)
        {
          pos_distance pos_dis{.distance = boost::math::tools::l2_distance(
                                   m_data[pos].getValue(), sample_to_predict),
                               .pos = pos};
          if (pos_dis.distance > k_min_elements.top().distance)
            {
              k_min_elements.pop();
              k_min_elements.push(pos_dis);
            }
        }

      std::map<underneeth_expected_result_type, double> smth;

      for (std::size_t i = k_min_elements.size(); i != 0; --i)
        {
          smth[m_data[k_min_elements.top().pos].getExpectedResult()]
              += 1.0
                 / (k_min_elements.top().distance
                    * k_min_elements.top().distance);
        }

      return (*std::max_element(
                  smth.cbegin(), smth.cend(),
                  [](std::pair<underneeth_expected_result_type, double> const&
                         pair1,
                     std::pair<underneeth_expected_result_type, double> const&
                         pair2) -> bool {
                    return pair1.second < pair2.second;
                  }))
          .first;
    }
  };

  template <std::ranges::bidirectional_range RangeT>
  [[nodiscard]] inline auto create_model(RangeT&& a_range,
                                         std::size_t p_k) noexcept(false)
    requires Con_HasgetValueReturnsRange<
                 std::iter_value_t<std::ranges::iterator_t<RangeT>>>
             and Con_HasgetExpectedResult<
                 std::iter_value_t<std::ranges::iterator_t<RangeT>>>
  {
    if (a_range.size() == 0)
      {
        throw std::logic_error("U gave no data. What is wrong with you?");
      }

    return model(std::forward<RangeT>(a_range), p_k);
  }

}  // namespace datamining_lab1::knn

#endif  // INCLUDE_DATAMINING_LAB1KNNKNN_HPP_
