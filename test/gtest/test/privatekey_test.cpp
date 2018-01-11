#include <gtest/gtest.h>
#include "common/private_key.h"
#include "utils/crypto.h"
#include "common/data_secret_key.h"
#include <iostream>

void encode_decode(){
    //for (int i = 0; i < 1000; i++){
        bubi::PrivateKey priv_key(bubi::SIGNTYPE_ED25519);
        std::string public_key = priv_key.GetEncPublicKey();
        std::string private_key = priv_key.GetEncPrivateKey();
        std::string public_address = priv_key.GetEncAddress();
         std::cout << "public_key:" << public_key << std::endl;
         std::cout << "private_key:" << private_key << std::endl;
         std::cout << "public_address:" << public_address << std::endl;
         
        bubi::PrivateKey priv_key1(private_key);
        std::string public_key1 = priv_key.GetEncPublicKey();
        std::string private_key1 = priv_key.GetEncPrivateKey();
        std::string public_address1 = priv_key.GetEncAddress();
        /* std::cout << "public_key1:" << public_key1 << std::endl;
         std::cout << "private_key1:" << private_key1 << std::endl;
         std::cout << "public_address1:" << public_address1 << std::endl;*/
        ASSERT_EQ(public_key, public_key1);
        ASSERT_EQ(private_key, private_key1);
        ASSERT_EQ(public_address, public_address);

        bubi::PublicKey pubkey(public_key);
        std::string addr = pubkey.GetEncAddress();
        std::cout << "public_address:" << addr << std::endl;
        ASSERT_EQ(addr, public_address);
        ASSERT_EQ(pubkey.IsValid(), true);
    //}
}

void sign_verify(){
    bubi::PrivateKey skey(bubi::SIGNTYPE_ED25519);
    std::string strpubkey = skey.GetEncPublicKey();
    for (int i = 0; i < 1; i++)
    {
        std::string data = "hello" + std::to_string(i);
        std::string sig = skey.Sign(data);
        ASSERT_EQ(bubi::PublicKey::Verify(data, sig, strpubkey), true);
        std::cout << data << ":" << bubi::PublicKey::Verify(data, sig, strpubkey) << std::endl;
    }
}

void IsAddressValid(){
    bubi::PrivateKey priv_key(bubi::SIGNTYPE_ED25519);
    std::string public_key = priv_key.GetEncPublicKey();
    std::string private_key = priv_key.GetEncPrivateKey();
    std::string public_address = priv_key.GetEncAddress();

    
    std::cout << public_address << ":" << bubi::PublicKey::IsAddressValid(public_address)<< std::endl;
    ASSERT_EQ(bubi::PublicKey::IsAddressValid(public_address), true);

    std::string private_key_aes =utils::Aes::CryptoHex(private_key, bubi::GetDataSecuretKey());
    std::string private_key1 = utils::Aes::HexDecrypto(private_key_aes, bubi::GetDataSecuretKey());
    ASSERT_EQ(private_key, private_key1);
    bubi::PrivateKey priv_key1(private_key1);
    ASSERT_EQ(priv_key1.IsValid(), true);
}




TEST(encode_decode, encode_decode){
    encode_decode();
}

TEST(sign_verify, sign_verify){
    sign_verify();
}


TEST(IsAddressValid, IsAddressValid){
    IsAddressValid();
}