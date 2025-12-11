file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(GNVEApp ${SOURCES})
set_target_properties(GNVEApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
target_link_libraries(GNVEApp PRIVATE GNVEngine)
