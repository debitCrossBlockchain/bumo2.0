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
