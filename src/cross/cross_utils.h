/*
bumo is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

bumo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with bumo.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CROSS_UTILS_H_
#define CROSS_UTILS_H_
#include<iostream>
#include <utils/headers.h>
#include<ledger/ledger_manager.h>
using namespace std;
namespace bumo {
	class ContractTestParameter;
	class CrossUtilsManager :public utils::NonCopyable{
	public:
		CrossUtilsManager();
		~CrossUtilsManager();
		static void  CallContract(ContractTestParameter &test_parameter, std::string &reply);
	};

}


#endif