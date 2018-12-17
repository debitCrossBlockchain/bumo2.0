#bumo main module CmakeLists.txt -- bumo

set(APP_BUMO bumo)

set(APP_BUMO_SRC
    main.cpp
    configure.cpp
    ../api/web_server.cpp
    ../api/web_server_query.cpp
    ../api/web_server_update.cpp
    ../api/web_server_command.cpp
    ../api/web_server_helper.cpp
    ../api/websocket_server.cpp
    ../api/console.cpp
)

set(INNER_LIBS bumo_glue bumo_ledger bumo_consensus bumo_overlay bumo_common bumo_utils bumo_proto bumo_http bumo_ed25519 bumo_monitor bumo_cross)
set(V8_LIBS v8_base v8_libbase v8_external_snapshot v8_libplatform v8_libsampler icui18n icuuc inspector)

#Generate executable files
add_executable(${APP_BUMO} ${APP_BUMO_SRC})

#Specify dependent libraries for target objects
	
IF (${OS_NAME} MATCHES "OS_LINUX")  
    MESSAGE(STATUS "current platform: Linux ")  
	target_link_libraries(${APP_BUMO}
    -Wl,-dn ${INNER_LIBS} -Wl,--start-group ${V8_LIBS} -Wl,--end-group ${BUMO_DEPENDS_LIBS} ${BUMO_LINKER_FLAGS})
ELSE ()  
	MESSAGE(STATUS "current platform: MAC ")  
	add_definitions(${BUMO_LINKER_FLAGS})
	target_link_libraries(${APP_BUMO} ${INNER_LIBS} ${V8_LIBS} ${BUMO_DEPENDS_LIBS})
ENDIF () 


#Specify compiling options for target objets
target_compile_options(${APP_BUMO}
    PUBLIC -std=c++11 
    PUBLIC -DASIO_STANDALONE
    PUBLIC -D_WEBSOCKETPP_CPP11_STL_
    PUBLIC -D${OS_NAME}
)

install(TARGETS ${APP_BUMO}
    RUNTIME DESTINATION ${BUMOCHAIN}/bin
)
