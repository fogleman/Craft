// BASIC INCLUDES //
#include <gtest/gtest.h>
#include <stdbool.h>
#include <math.h>

//never include c files, you can rename main.c to craftma
//TESTING FILE INCLUDES //
//#include "../src/deleteme.c"
//#include "../src/main.c" <--- SO MANY ERRORS!!!!!!!!!!!!!.

extern "C" bool helloWorld;
extern "C" bool func_1();

/// Externals used for testing [Issue #7]: https://github.com/Team-10-But-Better/Craft/issues/7
extern "C" double mouseDPIMultiplier;
extern "C" double keyDPIMultiplier;
extern "C" void incrementDPI();
extern "C" void decrementDPI();
extern "C" void setDefaultDPI();

/// Externals used for testing [Issue #41]: https://github.com/Team-10-But-Better/Craft/issues/41
extern "C" int isDefinedCraftKey(char *str, int start, int end);

//----------------------------------------------------
TEST(Assert, empty)
{
	GTEST_ASSERT_EQ(true, true);
}

//----------------------------------------------------
TEST(tests, issueFoo)
{
	EXPECT_EQ(helloWorld, true);
}

TEST(tests, issueBar)
{
	bool returnValue = func_1;
	EXPECT_EQ(true, returnValue);
}

TEST(tests, issue7)
{
	setDefaultDPI();

	// Creation of variables used for comparison
	double initialMouseDPIMultiplier = mouseDPIMultiplier;
	double initialKeyDPIMultiplier = keyDPIMultiplier;

	// Testing of incrementation of DPI
	// Test one incrementation
	incrementDPI();
	EXPECT_EQ(initialMouseDPIMultiplier + .0001, mouseDPIMultiplier) << "Issue 7 Testing Error 1: mouseDPIMultiplier was not incremented by .0001\n";
	EXPECT_EQ(initialKeyDPIMultiplier + .1, keyDPIMultiplier) << "Issue 7 Testing Error 2: keyDPIMultiplier was not incremented by .1\n";

	// Testing max incrementations
	for (int i = 0; i < 10; i++)
		incrementDPI();
	EXPECT_EQ(round(initialMouseDPIMultiplier + .0009), round(mouseDPIMultiplier)) << "Issue 7 Testing Error 3: mouseDPIMultiplier was not equal to 1\n";
	EXPECT_EQ(round(initialKeyDPIMultiplier + .9), round(keyDPIMultiplier)) << "Issue 7 Testing Error 4: keyDPIMultiplier was not equal to 2\n";

	// Testing resetting DPIs to default values
	setDefaultDPI();
	EXPECT_EQ(initialMouseDPIMultiplier, mouseDPIMultiplier) << "Issue 7 Testing Error 5: mouseDPIMultiplier was not equal to .0001\n";
	EXPECT_EQ(initialKeyDPIMultiplier, keyDPIMultiplier) << "Issue 7 Testing Error 6: keyDPIMultiplier was not equal to .1\n";

	// Testing one decrementation
	decrementDPI();
	EXPECT_EQ(initialMouseDPIMultiplier - .0001, mouseDPIMultiplier) << "Issue 7 Testing Error 7: mouseDPIMultiplier was not decremented by .0001\n";
	EXPECT_EQ(initialKeyDPIMultiplier - .1, keyDPIMultiplier) << "Issue 7 Testing Error 8: keyDPIMultiplier was not decremented by .1\n";

	// Testing max decrementation
	decrementDPI();
	EXPECT_EQ(initialMouseDPIMultiplier - .0001, mouseDPIMultiplier) << "Issue 7 Testing Error 9: mouseDPIMultiplier was not equal to 0\n";
	EXPECT_EQ(initialKeyDPIMultiplier - .1, keyDPIMultiplier) << "Issue 7 Testing Error 10: keyDPIMultiplier was not equal to 1\n";
}

TEST(tests, issue41)
{
	// Testing a line of text that does not define a CRAFT keybind
	EXPECT_EQ(isDefinedCraftKey("This is not a defined craft key", 8, 13), 0) << "Issue 41 Testing Error 1: test found string to be a defined CRAFT keybind\n";

	// Testing a line of text that does define a CRAFT keybind
	EXPECT_EQ(isDefinedCraftKey("#define CRAFT_KEY_TESTING_ISSUE_41", 8, 13), 1) << "Issue 41 Testing Error 2: test found string to not be a defined CRAFT keybind\n";
}
extern "C" int craft_main(int argc, char *argv[]);

//----------------------------------------------------
int main(int argc, char *argv[])
{
	craft_main(argc, argv);
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
