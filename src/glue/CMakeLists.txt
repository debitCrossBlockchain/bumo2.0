#bumo glue module CmakeLists.txt -- libbumo_glue.a

set(LIB_BUMO_GLUE bumo_glue)

#automaticall get src file in specifical dir
aux_source_directory(${BUMO_SRC_DIR}/glue/ GLUE_SRC)

#generate static library file
add_library(${LIB_BUMO_GLUE} ${GLUE_SRC})

#specify compile options for target obj
target_compile_options(${LIB_BUMO_GLUE}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
    PUBLIC -D${OS_NAME}
)
