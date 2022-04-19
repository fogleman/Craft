// BASIC INCLUDES //
#include <gtest/gtest.h>
#include <stdbool.h>

//never include c files, you can rename main.c to craftma
//TESTING FILE INCLUDES //
//#include "../src/deleteme.c"
//#include "../src/main.c" <--- SO MANY ERRORS!!!!!!!!!!!!!.


extern "C" int item_count;


//----------------------------------------------------
TEST(tests, issue13){
	EXPECT_EQ(item_count, 63);
}

extern "C" int craft_main(int argc, char* argv[]);

//----------------------------------------------------
int main(int argc, char* argv[])
{
	craft_main(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
