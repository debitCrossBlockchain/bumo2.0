#ifndef __BU_HPP__
#define __BU_HPP__

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
#endif

/********************************************************************************
description:
	Start the block chain program
input: 
	bu_home_path:    The buchain path. For example, "/sdcard/buchain/"
output:
	
return:
	0：OK，other error
********************************************************************************/
int Init(char *bu_home_path);

/********************************************************************************
description:
	Stop the block chain program
input: 
	bu_home_path:    The buchain path. For example, "/sdcard/buchain/"
output:
	
return:
	0：OK，other error
********************************************************************************/
int UnInit();

#endif
