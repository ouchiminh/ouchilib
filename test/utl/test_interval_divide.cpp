#include "../test.hpp"
#include "ouchilib/utl/interval_divide.hpp"

using namespace ouchi;

DEFINE_TEST(test_interval_divide_instantiate)
{
    interval_divide<int> id{interval_divide<int>{0, 3}};
    interval_divide<int>{id};

    interval_divide<int>{std::piecewise_construct, std::forward_as_tuple(0), std::forward_as_tuple(4)};
}

