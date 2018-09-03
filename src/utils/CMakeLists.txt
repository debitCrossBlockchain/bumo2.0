#bumo utils module CmakeLists.txt -- libbumo_utils.a

set(LIB_BUMO_UTILS bumo_utils)
set(UTILS_SRC
    file.cpp logger.cpp net.cpp thread.cpp timestamp.cpp utils.cpp 
    crypto.cpp lrucache.hpp timer.cpp system.cpp
    sm3.cpp ecc_sm2.cpp random.cpp
)

#Generate static library files
add_library(${LIB_BUMO_UTILS} ${UTILS_SRC}) 

#Specify compiling options for target objects
target_compile_options(${LIB_BUMO_UTILS}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D${OS_NAME}
	PUBLIC -fPIC
)
