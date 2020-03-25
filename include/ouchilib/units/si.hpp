#pragma once
#include "units.hpp"

namespace ouchi::units::si {
namespace detail {
struct si_tag {};
}

using dimensionless         = basic_system_of_units<detail::si_tag, 0, 0, 0, 0, 0, 0, 0>;
using length                = basic_system_of_units<detail::si_tag, 1, 0, 0, 0, 0, 0, 0>;
using mass                  = basic_system_of_units<detail::si_tag, 0, 1, 0, 0, 0, 0, 0>;
using time                  = basic_system_of_units<detail::si_tag, 0, 0, 1, 0, 0, 0, 0>;
using intensity             = basic_system_of_units<detail::si_tag, 0, 0, 0, 1, 0, 0, 0>;
using temperature           = basic_system_of_units<detail::si_tag, 0, 0, 0, 0, 1, 0, 0>;
using amount                = basic_system_of_units<detail::si_tag, 0, 0, 0, 0, 0, 1, 0>;
using luminous_intensity    = basic_system_of_units<detail::si_tag, 0, 0, 0, 0, 0, 0, 1>;


inline constexpr dimensionless         dimensionless_quantity;
inline constexpr length                meter;
inline constexpr mass                  kg;
inline constexpr time                  seconds;
inline constexpr intensity             A;
inline constexpr temperature           K;
inline constexpr amount                mol;
inline constexpr luminous_intensity    cd;
}

