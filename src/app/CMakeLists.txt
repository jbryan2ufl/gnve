file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(GNVEApp ${SOURCES})
target_link_libraries(GNVEApp PRIVATE GNVEngine)

add_dependencies(GNVEApp
    # GNVETextures
    GNVEModels
    GNVEShaders
    GNVEngine
)

install(
    TARGETS GNVEApp
    RUNTIME DESTINATION .
)
