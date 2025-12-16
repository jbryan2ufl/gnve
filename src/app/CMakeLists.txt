file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(GNVEApp ${SOURCES})

install(TARGETS GNVEApp
    DESTINATION bin
    RUNTIME_DEPENDENCY_SET app_deps
)

install(RUNTIME_DEPENDENCY_SET app_deps
    DESTINATION bin
    PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-"
    POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
)

if(MINGW)
    target_link_options(GNVEApp PRIVATE -static-libgcc -static-libstdc++)
    target_link_options(GNVEApp PRIVATE -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive)
endif()

set_target_properties(GNVEApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(GNVEApp PRIVATE GNVEngine)
