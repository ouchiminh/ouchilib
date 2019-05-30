#include "../test.hpp"
#include "ouchilib/program_options/program_options.hpp"

DEFINE_TEST(test_program_options_description)
{
    namespace opo = ouchi::program_options;
    opo::basic_options_description<char> d;
    // instantiate test
    d.add("key", "something", opo::default_value = 12, opo::single<int>);

    CHECK_EQUAL(1, d.get_container().size());
}
