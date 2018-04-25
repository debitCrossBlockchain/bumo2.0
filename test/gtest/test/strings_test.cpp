#include "gtest/gtest.h"
#include "utils/base_int.h"

class UtilsStringsTest : public testing::Test
{
protected:

	// Sets up the test fixture.
	virtual void SetUp()
	{
	}

	// Tears down the test fixture.
	virtual void TearDown()
	{

	}

protected:
	void UT_SafeStoi64();
	void UT_SafeStoui64();
	void UT_SafeStoi();
	void UT_SafeStoui();
};

TEST_F(UtilsStringsTest, UT_SafeStoi64){ UT_SafeStoi64(); }
TEST_F(UtilsStringsTest, UT_SafeStoui64){ UT_SafeStoui64(); }
TEST_F(UtilsStringsTest, UT_SafeStoi){ UT_SafeStoi(); }
TEST_F(UtilsStringsTest, UT_SafeStoui){ UT_SafeStoui(); }

void UtilsStringsTest::UT_SafeStoi64()
{
	int64_t num = 0;
	EXPECT_EQ(utils::String::SafeStoi64("92233720368547758070", num), false);
	printf("num = %lld\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoi64("-92233720368547758080", num), false);
	printf("num = %lld\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoi64("9223372036854775807", num), true);
	printf("num = %lld\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoi64("9223372036854775808", num), false);
	printf("num = %lld\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoi64("-9223372036854775808", num), true);
	printf("num = %lld\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoi64("-9223372036854775809", num), false);
	printf("num = %lld\n", num);
}

void UtilsStringsTest::UT_SafeStoui64()
{
	uint64_t num = 0;
	EXPECT_EQ(utils::String::SafeStoui64("18446744073709551615", num), true);
	printf("num = %llu\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoui64("184467440737095516150", num), false);
	printf("num = %llu\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoui64("18446744073709551616", num), false);
	printf("num = %llu\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoui64("0", num), true);
	printf("num = %llu\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoui64("-1", num), false);
	printf("num = %llu\n", num);
}

void UtilsStringsTest::UT_SafeStoi()
{
	int num = 0;
	EXPECT_EQ(utils::String::SafeStoi("2147483647", num), true);
	printf("num = %d\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoi("21474836470", num), false);
	printf("num = %d\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoi("2147483648", num), false);
	printf("num = %d\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoi("-2147483648", num), true);
	printf("num = %d\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoi("-21474836480", num), false);
	printf("num = %d\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoi("-2147483649", num), false);
	printf("num = %d\n", num);
}

void UtilsStringsTest::UT_SafeStoui()
{
	unsigned int num = 0;
	EXPECT_EQ(utils::String::SafeStoui("4294967295", num), true);
	printf("num = %u\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoui("42949672950", num), false);
	printf("num = %u\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoui("4294967296", num), false);
	printf("num = %u\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoui("0", num), true);
	printf("num = %u\n", num);

	num = 0;
	EXPECT_EQ(utils::String::SafeStoui("-1", num), false);
	printf("num = %u\n", num);
}
