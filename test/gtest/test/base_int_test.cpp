#include "gtest/gtest.h"
#include "utils/base_int.h"

class SafeIntegerOpeTest : public testing::Test
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

private:
	int64_t argLeft_ = 9223372036854775806;
	int64_t argRight_ = 1;
	int64_t result_ = 0;

protected:
	void resetArgs(int64_t x, int64_t y, int64_t z);
	void UT_SafeIntAdd();
	void UT_SafeIntSub();
	void UT_SafeIntMul();
};

TEST_F(SafeIntegerOpeTest, UT_SafeIntAdd){ UT_SafeIntAdd(); }
TEST_F(SafeIntegerOpeTest, UT_SafeIntSub){ UT_SafeIntSub(); }
TEST_F(SafeIntegerOpeTest, UT_SafeIntMul){ UT_SafeIntMul(); }

void SafeIntegerOpeTest::resetArgs(int64_t x, int64_t y, int64_t z){
	argLeft_ = x;
	argRight_ = y;
	result_ = z;
}

void SafeIntegerOpeTest::UT_SafeIntAdd()
{
	resetArgs(9223372036854775806, 1, 0);
	EXPECT_EQ(utils::SafeIntAdd(argLeft_, argRight_, result_), true);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs(324524352354, 2345, 0);
	EXPECT_EQ(utils::SafeIntAdd(argLeft_, argRight_, result_), true);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);
	
	resetArgs(9223372036854775807, 1, 0);
	EXPECT_EQ(utils::SafeIntAdd(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs((-9223372036854775807) - 1, 1, 0);
	EXPECT_EQ(utils::SafeIntAdd(argLeft_, argRight_, result_), true);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs(-94775807, 1, 0);
	EXPECT_EQ(utils::SafeIntAdd(argLeft_, argRight_, result_), true);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs((-9223372036854775807) - 1, -1, 0);
	EXPECT_EQ(utils::SafeIntAdd(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs((-9223372036854775807) - 1, (-9223372036854775807) - 1, 0);
	EXPECT_EQ(utils::SafeIntAdd(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);
}

void SafeIntegerOpeTest::UT_SafeIntSub()
{
	resetArgs(9223372036854775807, 1, 0);
	EXPECT_EQ(utils::SafeIntSub(argLeft_, argRight_, result_), true);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs(9223372036854775807, -1, 0);
	EXPECT_EQ(utils::SafeIntSub(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs(9223372036854775807, -9223372036854775807, 0);
	EXPECT_EQ(utils::SafeIntSub(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs((-9223372036854775807) - 1, -1, 0);
	EXPECT_EQ(utils::SafeIntSub(argLeft_, argRight_, result_), true);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs((-9223372036854775807) - 1, 1, 0);
	EXPECT_EQ(utils::SafeIntSub(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs((-9223372036854775807) - 1, -3372036854775807, 0);
	EXPECT_EQ(utils::SafeIntSub(argLeft_, argRight_, result_), true);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);
}

void SafeIntegerOpeTest::UT_SafeIntMul()
{
	resetArgs(6854775, 2, 0);
	EXPECT_EQ(utils::SafeIntMul(argLeft_, argRight_, result_), true);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs(9223372036854775807, 2, 0);
	EXPECT_EQ(utils::SafeIntMul(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs(9223372036854775807, -1, 0);
	EXPECT_EQ(utils::SafeIntMul(argLeft_, argRight_, result_), true);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs(9223372036854775807, -2, 0);
	EXPECT_EQ(utils::SafeIntMul(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs((-9223372036854775807) - 1, -1, 0);
	EXPECT_EQ(utils::SafeIntMul(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs((-9223372036854775807) - 1, -2, 0);
	EXPECT_EQ(utils::SafeIntMul(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs((-9223372036854775807) - 1, 1, 0);
	EXPECT_EQ(utils::SafeIntMul(argLeft_, argRight_, result_), true);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);

	resetArgs((-9223372036854775807) - 1, 2, 0);
	EXPECT_EQ(utils::SafeIntMul(argLeft_, argRight_, result_), false);
	printf("a = %lld, b = %lld, r = %lld\n", argLeft_, argRight_, result_);
}