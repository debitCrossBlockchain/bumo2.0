#bumo monitor module CmakeLists.txt -- libbumo_monitor.a
set(LIB_BUMO_MONITOR bumo_monitor)

#Automatically get src files from the specified directory
aux_source_directory(${BUMO_SRC_DIR}/monitor/ MONITOR_SRC)

#Generate static library files
add_library(${LIB_BUMO_MONITOR} ${MONITOR_SRC})

#Specify compiling options for the target objects
target_compile_options(${LIB_BUMO_MONITOR}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
    PUBLIC -D${OS_NAME}
)
