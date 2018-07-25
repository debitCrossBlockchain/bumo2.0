#bumo glue module CmakeLists.txt -- libbumo_glue.a

set(LIB_BUMO_GLUE bumo_glue)

#Automatically get src files from the specified directory
aux_source_directory(${BUMO_SRC_DIR}/glue/ GLUE_SRC)

#Generate static library files
add_library(${LIB_BUMO_GLUE} ${GLUE_SRC})

#Specify compiling options for target objects
target_compile_options(${LIB_BUMO_GLUE}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
    PUBLIC -D${OS_NAME}
)
