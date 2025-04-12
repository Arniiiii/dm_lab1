#ifndef INCLUDE_DATAMINING_LAB1_COMMON_HPP
#define INCLUDE_DATAMINING_LAB1_COMMON_HPP

#include <ranges>
#include <type_traits>
namespace datamining_lab1
{
  template <typename T>
  concept is_void_type = std::is_void_v<T>;

  template <typename T>
  concept is_not_void_type = !is_void_type<T>;

  template <typename T>
  concept Con_HasgetValueReturnsRange = requires(T an_object) {
    { an_object.getValue() } -> std::ranges::bidirectional_range;
  };

  template <typename T>
  concept Con_HasgetExpectedResult = requires(T an_object) {
    { an_object.getExpectedResult() } -> is_not_void_type;
  };

  template <typename T, typename U, typename V>
  concept Con_Hadpredict = requires(T an_object, U something) {
    { an_object.predict(something) } -> std::same_as<V>;
  };

}  // namespace datamining_lab1

#endif  // !INCLUDE_DATAMINING_LAB1_COMMON_HPP
