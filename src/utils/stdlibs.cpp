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
#include<iostream>
#include <cstdlib>
#include"stdlibs.h"
using namespace std;
Stdlib::Stdlib()
{
}

Stdlib::~Stdlib()
{
}

int  Stdlib::Bumoatoi(const std::string &str)
{
     return atoi(str.c_str()); 
    
}
long long  Stdlib::Bumoatoll(const std::string &str)
{ 
    return atoll(str.c_str()); 
}
long  Stdlib::Bumoatol(const std::string &str)
{ 
    return atol(str.c_str());
}
double Stdlib::Bumoatof(const std::string &str)
{ 
    return atof(str.c_str());
}
void  Stdlib::Bumoabort(void)
{
     abort(); 
}
