#bumo daemon module CmakeLists.txt

set(DEAMON_DEPENDS_LIB ssl crypto)

# -- bumo_bumod -- #
set(BIN_BUMO_ID bumod)

add_executable(${BIN_BUMO_ID} bumo/main.cpp)
target_link_libraries(${BIN_BUMO_ID} -Wl,-dn ${DEAMON_DEPENDS_LIB} ${BUMO_LINKER_FLAGS})

target_compile_options(${BIN_BUMO_ID}
    PUBLIC -std=c++11 
)

install(TARGETS ${BIN_BUMO_ID}
    RUNTIME DESTINATION ${BUMOCHAIN}/bin
)

