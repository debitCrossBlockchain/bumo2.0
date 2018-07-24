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
int utils::Atoi(const std::string &str)
{
    return atoi(str.c_str()); 
}
long long utils::Atoll(const std::string &str)
{ 
    return atoll(str.c_str()); 
}
long utils::Atol(const std::string &str)
{ 
    return atol(str.c_str());
}
double utils::Atof(const std::string &str)
{ 
    return atof(str.c_str());
}
void utils::Aort(void)
{
    abort(); 
}
