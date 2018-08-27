cmake_minimum_required(VERSION 2.8)

project(bumo)
add_compile_options(-g -O2)
add_definitions(-DSVNVERSION=\"${SVNVERSION}\")
message(STATUS "-DSVNVERSION="${SVNVERSION})

set( V8_LIB_PATH "linux")
set( OS_NAME "OS_LINUX")
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")  
    MESSAGE(STATUS "current platform: Linux ")  
	set( OS_NAME "OS_LINUX")
	set(BUMO_LINKER_FLAGS "-Wl,-dy -lpthread -lrt -ldl")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")  
    MESSAGE(STATUS "current platform: Darwin")  
	set( OS_NAME "OS_MAC")
	set(BUMO_LINKER_FLAGS "-Wl, -lpthread -ldl")
	set( V8_LIB_PATH "mac")
ELSE ()  
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")  
ENDIF ()  

set(BUMOCHAIN buchain)
set(BUMO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUMO_ROOT_DIR ${BUMO_SRC_DIR}/..)
set(LIBRARY_OUTPUT_PATH ${BUMO_ROOT_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${BUMO_ROOT_DIR}/bin)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

execute_process(COMMAND make all WORKING_DIRECTORY ${BUMO_SRC_DIR}/3rd)

include_directories(
    ${BUMO_SRC_DIR}
    ${BUMO_SRC_DIR}/3rd/basic/include/v8
    ${BUMO_SRC_DIR}/3rd/protobuf/src
    ${BUMO_SRC_DIR}/3rd/jsoncpp/include
    ${BUMO_SRC_DIR}/3rd/sqlite 
    ${BUMO_SRC_DIR}/3rd/rocksdb/include
    ${BUMO_SRC_DIR}/3rd/pcre-8.39
    ${BUMO_SRC_DIR}/3rd/websocketpp
    ${BUMO_SRC_DIR}/3rd/bzip2-1.0.6
    ${BUMO_SRC_DIR}/3rd/zlib-1.2.8
    ${BUMO_SRC_DIR}/3rd/asio/include
    ${BUMO_SRC_DIR}/3rd/openssl/include
    ${BUMO_SRC_DIR}/3rd/http
    ${BUMO_SRC_DIR}/3rd/libscrypt
)

link_directories(
    ${BUMO_SRC_DIR}/3rd/protobuf/src/.libs
    ${BUMO_SRC_DIR}/3rd/rocksdb
    ${BUMO_SRC_DIR}/3rd/pcre-8.39/.libs
    ${BUMO_SRC_DIR}/3rd/jsoncpp/libs
    ${BUMO_SRC_DIR}/3rd/openssl
    ${BUMO_SRC_DIR}/3rd/zlib-1.2.8
    ${BUMO_SRC_DIR}/3rd/libscrypt
    ${BUMO_SRC_DIR}/3rd/bzip2-1.0.6
    ${BUMO_SRC_DIR}/3rd/basic/lib
    ${BUMO_SRC_DIR}/3rd/curl/lib/.libs/
    ${BUMO_SRC_DIR}/3rd/v8_target/${V8_LIB_PATH}/
    ${LIBRARY_OUTPUT_PATH}
)

set(BUMO_DEPENDS_LIBS protobuf rocksdb pcreposix pcrecpp pcre json ssl crypto z bz2 scrypt)

add_subdirectory(3rd/http)
add_subdirectory(3rd/ed25519-donna)
add_subdirectory(glue)
add_subdirectory(utils)
add_subdirectory(proto)
add_subdirectory(ledger)
add_subdirectory(common)
add_subdirectory(libbumo_tools)
add_subdirectory(overlay)
add_subdirectory(consensus)
add_subdirectory(monitor)
add_subdirectory(main)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")  
	add_subdirectory(daemon)
	set(BUMO_SCRIPTS ${BUMO_ROOT_DIR}/deploy)
	install(
		PROGRAMS ${BUMO_SCRIPTS}/bumo ${BUMO_SCRIPTS}/bumod ${BUMO_SCRIPTS}/start-stop-daemon
		DESTINATION ${BUMOCHAIN}/scripts
	)
ENDIF ()  

install(
    DIRECTORY ${BUMO_ROOT_DIR}/bin/
    DESTINATION ${BUMOCHAIN}/bin
    FILES_MATCHING PATTERN "*.bin" PATTERN "*.dat"
)

install(
    DIRECTORY ${BUMO_ROOT_DIR}/build/win32/config/
    DESTINATION ${BUMOCHAIN}/config
    FILES_MATCHING PATTERN "bumo.json" PATTERN "bumo-mainnet.json" PATTERN "bumo-testnet.json" PATTERN "bumo-single.json"
)

install(
    DIRECTORY ${BUMO_ROOT_DIR}/build/win32/jslib/
    DESTINATION ${BUMOCHAIN}/jslib
    FILES_MATCHING PATTERN "*.js"
)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")  
	set(SUPPLEMENT ${BUMO_ROOT_DIR}/build/linux/MakeSupplement)
	set(FIX_DIR ${CMAKE_INSTALL_PREFIX}/${BUMOCHAIN})
	message(STATUS "install dir: ${FIX_DIR}")

	file(WRITE ${SUPPLEMENT} "soft_link:\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s ${FIX_DIR}/scripts/bumo /etc/init.d/bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s ${FIX_DIR}/scripts/bumod /etc/init.d/bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumo /etc/rc1.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumo /etc/rc1.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumo /etc/rc2.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumo /etc/rc3.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumo /etc/rc4.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumo /etc/rc5.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumod /etc/rc1.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumod /etc/rc1.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumod /etc/rc2.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumod /etc/rc3.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumod /etc/rc4.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo ln -s -f /etc/init.d/bumod /etc/rc5.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\tsudo mkdir -p ${FIX_DIR}/data;\n")
	file(APPEND ${SUPPLEMENT} "\tsudo mkdir -p ${FIX_DIR}/coredump;\n")

	file(APPEND ${SUPPLEMENT} "\nuninstall:\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/rc1.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/rc2.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/rc3.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/rc4.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/rc5.d/S99bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/init.d/bumo;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/rc1.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/rc2.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/rc3.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/rc4.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/rc5.d/S50bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -f /etc/init.d/bumod;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -rf ${FIX_DIR}/bin;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -rf ${FIX_DIR}/cfca;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -rf ${FIX_DIR}/config;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -rf ${FIX_DIR}/scripts\n")
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")  
    set(SUPPLEMENT ${BUMO_ROOT_DIR}/build/mac/MakeSupplement)
	set(FIX_DIR ${CMAKE_INSTALL_PREFIX}/${BUMOCHAIN})
	message(STATUS "install dir: ${FIX_DIR}")

	file(WRITE ${SUPPLEMENT} "soft_link:\n")
	file(APPEND ${SUPPLEMENT} "\tsudo mkdir -p ${FIX_DIR}/data;\n")
	file(APPEND ${SUPPLEMENT} "\tsudo mkdir -p ${FIX_DIR}/coredump;\n")

	file(APPEND ${SUPPLEMENT} "\nuninstall:\n")
	file(APPEND ${SUPPLEMENT} "\trm -rf ${FIX_DIR}/bin;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -rf ${FIX_DIR}/cfca;\\\n")
	file(APPEND ${SUPPLEMENT} "\trm -rf ${FIX_DIR}/config;\\\n")
ELSE ()  
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")  
ENDIF ()  
