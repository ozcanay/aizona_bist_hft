#include <gtest/gtest.h>

TEST(DummyTest, AlwaysPasses)
{
    EXPECT_EQ(1, 1);
}

TEST(DummyTest, AlwaysFails)
{
    EXPECT_NE(1, 2);
}
