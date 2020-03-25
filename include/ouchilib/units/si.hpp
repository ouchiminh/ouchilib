#pragma once
#include "units.hpp"

namespace ouchi::units::si {
namespace detail {
struct si_tag {};
}
//                                                                  L  M  T  I Th  N  J
using dimensionless         = basic_system_of_units<detail::si_tag, 0, 0, 0, 0, 0, 0, 0>;
using length                = basic_system_of_units<detail::si_tag, 1, 0, 0, 0, 0, 0, 0>;
using mass                  = basic_system_of_units<detail::si_tag, 0, 1, 0, 0, 0, 0, 0>;
using time                  = basic_system_of_units<detail::si_tag, 0, 0, 1, 0, 0, 0, 0>;
using intensity             = basic_system_of_units<detail::si_tag, 0, 0, 0, 1, 0, 0, 0>;
using temperature           = basic_system_of_units<detail::si_tag, 0, 0, 0, 0, 1, 0, 0>;
using amount                = basic_system_of_units<detail::si_tag, 0, 0, 0, 0, 0, 1, 0>;
using luminous_intensity    = basic_system_of_units<detail::si_tag, 0, 0, 0, 0, 0, 0, 1>;

inline constexpr dimensionless         dimensionless_quantity{};
inline constexpr length                m{};
inline constexpr mass                  kg{};
inline constexpr time                  s{};
inline constexpr intensity             A{};
inline constexpr temperature           K{};
inline constexpr amount                mol{};
inline constexpr luminous_intensity    cd{};

using angle                 = dimensionless;
inline constexpr angle      rad{};
using solid_angle           = dimensionless;
inline constexpr solid_angle sr{};
using frequency             = basic_system_of_units<detail::si_tag, 0, 0, -1, 0, 0, 0, 0>;
inline constexpr frequency  Hz{};
using force                 = basic_system_of_units<detail::si_tag, 1, 1, -2, 0, 0, 0, 0>;
inline constexpr force      N{};
using pressure              = decltype(N / m / m);
inline constexpr pressure   Pa{};
using energy                = decltype(N * m);
inline constexpr energy     J{};
using power                 = decltype(J / s);
inline constexpr power      W{};
using electric_charge       = decltype(s * A);
inline constexpr electric_charge C{};
using voltage               = decltype(W / A);
inline constexpr voltage    V{};
using capacitance           = decltype(C / V);
using electrical_resistance = decltype(V / A);
inline constexpr electrical_resistance Ohm{};
using conductance           = typename electrical_resistance::inv_t;
inline constexpr conductance S{};
using magnetic_flux         = decltype(V* s);
inline constexpr magnetic_flux Wb{};
using magnetic_field        = decltype(Wb / m / m);
inline constexpr magnetic_field T{};
using inductance            = decltype(Wb / A);
inline constexpr inductance H{};
using luminous_flux         = luminous_intensity;
inline constexpr luminous_flux lm{};
using illuminance           = decltype(lm / m / m);
inline constexpr illuminance lx{};
using radioactivity         = typename time::inv_t;
inline constexpr radioactivity Bq;
using absorbed_dose         = decltype(J / kg);
inline constexpr absorbed_dose Gy{};
using dose_equivalent       = decltype(J / kg);
inline constexpr dose_equivalent Sv{};
using enzyme_activity       = decltype(mol / s);
inline constexpr enzyme_activity Kat{};
}

