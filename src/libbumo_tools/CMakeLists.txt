#bumo tools module CmakeLists.txt -- libbumotools.so

set(LIB_BUMO_TOOLS bumotools)
set(TOOLS_SRC
    lib_bumo_tools.cpp
)

set(INNER_TOOLS_LIBS bumo_common bumo_utils bumo_ed25519)
set(BUMO_TOOLS_DEPENDS_LIBS json ssl crypto scrypt)


#generate share library file
add_library(${LIB_BUMO_TOOLS} SHARED ${TOOLS_SRC}) 

target_link_libraries(${LIB_BUMO_TOOLS} ${INNER_TOOLS_LIBS} ${BUMO_TOOLS_DEPENDS_LIBS})

#specify compile options for target obj
target_compile_options(${LIB_BUMO_TOOLS}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
    PUBLIC -D${OS_NAME}
)
