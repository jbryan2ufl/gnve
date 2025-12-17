file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(GNVEApp ${SOURCES})
add_dependencies(GNVEApp GNVEShaders)
target_link_libraries(GNVEApp PRIVATE GNVEngine)
set_target_properties(GNVEApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

install(
    TARGETS GNVEApp
    RUNTIME
    DESTINATION bin
)
install(
    DIRECTORY ${CMAKE_BINARY_DIR}/bin/shaders/
    DESTINATION bin/shaders
    FILES_MATCHING PATTERN "*.spv"
)
