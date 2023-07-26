#include <gtest/gtest.h>

extern "C" {  /// necessary to link to the c code
  #include "../src/util.h"
}

/// TEST(TestSuiteName, TestName) {
///   ... test body ...
/// }

TEST(UtilTests, stringWidth) {
  ASSERT_EQ(string_width("1"), 3);
  ASSERT_EQ(string_width("2"), 6);
  ASSERT_EQ(string_width("3"), 6);
}



int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}