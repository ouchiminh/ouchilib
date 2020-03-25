#pragma once
#include "units.hpp"

namespace ouchi::units::si {
namespace detail {
struct si_tag {};
}

using length                = system_of_units_base<detail::si_tag, 1, 0, 0, 0, 0, 0, 0>;
using mass                  = system_of_units_base<detail::si_tag, 0, 1, 0, 0, 0, 0, 0>;
using time                  = system_of_units_base<detail::si_tag, 0, 0, 1, 0, 0, 0, 0>;
using intensity             = system_of_units_base<detail::si_tag, 0, 0, 0, 1, 0, 0, 0>;
using temperature           = system_of_units_base<detail::si_tag, 0, 0, 0, 0, 1, 0, 0>;
using amount                = system_of_units_base<detail::si_tag, 0, 0, 0, 0, 0, 1, 0>;
using luminous_intensity    = system_of_units_base<detail::si_tag, 0, 0, 0, 0, 0, 0, 1>;

}

