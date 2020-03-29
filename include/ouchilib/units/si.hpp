#pragma once
#include "units.hpp"

namespace ouchi::units::si {

struct length {};
struct mass {};
struct time {};
struct intensity {};
struct thermodynamic_temperature {};
struct amount {};
struct luminous_intensity {};

using si = system_of_units<length, mass, time, intensity, thermodynamic_temperature, amount, luminous_intensity>;

using dimensionless_t = si::dimensionless_t;
using meter_t = typename si::template unit_t<length>;
using gram_t = typename si::template unit_t<mass>;
using kilogram_t = typename si::template unit_t<mass, 1, std::kilo>;
using second_t = typename si::template unit_t<time>;
using ampere_t = typename si::template unit_t<intensity>;
using kelvin_t = typename si::template unit_t<thermodynamic_temperature>;
using mole_t = typename si::template unit_t<amount>;
using candela_t = typename si::template unit_t<luminous_intensity>;

inline constexpr dimensionless_t dimensionless{};
inline constexpr meter_t meter{};
inline constexpr gram_t gram{};
inline constexpr kilogram_t kilogram{};
inline constexpr second_t second{};
inline constexpr ampere_t ampere{};
inline constexpr kelvin_t kelvin{};
inline constexpr mole_t mole{};
inline constexpr candela_t candela{};

inline constexpr auto hertz = dimensionless / second;
inline constexpr auto newton = meter * kilogram / (second * second);
inline constexpr auto pascal = newton / (meter * meter);
inline constexpr auto joule = newton * meter;
inline constexpr auto watt = joule / second;
inline constexpr auto coulomb = second * ampere;
inline constexpr auto volt = watt / ampere;
inline constexpr auto farad = coulomb / volt;
inline constexpr auto ohm = volt / ampere;
inline constexpr auto siemens = ampere / volt;
inline constexpr auto weber = volt * second;
inline constexpr auto tesla = weber* second;
inline constexpr auto henry = weber / ampere;
inline constexpr auto lumen = candela / dimensionless;
inline constexpr auto lux = lumen / (meter * meter);
inline constexpr auto becquerel = dimensionless / second;
inline constexpr auto gray = joule / kilogram;
inline constexpr auto sievert = joule / kilogram;
inline constexpr auto katal = mole / second;

}

