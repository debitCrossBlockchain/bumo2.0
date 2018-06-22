#include "gtest/gtest.h"
#include "common/general.h"

/*
total reward is five hundred million, decreasing 1/4 each year.
the reward for first periord: 1.25 hundred million£¬ 125,000,000 BU COIN
a block per 10 seconds, 3153600 blocks per year, five years: 3153600 * 5 = 15768000 block

reward per block: 125,000,000  / 15768000 = 7.927447995941147

the process of inferencing for the reward of first periord:
8 bu per block, 8 * 3153600 * 5  = 126144000
*/
class decrement_value_utest : public testing::Test
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
    void UT_Calc_Block_Decrement_Vaule();
};

TEST_F(decrement_value_utest, UT_Calc_Block_Decrement_Vaule){ UT_Calc_Block_Decrement_Vaule(); }
void decrement_value_utest::UT_Calc_Block_Decrement_Vaule()
{
	//±ê×¼ÓÃÀý
	EXPECT_EQ(bumo::GetBlockReward(0 * bumo::General::REWARD_PERIOD), 800000000);
	EXPECT_EQ(bumo::GetBlockReward(1 * bumo::General::REWARD_PERIOD - 1), 800000000);
	EXPECT_EQ(bumo::GetBlockReward(1 * bumo::General::REWARD_PERIOD), 600000000);
	EXPECT_EQ(bumo::GetBlockReward(1 * bumo::General::REWARD_PERIOD + 1), 600000000);

	EXPECT_EQ(bumo::GetBlockReward(2 * bumo::General::REWARD_PERIOD - 1), 600000000);
	EXPECT_EQ(bumo::GetBlockReward(2 * bumo::General::REWARD_PERIOD), 450000000);
	EXPECT_EQ(bumo::GetBlockReward(2 * bumo::General::REWARD_PERIOD + 1), 450000000);

	EXPECT_EQ(bumo::GetBlockReward(3 * bumo::General::REWARD_PERIOD - 1), 450000000);
	EXPECT_EQ(bumo::GetBlockReward(3 * bumo::General::REWARD_PERIOD), 337500000);
	EXPECT_EQ(bumo::GetBlockReward(3 * bumo::General::REWARD_PERIOD + 1), 337500000);

	EXPECT_EQ(bumo::GetBlockReward(4 * bumo::General::REWARD_PERIOD - 1), 337500000);
	EXPECT_EQ(bumo::GetBlockReward(4 * bumo::General::REWARD_PERIOD), 253125000);
	EXPECT_EQ(bumo::GetBlockReward(4 * bumo::General::REWARD_PERIOD + 1), 253125000);

	EXPECT_EQ(bumo::GetBlockReward(5 * bumo::General::REWARD_PERIOD - 1), 253125000);
	EXPECT_EQ(bumo::GetBlockReward(5 * bumo::General::REWARD_PERIOD), 189843750);
	EXPECT_EQ(bumo::GetBlockReward(5 * bumo::General::REWARD_PERIOD + 1), 189843750);

	EXPECT_EQ(bumo::GetBlockReward(6 * bumo::General::REWARD_PERIOD - 1), 189843750);
	EXPECT_EQ(bumo::GetBlockReward(6 * bumo::General::REWARD_PERIOD), 142382813);
	EXPECT_EQ(bumo::GetBlockReward(6 * bumo::General::REWARD_PERIOD + 1), 142382813);

	EXPECT_EQ(bumo::GetBlockReward(7 * bumo::General::REWARD_PERIOD - 1), 142382813);
	EXPECT_EQ(bumo::GetBlockReward(7 * bumo::General::REWARD_PERIOD), 106787110);
	EXPECT_EQ(bumo::GetBlockReward(7 * bumo::General::REWARD_PERIOD + 1), 106787110);

	EXPECT_EQ(bumo::GetBlockReward(8 * bumo::General::REWARD_PERIOD - 1), 106787110);
	EXPECT_EQ(bumo::GetBlockReward(8 * bumo::General::REWARD_PERIOD), 80090333);
	EXPECT_EQ(bumo::GetBlockReward(8 * bumo::General::REWARD_PERIOD + 1), 80090333);

	EXPECT_EQ(bumo::GetBlockReward(9 * bumo::General::REWARD_PERIOD - 1), 80090333);
	EXPECT_EQ(bumo::GetBlockReward(9 * bumo::General::REWARD_PERIOD), 60067750);
	EXPECT_EQ(bumo::GetBlockReward(9 * bumo::General::REWARD_PERIOD + 1), 60067750);

	EXPECT_EQ(bumo::GetBlockReward(10 * bumo::General::REWARD_PERIOD - 1), 60067750);
	EXPECT_EQ(bumo::GetBlockReward(10 * bumo::General::REWARD_PERIOD), 45050813);
	EXPECT_EQ(bumo::GetBlockReward(10 * bumo::General::REWARD_PERIOD + 1), 45050813);

	EXPECT_EQ(bumo::GetBlockReward(11 * bumo::General::REWARD_PERIOD - 1), 45050813);
	EXPECT_EQ(bumo::GetBlockReward(11 * bumo::General::REWARD_PERIOD), 33788110);
	EXPECT_EQ(bumo::GetBlockReward(11 * bumo::General::REWARD_PERIOD + 1), 33788110);

	EXPECT_EQ(bumo::GetBlockReward(12 * bumo::General::REWARD_PERIOD - 1), 33788110);
	EXPECT_EQ(bumo::GetBlockReward(12 * bumo::General::REWARD_PERIOD), 25341083);
	EXPECT_EQ(bumo::GetBlockReward(12 * bumo::General::REWARD_PERIOD + 1), 25341083);

	EXPECT_EQ(bumo::GetBlockReward(13 * bumo::General::REWARD_PERIOD - 1), 25341083);
	EXPECT_EQ(bumo::GetBlockReward(13 * bumo::General::REWARD_PERIOD), 19005813);
	EXPECT_EQ(bumo::GetBlockReward(13 * bumo::General::REWARD_PERIOD + 1), 19005813);

	EXPECT_EQ(bumo::GetBlockReward(14 * bumo::General::REWARD_PERIOD - 1), 19005813);
	EXPECT_EQ(bumo::GetBlockReward(14 * bumo::General::REWARD_PERIOD), 14254360);
	EXPECT_EQ(bumo::GetBlockReward(14 * bumo::General::REWARD_PERIOD + 1), 14254360);

	EXPECT_EQ(bumo::GetBlockReward(15 * bumo::General::REWARD_PERIOD - 1), 14254360);
	EXPECT_EQ(bumo::GetBlockReward(15 * bumo::General::REWARD_PERIOD), 10690770);
	EXPECT_EQ(bumo::GetBlockReward(15 * bumo::General::REWARD_PERIOD + 1), 10690770);

	EXPECT_EQ(bumo::GetBlockReward(16 * bumo::General::REWARD_PERIOD - 1), 10690770);
	EXPECT_EQ(bumo::GetBlockReward(16 * bumo::General::REWARD_PERIOD), 8018078);
	EXPECT_EQ(bumo::GetBlockReward(16 * bumo::General::REWARD_PERIOD + 1), 8018078);


	EXPECT_EQ(bumo::GetBlockReward(16 * bumo::General::REWARD_PERIOD - 1), 10690770);
	EXPECT_EQ(bumo::GetBlockReward(16 * bumo::General::REWARD_PERIOD), 8018078);
	EXPECT_EQ(bumo::GetBlockReward(16 * bumo::General::REWARD_PERIOD + 1), 8018078);


	EXPECT_EQ(bumo::GetBlockReward(17 * bumo::General::REWARD_PERIOD - 1), 8018078);
	EXPECT_EQ(bumo::GetBlockReward(17 * bumo::General::REWARD_PERIOD), 6013559);
	EXPECT_EQ(bumo::GetBlockReward(17 * bumo::General::REWARD_PERIOD + 1), 6013559);


	EXPECT_EQ(bumo::GetBlockReward(18 * bumo::General::REWARD_PERIOD - 1), 6013559);
	EXPECT_EQ(bumo::GetBlockReward(18 * bumo::General::REWARD_PERIOD), 4510170);
	EXPECT_EQ(bumo::GetBlockReward(18 * bumo::General::REWARD_PERIOD + 1), 4510170);


	EXPECT_EQ(bumo::GetBlockReward(19 * bumo::General::REWARD_PERIOD - 1), 4510170);
	EXPECT_EQ(bumo::GetBlockReward(19 * bumo::General::REWARD_PERIOD), 3382628);
	EXPECT_EQ(bumo::GetBlockReward(19 * bumo::General::REWARD_PERIOD + 1), 3382628);


	EXPECT_EQ(bumo::GetBlockReward(20 * bumo::General::REWARD_PERIOD - 1), 3382628);
	EXPECT_EQ(bumo::GetBlockReward(20 * bumo::General::REWARD_PERIOD), 2536971);
	EXPECT_EQ(bumo::GetBlockReward(20 * bumo::General::REWARD_PERIOD + 1), 2536971);


	EXPECT_EQ(bumo::GetBlockReward(21 * bumo::General::REWARD_PERIOD - 1), 2536971);
	EXPECT_EQ(bumo::GetBlockReward(21 * bumo::General::REWARD_PERIOD), 1902729);
	EXPECT_EQ(bumo::GetBlockReward(21 * bumo::General::REWARD_PERIOD + 1), 1902729);


	EXPECT_EQ(bumo::GetBlockReward(22 * bumo::General::REWARD_PERIOD - 1), 1902729);
	EXPECT_EQ(bumo::GetBlockReward(22 * bumo::General::REWARD_PERIOD), 1427047);
	EXPECT_EQ(bumo::GetBlockReward(22 * bumo::General::REWARD_PERIOD + 1), 1427047);


	EXPECT_EQ(bumo::GetBlockReward(23 * bumo::General::REWARD_PERIOD - 1), 1427047);
	EXPECT_EQ(bumo::GetBlockReward(23 * bumo::General::REWARD_PERIOD), 1070286);
	EXPECT_EQ(bumo::GetBlockReward(23 * bumo::General::REWARD_PERIOD + 1), 1070286);


	EXPECT_EQ(bumo::GetBlockReward(24 * bumo::General::REWARD_PERIOD - 1), 1070286);
	EXPECT_EQ(bumo::GetBlockReward(24 * bumo::General::REWARD_PERIOD), 802715);
	EXPECT_EQ(bumo::GetBlockReward(24 * bumo::General::REWARD_PERIOD + 1), 802715);


	EXPECT_EQ(bumo::GetBlockReward(25 * bumo::General::REWARD_PERIOD - 1), 802715);
	EXPECT_EQ(bumo::GetBlockReward(25 * bumo::General::REWARD_PERIOD), 602037);
	EXPECT_EQ(bumo::GetBlockReward(25 * bumo::General::REWARD_PERIOD + 1), 602037);


	EXPECT_EQ(bumo::GetBlockReward(26 * bumo::General::REWARD_PERIOD - 1), 602037);
	EXPECT_EQ(bumo::GetBlockReward(26 * bumo::General::REWARD_PERIOD), 451528);
	EXPECT_EQ(bumo::GetBlockReward(26 * bumo::General::REWARD_PERIOD + 1), 451528);


	EXPECT_EQ(bumo::GetBlockReward(27 * bumo::General::REWARD_PERIOD - 1), 451528);
	EXPECT_EQ(bumo::GetBlockReward(27 * bumo::General::REWARD_PERIOD), 338646);
	EXPECT_EQ(bumo::GetBlockReward(27 * bumo::General::REWARD_PERIOD + 1), 338646);


	EXPECT_EQ(bumo::GetBlockReward(28 * bumo::General::REWARD_PERIOD - 1), 338646);
	EXPECT_EQ(bumo::GetBlockReward(28 * bumo::General::REWARD_PERIOD), 253985);
	EXPECT_EQ(bumo::GetBlockReward(28 * bumo::General::REWARD_PERIOD + 1), 253985);


	EXPECT_EQ(bumo::GetBlockReward(29 * bumo::General::REWARD_PERIOD - 1), 253985);
	EXPECT_EQ(bumo::GetBlockReward(29 * bumo::General::REWARD_PERIOD), 190489);
	EXPECT_EQ(bumo::GetBlockReward(29 * bumo::General::REWARD_PERIOD + 1), 190489);


	EXPECT_EQ(bumo::GetBlockReward(30 * bumo::General::REWARD_PERIOD - 1), 190489);
	EXPECT_EQ(bumo::GetBlockReward(30 * bumo::General::REWARD_PERIOD), 142867);
	EXPECT_EQ(bumo::GetBlockReward(30 * bumo::General::REWARD_PERIOD + 1), 142867);


	EXPECT_EQ(bumo::GetBlockReward(31 * bumo::General::REWARD_PERIOD - 1), 142867);
	EXPECT_EQ(bumo::GetBlockReward(31 * bumo::General::REWARD_PERIOD), 107151);
	EXPECT_EQ(bumo::GetBlockReward(31 * bumo::General::REWARD_PERIOD + 1), 107151);


	EXPECT_EQ(bumo::GetBlockReward(32 * bumo::General::REWARD_PERIOD - 1), 107151);
	EXPECT_EQ(bumo::GetBlockReward(32 * bumo::General::REWARD_PERIOD), 80364);
	EXPECT_EQ(bumo::GetBlockReward(32 * bumo::General::REWARD_PERIOD + 1), 80364);


	EXPECT_EQ(bumo::GetBlockReward(33 * bumo::General::REWARD_PERIOD - 1), 80364);
	EXPECT_EQ(bumo::GetBlockReward(33 * bumo::General::REWARD_PERIOD), 60273);
	EXPECT_EQ(bumo::GetBlockReward(33 * bumo::General::REWARD_PERIOD + 1), 60273);


	EXPECT_EQ(bumo::GetBlockReward(34 * bumo::General::REWARD_PERIOD - 1), 60273);
	EXPECT_EQ(bumo::GetBlockReward(34 * bumo::General::REWARD_PERIOD), 45205);
	EXPECT_EQ(bumo::GetBlockReward(34 * bumo::General::REWARD_PERIOD + 1), 45205);


	EXPECT_EQ(bumo::GetBlockReward(35 * bumo::General::REWARD_PERIOD - 1), 45205);
	EXPECT_EQ(bumo::GetBlockReward(35 * bumo::General::REWARD_PERIOD), 33904);
	EXPECT_EQ(bumo::GetBlockReward(35 * bumo::General::REWARD_PERIOD + 1), 33904);


	EXPECT_EQ(bumo::GetBlockReward(36 * bumo::General::REWARD_PERIOD - 1), 33904);
	EXPECT_EQ(bumo::GetBlockReward(36 * bumo::General::REWARD_PERIOD), 25428);
	EXPECT_EQ(bumo::GetBlockReward(36 * bumo::General::REWARD_PERIOD + 1), 25428);


	EXPECT_EQ(bumo::GetBlockReward(37 * bumo::General::REWARD_PERIOD - 1), 25428);
	EXPECT_EQ(bumo::GetBlockReward(37 * bumo::General::REWARD_PERIOD), 19071);
	EXPECT_EQ(bumo::GetBlockReward(37 * bumo::General::REWARD_PERIOD + 1), 19071);


	EXPECT_EQ(bumo::GetBlockReward(38 * bumo::General::REWARD_PERIOD - 1), 19071);
	EXPECT_EQ(bumo::GetBlockReward(38 * bumo::General::REWARD_PERIOD), 14304);
	EXPECT_EQ(bumo::GetBlockReward(38 * bumo::General::REWARD_PERIOD + 1), 14304);


	EXPECT_EQ(bumo::GetBlockReward(39 * bumo::General::REWARD_PERIOD - 1), 14304);
	EXPECT_EQ(bumo::GetBlockReward(39 * bumo::General::REWARD_PERIOD), 10728);
	EXPECT_EQ(bumo::GetBlockReward(39 * bumo::General::REWARD_PERIOD + 1), 10728);


	EXPECT_EQ(bumo::GetBlockReward(40 * bumo::General::REWARD_PERIOD - 1), 10728);
	EXPECT_EQ(bumo::GetBlockReward(40 * bumo::General::REWARD_PERIOD), 8046);
	EXPECT_EQ(bumo::GetBlockReward(40 * bumo::General::REWARD_PERIOD + 1), 8046);


	EXPECT_EQ(bumo::GetBlockReward(41 * bumo::General::REWARD_PERIOD - 1), 8046);
	EXPECT_EQ(bumo::GetBlockReward(41 * bumo::General::REWARD_PERIOD), 6035);
	EXPECT_EQ(bumo::GetBlockReward(41 * bumo::General::REWARD_PERIOD + 1), 6035);


	EXPECT_EQ(bumo::GetBlockReward(42 * bumo::General::REWARD_PERIOD - 1), 6035);
	EXPECT_EQ(bumo::GetBlockReward(42 * bumo::General::REWARD_PERIOD), 4527);
	EXPECT_EQ(bumo::GetBlockReward(42 * bumo::General::REWARD_PERIOD + 1), 4527);


	EXPECT_EQ(bumo::GetBlockReward(43 * bumo::General::REWARD_PERIOD - 1), 4527);
	EXPECT_EQ(bumo::GetBlockReward(43 * bumo::General::REWARD_PERIOD), 3396);
	EXPECT_EQ(bumo::GetBlockReward(43 * bumo::General::REWARD_PERIOD + 1), 3396);


	EXPECT_EQ(bumo::GetBlockReward(44 * bumo::General::REWARD_PERIOD - 1), 3396);
	EXPECT_EQ(bumo::GetBlockReward(44 * bumo::General::REWARD_PERIOD), 2547);
	EXPECT_EQ(bumo::GetBlockReward(44 * bumo::General::REWARD_PERIOD + 1), 2547);


	EXPECT_EQ(bumo::GetBlockReward(45 * bumo::General::REWARD_PERIOD - 1), 2547);
	EXPECT_EQ(bumo::GetBlockReward(45 * bumo::General::REWARD_PERIOD), 1911);
	EXPECT_EQ(bumo::GetBlockReward(45 * bumo::General::REWARD_PERIOD + 1), 1911);


	EXPECT_EQ(bumo::GetBlockReward(46 * bumo::General::REWARD_PERIOD - 1), 1911);
	EXPECT_EQ(bumo::GetBlockReward(46 * bumo::General::REWARD_PERIOD), 1434);
	EXPECT_EQ(bumo::GetBlockReward(46 * bumo::General::REWARD_PERIOD + 1), 1434);


	EXPECT_EQ(bumo::GetBlockReward(47 * bumo::General::REWARD_PERIOD - 1), 1434);
	EXPECT_EQ(bumo::GetBlockReward(47 * bumo::General::REWARD_PERIOD), 1076);
	EXPECT_EQ(bumo::GetBlockReward(47 * bumo::General::REWARD_PERIOD + 1), 1076);


	EXPECT_EQ(bumo::GetBlockReward(48 * bumo::General::REWARD_PERIOD - 1), 1076);
	EXPECT_EQ(bumo::GetBlockReward(48 * bumo::General::REWARD_PERIOD), 807);
	EXPECT_EQ(bumo::GetBlockReward(48 * bumo::General::REWARD_PERIOD + 1), 807);


	EXPECT_EQ(bumo::GetBlockReward(49 * bumo::General::REWARD_PERIOD - 1), 807);
	EXPECT_EQ(bumo::GetBlockReward(49 * bumo::General::REWARD_PERIOD), 606);
	EXPECT_EQ(bumo::GetBlockReward(49 * bumo::General::REWARD_PERIOD + 1), 606);


	EXPECT_EQ(bumo::GetBlockReward(50 * bumo::General::REWARD_PERIOD - 1), 606);
	EXPECT_EQ(bumo::GetBlockReward(50 * bumo::General::REWARD_PERIOD), 455);
	EXPECT_EQ(bumo::GetBlockReward(50 * bumo::General::REWARD_PERIOD + 1), 455);


	EXPECT_EQ(bumo::GetBlockReward(51 * bumo::General::REWARD_PERIOD - 1), 455);
	EXPECT_EQ(bumo::GetBlockReward(51 * bumo::General::REWARD_PERIOD), 342);
	EXPECT_EQ(bumo::GetBlockReward(51 * bumo::General::REWARD_PERIOD + 1), 342);


	EXPECT_EQ(bumo::GetBlockReward(52 * bumo::General::REWARD_PERIOD - 1), 342);
	EXPECT_EQ(bumo::GetBlockReward(52 * bumo::General::REWARD_PERIOD), 257);
	EXPECT_EQ(bumo::GetBlockReward(52 * bumo::General::REWARD_PERIOD + 1), 257);


	EXPECT_EQ(bumo::GetBlockReward(53 * bumo::General::REWARD_PERIOD - 1), 257);
	EXPECT_EQ(bumo::GetBlockReward(53 * bumo::General::REWARD_PERIOD), 193);
	EXPECT_EQ(bumo::GetBlockReward(53 * bumo::General::REWARD_PERIOD + 1), 193);


	EXPECT_EQ(bumo::GetBlockReward(54 * bumo::General::REWARD_PERIOD - 1), 193);
	EXPECT_EQ(bumo::GetBlockReward(54 * bumo::General::REWARD_PERIOD), 145);
	EXPECT_EQ(bumo::GetBlockReward(54 * bumo::General::REWARD_PERIOD + 1), 145);


	EXPECT_EQ(bumo::GetBlockReward(55 * bumo::General::REWARD_PERIOD - 1), 145);
	EXPECT_EQ(bumo::GetBlockReward(55 * bumo::General::REWARD_PERIOD), 109);
	EXPECT_EQ(bumo::GetBlockReward(55 * bumo::General::REWARD_PERIOD + 1), 109);


	EXPECT_EQ(bumo::GetBlockReward(56 * bumo::General::REWARD_PERIOD - 1), 109);
	EXPECT_EQ(bumo::GetBlockReward(56 * bumo::General::REWARD_PERIOD), 82);
	EXPECT_EQ(bumo::GetBlockReward(56 * bumo::General::REWARD_PERIOD + 1), 82);


	EXPECT_EQ(bumo::GetBlockReward(57 * bumo::General::REWARD_PERIOD - 1), 82);
	EXPECT_EQ(bumo::GetBlockReward(57 * bumo::General::REWARD_PERIOD), 62);
	EXPECT_EQ(bumo::GetBlockReward(57 * bumo::General::REWARD_PERIOD + 1), 62);


	EXPECT_EQ(bumo::GetBlockReward(58 * bumo::General::REWARD_PERIOD - 1), 62);
	EXPECT_EQ(bumo::GetBlockReward(58 * bumo::General::REWARD_PERIOD), 47);
	EXPECT_EQ(bumo::GetBlockReward(58 * bumo::General::REWARD_PERIOD + 1), 47);


	EXPECT_EQ(bumo::GetBlockReward(59 * bumo::General::REWARD_PERIOD - 1), 47);
	EXPECT_EQ(bumo::GetBlockReward(59 * bumo::General::REWARD_PERIOD), 36);
	EXPECT_EQ(bumo::GetBlockReward(59 * bumo::General::REWARD_PERIOD + 1), 36);


	EXPECT_EQ(bumo::GetBlockReward(60 * bumo::General::REWARD_PERIOD - 1), 36);
	EXPECT_EQ(bumo::GetBlockReward(60 * bumo::General::REWARD_PERIOD), 27);
	EXPECT_EQ(bumo::GetBlockReward(60 * bumo::General::REWARD_PERIOD + 1), 27);


	EXPECT_EQ(bumo::GetBlockReward(61 * bumo::General::REWARD_PERIOD - 1), 27);
	EXPECT_EQ(bumo::GetBlockReward(61 * bumo::General::REWARD_PERIOD), 21);
	EXPECT_EQ(bumo::GetBlockReward(61 * bumo::General::REWARD_PERIOD + 1), 21);


	EXPECT_EQ(bumo::GetBlockReward(62 * bumo::General::REWARD_PERIOD - 1), 21);
	EXPECT_EQ(bumo::GetBlockReward(62 * bumo::General::REWARD_PERIOD), 16);
	EXPECT_EQ(bumo::GetBlockReward(62 * bumo::General::REWARD_PERIOD + 1), 16);


	EXPECT_EQ(bumo::GetBlockReward(63 * bumo::General::REWARD_PERIOD - 1), 16);
	EXPECT_EQ(bumo::GetBlockReward(63 * bumo::General::REWARD_PERIOD), 12);
	EXPECT_EQ(bumo::GetBlockReward(63 * bumo::General::REWARD_PERIOD + 1), 12);


	EXPECT_EQ(bumo::GetBlockReward(64 * bumo::General::REWARD_PERIOD - 1), 12);
	EXPECT_EQ(bumo::GetBlockReward(64 * bumo::General::REWARD_PERIOD), 9);
	EXPECT_EQ(bumo::GetBlockReward(64 * bumo::General::REWARD_PERIOD + 1), 9);


	EXPECT_EQ(bumo::GetBlockReward(65 * bumo::General::REWARD_PERIOD - 1), 9);
	EXPECT_EQ(bumo::GetBlockReward(65 * bumo::General::REWARD_PERIOD), 7);
	EXPECT_EQ(bumo::GetBlockReward(65 * bumo::General::REWARD_PERIOD + 1), 7);


	EXPECT_EQ(bumo::GetBlockReward(66 * bumo::General::REWARD_PERIOD - 1), 7);
	EXPECT_EQ(bumo::GetBlockReward(66 * bumo::General::REWARD_PERIOD), 6);
	EXPECT_EQ(bumo::GetBlockReward(66 * bumo::General::REWARD_PERIOD + 1), 6);


	EXPECT_EQ(bumo::GetBlockReward(67 * bumo::General::REWARD_PERIOD - 1), 6);
	EXPECT_EQ(bumo::GetBlockReward(67 * bumo::General::REWARD_PERIOD), 5);
	EXPECT_EQ(bumo::GetBlockReward(67 * bumo::General::REWARD_PERIOD + 1), 5);

	EXPECT_EQ(bumo::GetBlockReward(68 * bumo::General::REWARD_PERIOD - 1), 5);
	EXPECT_EQ(bumo::GetBlockReward(68 * bumo::General::REWARD_PERIOD), 4);
	EXPECT_EQ(bumo::GetBlockReward(68 * bumo::General::REWARD_PERIOD + 1), 4);

	EXPECT_EQ(bumo::GetBlockReward(69 * bumo::General::REWARD_PERIOD - 1), 4);
	EXPECT_EQ(bumo::GetBlockReward(69 * bumo::General::REWARD_PERIOD), 3);
	EXPECT_EQ(bumo::GetBlockReward(69 * bumo::General::REWARD_PERIOD + 1), 3);

	EXPECT_EQ(bumo::GetBlockReward(70 * bumo::General::REWARD_PERIOD - 1), 3);
	EXPECT_EQ(bumo::GetBlockReward(70 * bumo::General::REWARD_PERIOD), 2);
	EXPECT_EQ(bumo::GetBlockReward(70 * bumo::General::REWARD_PERIOD + 1), 2);

	EXPECT_EQ(bumo::GetBlockReward(71 * bumo::General::REWARD_PERIOD - 1), 2);
	EXPECT_EQ(bumo::GetBlockReward(71 * bumo::General::REWARD_PERIOD), 1);
	EXPECT_EQ(bumo::GetBlockReward(71 * bumo::General::REWARD_PERIOD + 1), 1);

	EXPECT_EQ(bumo::GetBlockReward(72 * bumo::General::REWARD_PERIOD - 1), 1);
	EXPECT_EQ(bumo::GetBlockReward(72 * bumo::General::REWARD_PERIOD), 0);
	EXPECT_EQ(bumo::GetBlockReward(72 * bumo::General::REWARD_PERIOD + 1), 0);

	EXPECT_EQ(bumo::GetBlockReward(73 * bumo::General::REWARD_PERIOD - 1), 0);
	EXPECT_EQ(bumo::GetBlockReward(73 * bumo::General::REWARD_PERIOD), 0);
	EXPECT_EQ(bumo::GetBlockReward(73 * bumo::General::REWARD_PERIOD + 1), 0);

	EXPECT_EQ(bumo::GetBlockReward(74 * bumo::General::REWARD_PERIOD - 1), 0);
	EXPECT_EQ(bumo::GetBlockReward(74 * bumo::General::REWARD_PERIOD), 0);
	EXPECT_EQ(bumo::GetBlockReward(74 * bumo::General::REWARD_PERIOD + 1), 0);

	EXPECT_EQ(bumo::GetBlockReward(1000 * bumo::General::REWARD_PERIOD - 1), 0);
	EXPECT_EQ(bumo::GetBlockReward(1000 * bumo::General::REWARD_PERIOD), 0);
	EXPECT_EQ(bumo::GetBlockReward(1000 * bumo::General::REWARD_PERIOD + 1), 0);

}
