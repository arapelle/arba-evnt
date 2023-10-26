#include <arba/evnt/version.hpp>
#include <gtest/gtest.h>
#include <format>
#include <cstdlib>

TEST(project_version_tests, test_version_macros)
{
    const unsigned major = 0;
    const unsigned minor = 5;
    const unsigned patch = 0;
    ASSERT_EQ(ARBA_EVNT_VERSION_MAJOR, major);
    ASSERT_EQ(ARBA_EVNT_VERSION_MINOR, minor);
    ASSERT_EQ(ARBA_EVNT_VERSION_PATCH, patch);
    ASSERT_EQ(ARBA_EVNT_VERSION, std::format("{}.{}.{}", major, minor, patch));
}
