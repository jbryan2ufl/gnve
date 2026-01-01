find_program(SLANGC_EXECUTABLE slangc REQUIRED)
set(SHADER_OUT_DIR ${CMAKE_BINARY_DIR}/shaders/gen)
file(GLOB_RECURSE SLANG_SHADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.slang")
set(SPIRV_OUTPUTS)

foreach(SHADER ${SLANG_SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    set(SPIRV ${SHADER_OUT_DIR}/${SHADER_NAME}.spv)

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${SLANGC_EXECUTABLE}
                    ${SHADER}
                    -target spirv
                    -profile spirv_1_4
                    -emit-spirv-directly
                    -fvk-use-entrypoint-name
                    -entry vertMain
                    -entry fragMain
                    -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_NAME}.slang -> ${SHADER_NAME}.spv"
        VERBATIM
    )

    list(APPEND SPIRV_OUTPUTS ${SPIRV})
endforeach()
add_custom_target(GNVEShaders DEPENDS ${SPIRV_OUTPUTS})

install(
    DIRECTORY "${SHADER_OUT_DIR}/"
    DESTINATION shaders/
    FILES_MATCHING PATTERN "*.spv"
)
