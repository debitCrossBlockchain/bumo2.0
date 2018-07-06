#include "gtest/gtest.h"
#include "common/general.h"
#include "lib_bumo_tools.h"

class bumo_tools_utest : public testing::Test
{
protected:
    virtual void SetUp()
    {
		InitBumoTools();
    }

    virtual void TearDown()
    {
		UnInitBumoTools();
    }

protected:
	void UT_CreateAccountAddress();
	void UT_CheckAccountAddressValid();
	void UT_CreateKeystore();
	void UT_CheckKeystoreValid();
	void UT_SignData();
	void UT_SignDataWithKeystore();
	void UT_CheckSignedData();
	void UT_CreateKeystoreFromPrivkey();
	void UT_GetAddressFromPubkey();
	void UT_GetPrivatekeyFromKeystore();
};

TEST_F(bumo_tools_utest, UT_CreateAccountAddress){ UT_CreateAccountAddress(); }
void bumo_tools_utest::UT_CreateAccountAddress()
{
	{
		char input_signtype[1024] = "ed25519";
		char output_result[1024] = { 0 };
		int output_len = sizeof(output_result);
		EXPECT_EQ(CreateAccountAddress(input_signtype, output_result, &output_len), 0);

		Json::Value result;
		result.fromString(output_result);
		//printf("%s\n", result.toFastString().c_str());
		EXPECT_EQ(result["sign_type"].asString(), "ed25519");
	}


	{
		char input_signtype[1024] = "sm2";
		char output_result[1024] = { 0 };
		int output_len = sizeof(output_result);
		EXPECT_EQ(CreateAccountAddress(input_signtype, output_result, &output_len), 0);

		Json::Value result;
		result.fromString(output_result);
		//printf("%s\n", result.toFastString().c_str());
		EXPECT_EQ(result["sign_type"].asString(), "sm2");
	}

	{
		char input_signtype[1024] = "aa";
		char output_result[1024] = { 0 };
		int output_len = sizeof(output_result);
		EXPECT_EQ(CreateAccountAddress(input_signtype, output_result, &output_len), -2);

		Json::Value result;
		result.fromString(output_result);
		//printf("%s\n", result.toFastString().c_str());
		EXPECT_EQ(result["sign_type"].asString(), "");
	}

	{
		char input_signtype[1024] = "sm2";
		char *output_result = nullptr;
		int output_len = sizeof(output_result);
		EXPECT_EQ(CreateAccountAddress(input_signtype, output_result, &output_len), -1);
	}

}

TEST_F(bumo_tools_utest, UT_CheckAccountAddressValid){ UT_CheckAccountAddressValid(); }
void bumo_tools_utest::UT_CheckAccountAddressValid()
{
	{
		char input_encode_address[1024] = "dfafdads";
		EXPECT_EQ(CheckAccountAddressValid(input_encode_address), -2);
	}

	{
		//ed25519
		char input_encode_address[1024] = "buQY2pK5tdBkwP7z51Q5yy8WdgychHoRV6He";
		EXPECT_EQ(CheckAccountAddressValid(input_encode_address), 0);
	}

	{
		//sm2
		char input_encode_address[1024] = "buR8BgyAe6JjBRFF4uGh9TW7jCJi7NTo4imU";
		EXPECT_EQ(CheckAccountAddressValid(input_encode_address), 0);
	}
}

TEST_F(bumo_tools_utest, UT_CreateKeystore){ UT_CreateKeystore(); }
void bumo_tools_utest::UT_CreateKeystore()
{
	{
		char input_password[1024] = { 0 };
		char output_keystore[1024] = { 0 };
		int output_len = 1024;
		EXPECT_EQ(CreateKeystore(input_password, output_keystore, &output_len), -1);
	}
	
	{
		char input_password[1024] = "123456";
		char output_keystore[1024] = { 0 };
		int output_len = 1024;
		EXPECT_EQ(CreateKeystore(input_password, output_keystore, &output_len), 0);
	}

	{
		char input_password[1024] = "123456";
		char output_keystore[1024] = { 0 };
		int output_len = 1024;
		EXPECT_EQ(CreateKeystore(input_password, output_keystore, &output_len), 0);
	}
}

TEST_F(bumo_tools_utest, UT_CheckKeystoreValid){ UT_CheckKeystoreValid(); }
void bumo_tools_utest::UT_CheckKeystoreValid()
{
	{
		char output_keystore[1024] = { 0 };
		int output_len = 1024;
		{
			char input_password[1024] = "123456";
			EXPECT_EQ(CreateKeystore(input_password, output_keystore, &output_len), 0);
		}

		char input_keystore[1024] = { 0 };
		memcpy(input_keystore, output_keystore, output_len);
		char input_password[1024] = "123456";
		EXPECT_EQ(CheckKeystoreValid(input_keystore, input_password), 0);
	}

	{
		char input_keystore[1024] = {0};
		char input_password[1024];
		EXPECT_EQ(CheckKeystoreValid(input_keystore, input_password), -1);
	}

	{
		char output_keystore[1024] = { 0 };
		int output_len = 1024;
		{
			char input_password[1024] = "123456";
			EXPECT_EQ(CreateKeystore(input_password, output_keystore, &output_len), 0);
		}

		char input_keystore[1024] = {0};
		memcpy(input_keystore, output_keystore, output_len);
		char input_password[1024];
		EXPECT_EQ(CheckKeystoreValid(input_keystore, input_password), -2);
	}
}

TEST_F(bumo_tools_utest, UT_SignData){ UT_SignData(); }
void bumo_tools_utest::UT_SignData()
{
	{
		char input_privkey[1024] = "privbsx8X6wzxmPyDTNNCsGgLXuFmtfuwnQzyeGQkSEHfAzcrvTEy7ex";
		char input_rawdata[1024] = "aaaaaaaaaaaaaaaaaaaaaa";
		char output_data[1024] = { 0 };
		int output_len = 1024;
		EXPECT_EQ(SignData(input_privkey, input_rawdata, output_data, &output_len), 0);
	}
	
	{
		char input_privkey[1024] = "111";
		char input_rawdata[1024] = "aaaaaaaaaaaaaaaaaaaaaa";
		char output_data[1024] = { 0 };
		int output_len = 1024;
		EXPECT_EQ(SignData(input_privkey, input_rawdata, output_data, &output_len), -2);
	}

	{
		char input_privkey[1024] = "privbsx8X6wzxmPyDTNNCsGgLXuFmtfuwnQzyeGQkSEHfAzcrvTEy7ex";
		char input_rawdata[1024] = "aaaaaaaaaaaaaaaaaaaaaa";
		char output_data[1024] = { 0 };
		int output_len = 0;
		EXPECT_EQ(SignData(input_privkey, input_rawdata, output_data, &output_len), -1);
	}
}

TEST_F(bumo_tools_utest, UT_SignDataWithKeystore){ UT_SignDataWithKeystore(); }
void bumo_tools_utest::UT_SignDataWithKeystore()
{
	{
		char input_keystore[1024] = { 0 };
		char input_password[1024] = { 0 };
		char input_blob[1024];
		char output_data[1024];
		int output_len = 1024;

		EXPECT_EQ(SignDataWithKeystore(input_keystore, input_password, input_blob, output_data, &output_len), -1);
	}

	{
		char input_keystore[1024] = "111111111111111111111";
		char input_password[1024] = "222222";
		char input_blob[1024];
		char output_data[1024];
		int output_len = 1024;

		EXPECT_EQ(SignDataWithKeystore(input_keystore, input_password, input_blob, output_data, &output_len), -2);
	}

	{
		char input_password[1024] = "123456";
		char input_keystore[1024] = { 0 };
		int input_keystore_len = 1024;
		EXPECT_EQ(CreateKeystore(input_password, input_keystore, &input_keystore_len), 0);

		char input_blob[1024] = "11111111111111111111";
		char output_data[1024];
		int output_len = 1024;

		EXPECT_EQ(SignDataWithKeystore(input_keystore, input_password, input_blob, output_data, &output_len), 0);

		Json::Value result;
		result.fromString(output_data);
		EXPECT_EQ(result["sign_data"].empty(), false);
	}
	
}

TEST_F(bumo_tools_utest, UT_CheckSignedData){ UT_CheckSignedData(); }
void bumo_tools_utest::UT_CheckSignedData()
{
	{
		char input_blob[1024] = { 0 };
		char input_signeddata[1024] = { 0 };
		char input_pubkey[1024] = { 0 };
		EXPECT_EQ(CheckSignedData(input_blob, input_signeddata, input_pubkey), -1);
	}

	{
		char input_signtype[1024] = "ed25519";
		char output_result[1024] = { 0 };
		int output_len = sizeof(output_result);
		EXPECT_EQ(CreateAccountAddress(input_signtype, output_result, &output_len), 0);

		Json::Value result;
		result.fromString(output_result);
		//printf("%s\n", result.toFastString().c_str());
		EXPECT_EQ(result["sign_type"].asString(), "ed25519");

		char input_privkey[1024] = {0};
		memcpy(input_privkey, result["private_key"].asString().c_str(), result["private_key"].asString().size());

		char input_rawdata[1024] = "aaaaaaaaaaaaaaaaaaaaaa";
		char output_data[1024] = { 0 };
		output_len = 1024;
		EXPECT_EQ(SignData(input_privkey, input_rawdata, output_data, &output_len), 0);

		Json::Value sign_result;
		sign_result.fromString(output_data);
		EXPECT_EQ(sign_result["sign_data"].empty(), false);

		EXPECT_EQ(CheckSignedData(sign_result["data"].asString().data(),  sign_result["sign_data"].asString().data(), sign_result["public_key"].asString().data()), 0);
	}

	{
		char input_signtype[1024] = "ed25519";
		char output_result[1024] = { 0 };
		int output_len = sizeof(output_result);
		EXPECT_EQ(CreateAccountAddress(input_signtype, output_result, &output_len), 0);

		Json::Value result;
		result.fromString(output_result);
		//printf("%s\n", result.toFastString().c_str());
		EXPECT_EQ(result["sign_type"].asString(), "ed25519");

		char input_privkey[1024] = { 0 };
		memcpy(input_privkey, result["private_key"].asString().c_str(), result["private_key"].asString().size());

		char input_rawdata[1024] = "aaaaaaaaaaaaaaaaaaaaaa";
		char output_data[1024] = { 0 };
		output_len = 1024;
		EXPECT_EQ(SignData(input_privkey, input_rawdata, output_data, &output_len), 0);

		Json::Value sign_result;
		sign_result.fromString(output_data);
		EXPECT_EQ(sign_result["sign_data"].empty(), false);

		EXPECT_EQ(CheckSignedData("dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd",
			sign_result["sign_data"].asString().data(), sign_result["public_key"].asString().data()),
			-2);
	}
}

TEST_F(bumo_tools_utest, UT_CreateKeystoreFromPrivkey){ UT_CreateKeystoreFromPrivkey(); }
void bumo_tools_utest::UT_CreateKeystoreFromPrivkey()
{
	{
		char input_privkey[1024] = "privbsreTFEgnQ3eBEuzBMo8NgCht8XA1CxXnBHpVzpVfX23Bw2kWGJM";
		char input_password[1024] = "123456";
		char output_data[1024] = { 0 };
		int output_len = 1024;
		EXPECT_EQ(CreateKeystoreFromPrivkey(input_privkey, input_password, output_data, &output_len), 0);
		Json::Value sign_result;
		sign_result.fromString(output_data);
		EXPECT_EQ(sign_result["cypher_text"].empty(), false);
	}

	{
		char input_privkey[1024] = "aaaaaa";
		char input_password[1024] = "123456";
		char output_data[1024] = { 0 };
		int output_len = 1024;
		EXPECT_EQ(CreateKeystoreFromPrivkey(input_privkey, input_password, output_data, &output_len), -2);
		Json::Value sign_result;
		sign_result.fromString(output_data);
		EXPECT_EQ(sign_result["cypher_text"].empty(), true);
	}
	
}

TEST_F(bumo_tools_utest, UT_GetAddressFromPubkey){ UT_GetAddressFromPubkey(); }
void bumo_tools_utest::UT_GetAddressFromPubkey()
{
	{
		char input_pubkey[1024] = "b001976695b47cc14c5e80f855a33f8516f9950032d8523a0a3cc43903b73ce32383fe725f79";
		char output_data[1024];
		int output_len = 1024;
		EXPECT_EQ(GetAddressFromPubkey(input_pubkey, output_data, &output_len), 0);
	}

	{
		char input_pubkey[1024] = "1111";
		char output_data[1024];
		int output_len = 1024;
		EXPECT_EQ(GetAddressFromPubkey(input_pubkey, output_data, &output_len), -2);
	}

	{
		char input_pubkey[1024] = "1111";
		char output_data[1024];
		int output_len = 1024;
		EXPECT_EQ(GetAddressFromPubkey(input_pubkey, output_data, &output_len), -2);
	}
}

TEST_F(bumo_tools_utest, UT_GetPrivatekeyFromKeystore){ UT_GetPrivatekeyFromKeystore(); }
void bumo_tools_utest::UT_GetPrivatekeyFromKeystore()
{
	{
		char output_keystore[1024] = { 0 };
		int output_len = 1024;
		char input_password[1024] = "123456";
		EXPECT_EQ(CreateKeystore(input_password, output_keystore, &output_len), 0);

		char output_data[1024];
		EXPECT_EQ(GetPrivatekeyFromKeystore(output_keystore, input_password, output_data, &output_len), 0);
	}
	
	{
		char output_keystore[1024] = { 0 };
		int output_len = 1024;
		char input_password[1024] = "123456";
		EXPECT_EQ(CreateKeystore(input_password, output_keystore, &output_len), 0);

		char output_data[1024];
		output_len = 1024;
		EXPECT_EQ(GetPrivatekeyFromKeystore(output_keystore, "111111", output_data, &output_len), -2);
	}

	{
		char output_keystore[1024] = { 0 };
		int output_len = 1024;
		char input_password[1024] = "123456";
		EXPECT_EQ(CreateKeystore(input_password, output_keystore, &output_len), 0);

		char output_data[1024];
		output_len = 1024;
		EXPECT_EQ(GetPrivatekeyFromKeystore(output_keystore, input_password, output_data, &output_len), 0);
		printf("");
	}
}
