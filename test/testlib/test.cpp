#include <stdio.h>
#include <memory>
#include <string>
#include <cstring>
#include <exception>
#include <asio.hpp>
#include <algorithm>

#include <utils/headers.h>
#include <overlay/peer.h>
#include <common/private_key.h>
#include <common/network.h>
#include <common/pb2json.h>
#include <common/key_store.h>
#include <proto/cpp/consensus.pb.h>
#include "channel.h"
#include "test.h"
#include "test_v8_terminate.h"

class Manager : public utils::Singleton<Manager>
{
	friend class utils::Singleton < Manager > ;
	Manager(){};
	~Manager(){};
public:
	void Output(const std::string &text){
		printf("%s", text.c_str());
	}
};

class MyThread : public utils::Runnable {
public:
	MyThread(){
		total_ = 10;
	};
	~MyThread(){};

	int total_;

	void Run( utils::Thread *this_thread){
		LOG_ERROR("Thread start");
		for (int n = 0; n < 100; n++ ){
			if ( total_ > 0 )
			{
				LOG_TRACE("Thread id "FMT_SIZE", n:%d \n", this_thread->thread_id(), total_--);
			}
		}
	}
};

class MyThread2 : public utils::Runnable {
public:
	MyThread2(){
		enable_ = false;
		thread_ptr_ = NULL;
	};
	~MyThread2(){};

	bool enable_;
	utils::Thread *thread_ptr_;

	bool Initialize(){
		enable_ = true;
		thread_ptr_ = new utils::Thread(this);
		if (!thread_ptr_->Start()){
			LOG_ERROR_ERRNO("Failed to start thread ", STD_ERR_CODE, STD_ERR_DESC);
			return false;
		}
		return true;
	}

	bool Exit(){
		enable_ = false;
		thread_ptr_->JoinWithStop();
		return true;
	}

	void Run(utils::Thread *this_thread){
		LOG_INFO("Thread start");
		while (enable_){
			utils::Sleep(100);
		}
		LOG_INFO("Thread stop");
	}
};

void InitLog(){
	utils::Logger::InitInstance();
	utils::Logger::Instance().Initialize(utils::LOG_DEST_ALL, utils::LOG_LEVEL_ALL, "D:\\bumo.log", true);

	LOG_FATAL_ERRNO("The trace is ok, thread id:" FMT_SIZE, utils::Thread::current_thread_id(), STD_ERR_CODE, STD_ERR_DESC);
	LOG_ERROR("The trace is error, thread id:%d", 11);

	int32_t b = 1;
	int64_t c = 1;
	std::string a = utils::String::Format("%d", b);
	std::string d = utils::String::Format(FMT_I64, c);
}

void TestAsio(){
	using asio::ip::tcp;
	asio::io_service io_service;

	tcp::resolver  resolver(io_service);;
	tcp::resolver::query query("www.baidu.com", "daytime");
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	
	tcp::socket socket1(io_service);

	utils::InetAddress local_addr("0.0.0.0",12000);
	local_addr.ToIpPort();//0.0.0.0:12000
	utils::Socket socket;
	if (!socket.Create(utils::Socket::SOCKET_TYPE_TCP, local_addr)){
		LOG_ERROR_ERRNO("Failed to create socket", STD_ERR_CODE, STD_ERR_DESC);
	}

	utils::AsyncIo io;
	io.Create(1, 1);

	utils::InetAddress addre;
	addre.Resolve("www.baidu.com");
	addre.SetPort(80);
	utils::InetAddress local_address("0.0.0.0", 8081);

	utils::AsyncSocketTcp sockettcp(&io);
	bool result = sockettcp.Bind(local_address);
	LOG_TRACE_ERRNO("The result of binding is", STD_ERR_CODE, STD_ERR_DESC);
	sockettcp.AsyncConnect(addre);
	utils::Sleep(40000);
}

void TestThread(){
	MyThread nThread;
	utils::Thread thread1(&nThread);
	utils::Thread thread2(&nThread);
	utils::Thread thread3(&nThread);
	thread1.Start();
	thread2.Start();
	thread3.Start();
}

void TestThread1(){
	MyThread2 thread1;
	thread1.Initialize();
	utils::Sleep(1000);
	thread1.Exit();
}

void TestSingleton(){
	Manager::InitInstance();
	Manager::Instance().Output("this is the singleton demo");
	Manager::ExitInstance();
}

void TestStringUtils(){
	std::string normal_string = "Hello the world";
	std::string upper_string = utils::String::ToUpper(normal_string);
	printf("%s\n", upper_string.c_str());


	char a[100];
	//sprintf_s(a, 100, "%s the world", "hello");

	std::string c = "hello";
	std::string b = utils::String::Format("%s the world", c.c_str());
}

void OnTimer(){
	LOG_INFO("Time is up ");
}

void TestTimer(){
}

void TestSsl(){
	asio::ssl::context ctx(asio::ssl::context::sslv23);
	asio::error_code a;
	ctx.load_verify_file("D:\\workspace\\bumo_code1\\build\\win32\\dbin\\cacert.pem", a);

	utils::AsyncIo io;
	io.Create(1, 1);

	try{
		utils::InetAddress address("127.0.0.1", 443);
		utils::AsyncSocketSsl sslscoket(&io, ctx);
		if (sslscoket.Connect(address)){
			if (sslscoket.HandShake()){
				LOG_INFO("Handle successful");
			}
		}
	}
	catch (std::exception &e){
		LOG_INFO("Exception %s", e.what());
	}
}

std::string get_password(std::size_t, asio::ssl::context_base::password_purpose purpose)
{
	return "test";
}

class TestSocketSsl : public utils::AsyncSocketSsl{
public:
	TestSocketSsl(utils::AsyncIo *asyncio_ptr, asio::ssl::context& context) : AsyncSocketSsl(asyncio_ptr, context){}
	~TestSocketSsl(){}

	virtual void OnHandShake(){
		LOG_INFO("Handshake successful");
	}
};

class TestSslServer : public utils::IAsyncSocketAcceptorNotify{

public:
	TestSslServer() : ctx(asio::ssl::context::sslv23){}
	~TestSslServer(){}

	asio::ssl::context ctx;
	utils::AsyncIo io;
	utils::AsyncSocketAcceptor *accept;
	TestSocketSsl *ssl;

	void Start(){

		asio::ssl::context ctx(asio::ssl::context::sslv23);
		ctx.set_options(
			asio::ssl::context::default_workarounds
			| asio::ssl::context::no_sslv2
			| asio::ssl::context::single_dh_use);
		ctx.set_password_callback(get_password);
		ctx.use_certificate_chain_file("D:\\workspace\\bumo_code1\\build\\win32\\config\\server.pem");
		ctx.use_private_key_file("D:\\workspace\\bumo_code1\\build\\win32\\config\\server.pem", asio::ssl::context::pem);
		ctx.use_tmp_dh_file("D:\\workspace\\bumo_code1\\build\\win32\\config\\dh512.pem");


		//start accept
		io.Create(1, 1);

		ssl = new TestSocketSsl(&io, ctx);
		accept = new utils::AsyncSocketAcceptor(&io, this);
		accept->Bind(utils::InetAddress("0.0.0.0", 443));
		accept->Listen();
		accept->AsyncAccept(ssl);
	}


	virtual void OnAccept(utils::AsyncSocketAcceptor *acceptor){
		LOG_INFO("Accept successful");

		ssl->AsyncHandShake();
	}

	virtual void OnError(utils::AsyncSocketAcceptor *acceptor){
	}
};

void TestSslServer1(){
	TestSslServer server;
	server.Start();

	TestSsl();

	utils::Sleep(100000);
}

void TestBase64(){
	char buffer[] = {1,2,3};
	std::string strbuffer;
	strbuffer.assign(buffer, sizeof(buffer));
	strbuffer = "hello the world";
	std::string str = utils::encode_b64(strbuffer);
	LOG_INFO("Base64:%s", str.c_str());
}

#if 0
class ValueFrm{
public:
	ValueFrm(){}
	ValueFrm(const int &value) :value_(value){};
	~ValueFrm(){};

	int value_;

	bool operator<(const ValueFrm &value_frm) const{
		return value_ < value_frm.value_;
	}

	bool operator == (ValueFrm &value_frm) const {
		return  value_ == value_frm.value_;
	}

	bool empty() const{
		return true;
	}
};

class ValueFrm1{
public:
	ValueFrm1(){}
	ValueFrm1(const protocol::Value &value) :value_(value){};
	~ValueFrm1(){};

	protocol::Value value_;

	//bool operator<(const ValueFrm1 &value_frm) const{
	//	return value_.SerializeAsString() < value_frm.value_.SerializeAsString();
	//}

	//bool operator == (ValueFrm1 &value_frm) const {
	//	return  value_.SerializeAsString() == value_frm.value_.SerializeAsString();
	//}

	bool empty() const{
		return  value_.hash_set().empty();
	}
};

bool operator<(const ValueFrm1 &value_frm1, const ValueFrm1 &value_frm2){
	return value_frm1.value_.SerializeAsString() < value_frm2.value_.SerializeAsString();
}


bool operator<(const protocol::Value &value_frm1, const protocol::Value &value_frm2){
	return value_frm1.SerializeAsString() < value_frm2.SerializeAsString();
}

#endif

void TestAsio1(){
	utils::AsyncIo io;
	io.Create(1, 1);

	utils::AsyncSocketTcp tcp(&io);
	std::string buffer = "hello the world";
	tcp.AsyncSendSome(buffer.c_str(), buffer.size());
	//utils::Sleep(100000);
}

void TestPeer(const std::string &argv1, const std::string &argv2){
	utils::AsyncIo io;
	io.Create(1, 1);

	utils::AsyncSocketTcp tcp(&io);
	utils::InetAddress serveraddr(argv1);
	bool ret = tcp.Connect(serveraddr);
	LOG_INFO("Connect server(%s) %s", serveraddr.ToIpPort().c_str(), ret ? "success" : "failed");

	int32_t len = utils::String::Stol(argv2);

// 	bumo::PeerMsgHearder header;
// 	memset(&header, 0, sizeof(bumo::PeerMsgHearder));
// 	header.type = htons(rand()%32);
// 	header.data_len = htonl(len);
// 	tcp.SendSome((void *)&header, sizeof(header));
// 
// 	LOG_INFO("Send total length %d", len);
// 	size_t total_len = 0;
// 	while (tcp.IsValid() && total_len < len){
// 		unsigned char buffer[1000];
// 		RAND_bytes(buffer, sizeof(buffer));
// 		size_t s = tcp.SendSome(buffer, sizeof(buffer));
// 		total_len += s;
// 		if (s <= 0){
// 			LOG_ERROR_ERRNO("Send msg failed", STD_ERR_CODE, STD_ERR_DESC);
// 			break;
// 		} 
// 	}

	LOG_INFO("Send completed");
}

void TestSignature(){
	int64_t t1 = 0, t2 = 0, t3 = 0;
	for (int32_t i = 0; i < 1000; i++){
		int64_t s1 = utils::Timestamp::HighResolution();
		bumo::PrivateKey private_key(bumo::SIGNTYPE_ED25519);
		int64_t s2 = utils::Timestamp::HighResolution();
		std::string data = utils::String::Format("abc%d",i);
		std::string sign_data = private_key.Sign(data);
		int64_t s3 = utils::Timestamp::HighResolution();

		bool ret = bumo::PublicKey::Verify(data, sign_data, private_key.GetEncPublicKey());
		int64_t s4 = utils::Timestamp::HighResolution();

		t1 += s2 - s1;
		t2 += s3 - s2;
		t3 += s4 - s3;
	}

	LOG_INFO("New:" FMT_I64 "us sign:" FMT_I64 "us verify:" FMT_I64 "us", t1 / 1000, t2 / 1000, t3 / 1000);
	
}

void ParseFromProto() {
	std::string strs = "0aff010a2d080110022a27081f10e2152220a11f61397cbfafc1be3dbe3a62250bed63817c906ffaa859daab827cb91c4aea12cd010a8801623030323034633932353065623261356233626335373733626631313662353965393064356165653964376462633962656637336635633436616232643962666336623835313936356563366362373164323637316364353935343434663665353066366138363765636465616262313338313831373832656263366433356566636133666337651240483b72291a5bb9654b2ed431fe08c839b64257aaf9fe33c286fea59efb784fdef51e3c1a6cfca8be6b4c6fbc4402e28cd91f9e97f51ae32bf65e79ec84ce17690a81020a2f080110022a29081f10e21518032220a11f61397cbfafc1be3dbe3a62250bed63817c906ffaa859daab827cb91c4aea12cd010a8801623030323034333430323432333966326664616462396630623037316138313861616634383738323432323435646537303438303334386132326230663832363137633438386665326665623736616434633033633163313232643533323539383836653263633232356564313338306431613765643333363637626632346635653166326162321240409f839b10d548535c2d30b2d7020cf104b2d448ae568f3f8a23cb8305961c09eaca619dc13fc7140e8f0ba1486fed0c5dc6b9121aebcfafd2795a5dfbab56b70a81020a2f080110022a29081f10e21518022220a11f61397cbfafc1be3dbe3a62250bed63817c906ffaa859daab827cb91c4aea12cd010a8801623030323034326537393738363065663830613063616133316639616536306135356532613637363462373862313963366163316663653734326237623739663932643864333965623462613266376365613434646161343661636336313938646164653461383263666535646434663338613633313938303066363133616161626337386664651240c6e0a577a74d2366040efc0763527e0e7bda71ea55e97e7dec869b96961d833fa91df4fc5f600942668c1b7c0c69e66507d820d294c89d15f5cd1aacf31044e2";
	protocol::PbftProof poof;
	poof.ParseFromString(utils::String::HexStringToBin(strs));
	LOG_INFO("%s", bumo::Proto2Json(poof).toStyledString().c_str());
}

int main2(int32_t argc, char** argv);
int main1(int32_t argc, char** argv);

void TestCtrCase3() {

	//aes 256
	unsigned char ckey2[32] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 };
	unsigned char iv2[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	unsigned char input2[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	unsigned char output2[16] = { 0xee, 0x80, 0x11, 0xf0, 0xfc, 0xd9, 0xd8, 0xb9, 0x0e, 0x57, 0xb3, 0xb1, 0xa0, 0xdd, 0x69, 0x39 };

	std::string sinput2;
	sinput2.assign((char *)input2, 16);
	std::string soutput2;
	soutput2.assign((char *)output2, 16);
	std::string skey2;
	skey2.assign((char *)ckey2, sizeof(ckey2));

	utils::AesCtr aes2(iv2, skey2);
	std::string out2;
	aes2.Encrypt(sinput2, out2);
	//assert(out2 == soutput2);

	std::string outs2;
	aes2.Encrypt(out2, outs2);
	//assert(sinput2 == outs2);
}

void TestCtrCase4() {

	//aes 256
	unsigned char ckey2[32] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 };
	unsigned char iv2[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	unsigned char input2[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	unsigned char output2[16] = { 0xee, 0x80, 0x11, 0xf0, 0xfc, 0xd9, 0xd8, 0xb9, 0x0e, 0x57, 0xb3, 0xb1, 0xa0, 0xdd, 0x69, 0x39 };

	std::string sinput2;
	sinput2.assign((char *)input2, 16);
	std::string soutput2;
	soutput2.assign((char *)output2, sizeof(output2));
	std::string skey2;
	skey2.assign((char *)ckey2, sizeof(ckey2));

	utils::AesCtr aes2(iv2, skey2);
	std::string out2;
	aes2.Encrypt(sinput2, out2);
	//assert(out2 == soutput2);

	std::string outs2;
	aes2.Encrypt(out2, outs2);
	//assert(sinput2 == outs2);
}

void TestCtr() {

	//aes 128
	unsigned char iv1[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	unsigned char ckey1[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	unsigned char input1[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	unsigned char output1[16] = { 0x68, 0x41, 0x38, 0x30, 0x37, 0x66, 0xeb, 0xd8, 0x53, 0x40, 0x2d, 0x33, 0xab, 0x03, 0x01, 0x4d };

	std::string sinput;
	sinput.assign((char *)input1, 16);
	std::string soutput1;
	soutput1.assign((char *)output1, 16);
	std::string skey1;
	skey1.assign((char *)ckey1, 16);

	utils::AesCtr aes1(iv1, skey1);
	std::string out1;
	aes1.Encrypt(sinput, out1);
	assert(out1 == soutput1);

	std::string outs1;
	aes1.Encrypt(out1, outs1);
	assert(sinput == outs1);

	//aes 128
	unsigned char ckey2[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 };
	unsigned char iv2[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	unsigned char input2[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
	unsigned char output2[16] = { 0x7f, 0xef, 0x69, 0xd5, 0x12, 0xfb, 0x90, 0x02, 0xc9, 0x96, 0x52, 0xab, 0x9a, 0x0d, 0x0f, 0xc1 };

	std::string sinput2;
	sinput2.assign((char *)input2, 16);
	std::string soutput2;
	soutput2.assign((char *)output2, 16);
	std::string skey2;
	skey2.assign((char *)ckey2, 16);

	utils::AesCtr aes2(iv2, skey2);
	std::string out2;
	aes2.Encrypt(sinput2, out2);
	assert(out2 == soutput2);

	std::string outs2;
	aes2.Encrypt(out2, outs2);
	assert(sinput2 == outs2);

	TestCtrCase3();
}



int main(int32_t argc, char *argv[]){

	protocol::TransactionEnv env;
	std::string address = env.transaction().source_address();

	printf("%1.16ef", 0.1f * 0.1f);
	printf("%1.16ef", 0.1f * 0.3f * 0.7f);

	utils::net::Initialize();
	InitLog();
	TestCtr();
// 	uint32_t s1 = std::stoul("4294967295");
// 	uint64_t s2 = std::stoull("FFFFFFFFFFFFFFFF", NULL ,16);
// 	uint32_t s = utils::String::Stoui("4294967295");
// 	uint64_t s3 = utils::String::Stoui64(utils::String::ToString(s2));
// 	assert(s == 0xFFFFFFFF);
// 
// 	assert(utils::String::IsDecNumber("0.01234", 8));
// 	assert(utils::String::IsDecNumber("0.012345", 8));
// 	assert(utils::String::IsDecNumber("0.0123456", 8));
// 	assert(utils::String::IsDecNumber("0.01234567", 8));
// 	assert(utils::String::IsDecNumber("10.01234567", 8));
// 	assert(utils::String::IsDecNumber(".12345678", 8));
// 	assert(!utils::String::IsDecNumber("100.012345678", 8));
// 
// 	assert(!utils::String::IsDecNumber("12345678.", 8));
// 	assert(!utils::String::IsDecNumber("0.0.12345678", 8));
// 	assert(!utils::String::IsDecNumber(" 0.012345678", 8));
// 	assert(!utils::String::IsDecNumber("0.012345678 ", 8));
// 	assert(!utils::String::IsDecNumber("0.012345678", 8));
// 	assert(!utils::String::IsDecNumber("00.012345678", 8));
// 	assert(!utils::String::IsDecNumber("x00.012345678", 8));
// 	assert(!utils::String::IsDecNumber("x00.012345x678", 8));
// 	assert(!utils::String::IsDecNumber("a00.012345x678", 8));
//	assert(!utils::String::IsDecNumber("0123", 8));
// 	assert(utils::String::IsDecNumber("0", 8));
// 	assert(utils::String::IsDecNumber("0.0", 8));
// 	assert(utils::String::IsDecNumber("123", 8));
// 	assert(utils::String::IsDecNumber("1.0002", 8));
// 	assert(utils::String::IsDecNumber("1000.00", 8));
// 
// 	assert(utils::String::MultiplyDecimal("0.01234", 1) == "0.1234");
// 	assert(utils::String::MultiplyDecimal("0.01234", 2) == "1.234");
// 	assert(utils::String::MultiplyDecimal("0.01234", 3) == "12.34");
// 	assert(utils::String::MultiplyDecimal("0.01234", 4) == "123.4");
// 	assert(utils::String::MultiplyDecimal("0.01234", 5) == "1234");
// 	assert(utils::String::MultiplyDecimal("0.01234", 6) == "12340");
// 	assert(utils::String::MultiplyDecimal("0.01234", 7) == "123400");
// 
// 
// 	assert(utils::String::MultiplyDecimal("1110.012345678", 1) == "11100.12345678");
// 	assert(utils::String::MultiplyDecimal("1110.012345678", 2) == "111001.2345678");
// 	assert(utils::String::MultiplyDecimal("1110.012345678", 3) == "1110012.345678");
// 	assert(utils::String::MultiplyDecimal("1110.012345678", 4) == "11100123.45678");
// 	assert(utils::String::MultiplyDecimal("1110.012345678", 5) == "111001234.5678");
// 	assert(utils::String::MultiplyDecimal("1110.012345678", 6) == "1110012345.678");
// 	assert(utils::String::MultiplyDecimal("1110.012345678", 7) == "11100123456.78");
// 
// 	assert(utils::String::FormatDecimal(100000000, 1) == "10000000");
// 	assert(utils::String::FormatDecimal(100000000, 2) == "1000000");
// 	assert(utils::String::FormatDecimal(100000000, 3) == "100000");
// 	assert(utils::String::FormatDecimal(100000000, 4) == "10000");
// 	assert(utils::String::FormatDecimal(100000000, 5) == "1000");
// 	assert(utils::String::FormatDecimal(100000000, 6) == "100");
// 	assert(utils::String::FormatDecimal(100000000, 7) == "10");
// 	assert(utils::String::FormatDecimal(100000000, 8) == "1");
// 	assert(utils::String::FormatDecimal(100000000, 9) == "0.1");
// 
// 	assert(utils::String::FormatDecimal(123456789, 1) == "12345678.9");
// 	assert(utils::String::FormatDecimal(123456789, 2) == "1234567.89");
// 	assert(utils::String::FormatDecimal(123456789, 3) == "123456.789");
// 	assert(utils::String::FormatDecimal(123456789, 4) == "12345.6789");
// 	assert(utils::String::FormatDecimal(123456789, 5) == "1234.56789");
// 	assert(utils::String::FormatDecimal(123456789, 6) == "123.456789");
// 	assert(utils::String::FormatDecimal(123456789, 7) == "12.3456789");
// 	assert(utils::String::FormatDecimal(123456789, 8) == "1.23456789");
// 	assert(utils::String::FormatDecimal(123456789, 9) == "0.123456789");
// 
// 	bumo::KeyStore key_store;
// 	Json::Value keyss;
// 	std::string new_private;
// 	bool ret = key_store.Generate("bumo#07150926", keyss, new_private);
// 	printf("%s\n", keyss.toFastString().c_str());
// 	std::string pk;
// 	ret = key_store.From(keyss, "bub", pk);
// 	ret = key_store.From(keyss, "bumo", pk);
// 	ret = key_store.From(keyss, "bumo#", pk);
// 	ret = key_store.From(keyss, "bumo#0", pk);
// 	ret = key_store.From(keyss, "bumo#07", pk);
// 	ret = key_store.From(keyss, "bumo#071", pk);
// 	ret = key_store.From(keyss, "bumo#0715092", pk);
// 	ret = key_store.From(keyss, "bumo#07150926", pk);

	//TestSignature();
	ParseFromProto();

	std::string data = "123456789";
	int32_t crc8 = utils::Crc8((uint8_t *)data.c_str(), data.length());

	bumo::SslParameter ssl_p;
	if (argc > 1 && strcmp(argv[1], "client") == 0) {
		bumo::Network peer(ssl_p);
		peer.Connect("ws://127.0.0.1:9002");
		peer.Start(utils::InetAddress("127.0.0.1:9001"));
		return 0;
	}
	else if (argc > 1 ) {
		bumo::Network peer(ssl_p);
		peer.Start(utils::InetAddress("127.0.0.1:9002"));
		return 0;
	}
	//test_sm3();

	int32_t len = 1;
	if (argc > 3){
		len = utils::String::Stoi(argv[3]);
	} 

	for (int32_t i = 0; i < len; i++){
		TestPeer(argv[1], argv[2]);
	}
	//TestAsio1();
	//TestWebsocket();
	//test_websocketssl();

	//bool a = utils::File::IsExist("");
	//TestEd25519();


//	protocol::Value a1;
//	std::set<protocol::Value> a;
//	if (a.find(a1) == a.end()){
//	}

	//ValueFrm1 b;
	//a.insert(b);

	utils::Semaphore a;
	a.Wait();

	TestBase64();
	TestSsl();
	TestTimer();

	TestSqlite3();
	TestJson();
	//TestLevelDbWin();



	TestThread1();
	TestAsio();
	TestThread();
	TestStringUtils();

	int64_t tickbegin = utils::Timestamp::HighResolution();
}


/*
using namespace Bumo;

void dfs (RadixMerkleTreeNode::pointer node, int tree_depth){
//	node->get_hash ().to_string ();	
	if (!node->is_inner ()){
		printf ("DEPTH: %d\n", tree_depth);
		printf ("INDEX: ");
		node->peek_leaf ()->get_index ().to_string ();
		printf ("SERI_DATA: ");
		node->peek_leaf ()->data_to_string ();
		return;
	}
	for (int i = 0; i < 16; i++){
		if (!node->is_empty_branch (i)){
			dfs (node->get_child (i), tree_depth+1);
		}
	}
}

int main (){
	std::string db_name = "/home/ivanzjj/radix_tree";
	RocksdbInstance::set_db_name (db_name);
	Ledger::pointer ledger = std::make_shared <Ledger> ();
	
	std::string db_name2 = "/home/ivanzjj/ledger.db";
	SqliteInstance::set_db_name (db_name2);

	uint256 hash;
	char hash_ch[32];
	for (int i=0;i<32;i++)
		hash_ch[i] = i;
	hash.init (hash_ch);
	hash.to_string ();

	Serializer ss;
	char ch[100];
	for (int i=0;i<100;i++)	ch[i] = i;
	ss.add_raw (ch, 100);

	if (!ledger->add_account_tree_entry (hash, ss)){
		printf ("add_account_tree_entry error!\n");
		return 1;
	}
//	dfs (ledger->get_account_tree ()->get_root (), 0);

	for (int i = 0; i < 32; i++){
		hash_ch[i] = i;
	}
	hash_ch[0] = 1;
	hash.init (hash_ch);
	hash.to_string ();
	if (!ledger->add_account_tree_entry (hash, ss)){
		printf ("add_account_tree_entry error2\n");
		return 1;
	}
//	dfs (ledger->get_account_tree ()->get_root (), 0);

	
	for (int i = 0; i < 32; i++)	hash_ch[i] = i;
	hash_ch[0] = 1;
	hash_ch[1] = 2;

	hash.init (hash_ch);
	hash.to_string ();
	if (!ledger->add_account_tree_entry (hash, ss)){
		printf ("add_account_tree_entry error2\n");
		return 1;
	}
//	dfs (ledger->get_account_tree ()->get_root (), 0);
	
	for (int i = 0; i < 32; i++){
		hash_ch[i] = i;
	}
	hash.init (hash_ch);
	if (ledger->has_account (hash)){
		hash.to_string ();
		printf ("YES\n");
	
		ch[0] = 10;
		ss.peek_data ().clear();
		ss.add_raw (ch, 100);
		RadixMerkleTreeLeaf::pointer new_item = std::make_shared<RadixMerkleTreeLeaf> (hash, ss);
		if (!ledger->update_account_tree_entry (new_item)){
			printf ("update_account_tree_entry error!\n");
			return 1;
		}
	}
	else {
		printf ("NO\n");
	}	
//	dfs (ledger->get_account_tree ()->get_root (), 0);
	return 0;
}

*/
