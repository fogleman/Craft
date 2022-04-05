// BASIC INCLUDES //
#include <gtest/gtest.h>
#include <stdbool.h>

//never include c files, you can rename main.c to craftma
//TESTING FILE INCLUDES //
//#include "../src/deleteme.c"
//#include "../src/main.c" <--- SO MANY ERRORS!!!!!!!!!!!!!.

extern "C" bool helloWorld;
extern "C" bool func_1();
//----------------------------------------------------
TEST(Assert, empty) { GTEST_ASSERT_EQ(true, true); }

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

extern "C" int craft_main(int argc, char *argv[]);

///[issue]https://github.com/Team-10-But-Better/Craft/issues/46
extern "C" int getZAxisChange(bool forward, bool backward);
extern "C" int getXAxisChange(bool left, bool right);
extern "C" float getHorizontalCameraChange(bool left, bool right, float m);
extern "C" float getVerticalCameraChange(bool up, bool down, float m);
extern "C" int getOrtho(bool ortho);
extern "C" int getFOV(bool fov);
TEST(tests, issue46)
{
	EXPECT_EQ(getZAxisChange(false, false), 0);
	EXPECT_EQ(getZAxisChange(true, false), -1);
	EXPECT_EQ(getZAxisChange(false, true), 1);
	EXPECT_EQ(getZAxisChange(true, true), 0);
	EXPECT_EQ(getXAxisChange(false, false), 0);
	EXPECT_EQ(getXAxisChange(true, false), -1);
	EXPECT_EQ(getXAxisChange(false, true), 1);
	EXPECT_EQ(getXAxisChange(true, true), 0);
	EXPECT_FLOAT_EQ(getHorizontalCameraChange(false, false, 1.0), 0.0);
	EXPECT_FLOAT_EQ(getHorizontalCameraChange(true, false, 1.0), -1.0);
	EXPECT_FLOAT_EQ(getHorizontalCameraChange(false, true, 1.0), 1.0);
	EXPECT_FLOAT_EQ(getHorizontalCameraChange(true, true, 1.0), 0.0);
	EXPECT_FLOAT_EQ(getVerticalCameraChange(false, false, 1.0), 0.0);
	EXPECT_FLOAT_EQ(getVerticalCameraChange(true, false, 1.0), 1.0);
	EXPECT_FLOAT_EQ(getVerticalCameraChange(false, true, 1.0), -1.0);
	EXPECT_FLOAT_EQ(getVerticalCameraChange(true, true, 1.0), 0.0);
	EXPECT_EQ(getOrtho(true), 64);
	EXPECT_EQ(getOrtho(false), 0);
	EXPECT_EQ(getFOV(true), 15);
	EXPECT_EQ(getFOV(false), 65);
}

//----------------------------------------------------
int main(int argc, char *argv[])
{
	craft_main(argc, argv);
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
