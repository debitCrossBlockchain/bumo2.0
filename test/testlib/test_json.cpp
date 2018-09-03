#include <json/json.h>
#include <utils/logger.h>
#include "test.h"

void TestJson(){
	std::string text = "{\"a\":[1,2,3,4], \"b\":\"bumo the json\"}";
	Json::Value jsonvalue;
	Json::Reader reader;
	if (!reader.parse(text, jsonvalue)){
		LOG_ERROR("Failed to parse json, raw json data is (%s) ", text.c_str());
	}

	std::string bumos = jsonvalue["b"].asString();
	Json::Value jsonarray = jsonvalue["a"];
	LOG_INFO("json string %s, array size:" FMT_SIZE, bumos.c_str(), jsonarray.size());
}

