#include "bu.h"
#include "bu-internal.h"
#include "configure.h"
#include "common/general.h"
int main(int argc, char *argv[]){

	for (int i = 0; i < 5; i++){
		Init("D:\\workspace\\github\\bumo\\build\\win32\\");
		utils::Sleep(10 * 1000);
		UnInit();
		utils::Sleep(10 * 1000);
	}
	Init("D:\\workspace\\github\\bumo\\build\\win32\\");

	Sleep(10000000000);

}

