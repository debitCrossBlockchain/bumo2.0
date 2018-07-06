#include <gtest/gtest.h>
#include "common/web_socket_server.h"

GTEST_API_ int main(int argc, char **argv)
{
	//testing::AddGlobalTestEnvironment(new FooEnvironment);
	testing::GTEST_FLAG(output) = "xml:gtest_result.xml";
	testing::InitGoogleTest(&argc, argv);
	RUN_ALL_TESTS();
	printf("enter for exit\n");
	getchar();
	return 0;
}