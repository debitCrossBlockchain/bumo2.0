#bumo overlay module CmakeLists.txt -- libbumo_overlay.a

set(LIB_BUMO_OVERLAY bumo_overlay)
#automaticall get src file in specifical dir
aux_source_directory(${BUMO_SRC_DIR}/overlay/ OVERLAY_SRC)

#generate static library file
add_library(${LIB_BUMO_OVERLAY} ${OVERLAY_SRC})

#specify compile options for target obj
target_compile_options(${LIB_BUMO_OVERLAY}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
)
