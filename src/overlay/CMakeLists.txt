#bumo overlay module CmakeLists.txt -- libbumo_overlay.a

set(LIB_BUMO_OVERLAY bumo_overlay)
#Automatically get src files from the specified directory
aux_source_directory(${BUMO_SRC_DIR}/overlay/ OVERLAY_SRC)

#Generate static library files
add_library(${LIB_BUMO_OVERLAY} ${OVERLAY_SRC})

#Specify compiling options for the target obj
target_compile_options(${LIB_BUMO_OVERLAY}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
    PUBLIC -D${OS_NAME}
)
