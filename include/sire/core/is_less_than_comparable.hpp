#ifndef SIRE_IS_LESS_THAN_COMPARABLE_HPP_
#define SIRE_IS_LESS_THAN_COMPARABLE_HPP_
#include <type_traits>
namespace sire::core {
// Ĭ������� ����T������false_type
template <typename T, typename = void>
struct is_less_than_comparable : std::false_type {};

// ��� T ��С�ںŵķ������򷵻�true_type
template <typename T>
struct is_less_than_comparable<
    T, typename std::enable_if_t<
           true, decltype(std::declval<T&>() < std::declval<T&>(), (void)0)>>
    : std::true_type {};
}  // namespace sire::core
#endif