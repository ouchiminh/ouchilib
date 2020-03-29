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

constexpr dimensionless_t dimensionless{};
constexpr meter_t meter{};
constexpr gram_t gram{};
constexpr kilogram_t kilogram{};
constexpr second_t second{};
constexpr ampere_t ampere{};
constexpr kelvin_t kelvin{};
constexpr mole_t mole{};
constexpr candela_t candela{};

constexpr auto hertz = dimensionless / second;
constexpr auto newton = meter * kilogram / (second * second);
constexpr auto pascal = newton / (meter * meter);
constexpr auto joule = newton * meter;
constexpr auto watt = joule / second;
constexpr auto coulomb = second * ampere;
constexpr auto volt = watt / ampere;
constexpr auto farad = coulomb / volt;
constexpr auto ohm = volt / ampere;
constexpr auto siemens = ampere / volt;
constexpr auto weber = volt * second;
constexpr auto tesla = weber* second;
constexpr auto henry = weber / ampere;
constexpr auto lumen = candela / dimensionless;
constexpr auto lux = lumen / (meter * meter);
constexpr auto becquerel = dimensionless / second;
constexpr auto gray = joule / kilogram;
constexpr auto sievert = joule / kilogram;
constexpr auto katal = mole / second;

}

