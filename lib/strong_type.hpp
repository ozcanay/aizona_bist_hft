#pragma once

#include <type_traits>

namespace algocor
{

template<typename T, typename Tag>
struct StrongType {
    static_assert(std::is_integral_v<T>);
    static_assert(std::is_trivial_v<T> && std::is_standard_layout_v<T>);
    T val;

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    operator T() const
    {
        return val;
    }

    [[nodiscard]] T get() const
    {
        return val;
    }
};

}  // namespace algocor
