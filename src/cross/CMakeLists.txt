#bumo glue module CmakeLists.txt -- libbumo_cross.a

set(LIB_BUMO_CROSS bumo_cross)

#Automatically get src files from the specified directory
aux_source_directory(${BUMO_SRC_DIR}/cross/ CROSS_SRC)

#Generate static library files
add_library(${LIB_BUMO_CROSS} ${CROSS_SRC})

#Specify compiling options for target objects
target_compile_options(${LIB_BUMO_CROSS}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
    PUBLIC -D${OS_NAME}
)
