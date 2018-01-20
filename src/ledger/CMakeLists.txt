#bumo ledger module CmakeLists.txt -- libbumo_ledger.a

set(LIB_BUMO_LEDGER bumo_ledger)

#automaticall get src file in specifical dir
aux_source_directory(${BUMO_SRC_DIR}/ledger/ LEDGER_SRC)

#generate static library file
add_library(${LIB_BUMO_LEDGER} ${LEDGER_SRC})

#specify compile options for target obj
target_compile_options(${LIB_BUMO_LEDGER}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
)
