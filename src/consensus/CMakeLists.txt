#bumo consensus module CmakeLists.txt -- libbumo_consensus.a

set(LIB_BUMO_CONSENSUS bumo_consensus)
#automaticall get src file in specifical dir
aux_source_directory(${BUMO_SRC_DIR}/consensus/ CONSENSUS_SRC)

#generate static library file
add_library(${LIB_BUMO_CONSENSUS} ${CONSENSUS_SRC})

#specify compile options for target obj
target_compile_options(${LIB_BUMO_CONSENSUS}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
)

