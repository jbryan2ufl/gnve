file(GLOB_RECURSE SOURCE_PNGS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.png")
file(GLOB_RECURSE SOURCE_JPGS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.jpg")
set(SOURCE_TEXTURES ${SOURCE_PNGS} ${SOURCE_JPGS})

set(TEXTURES_OUT_DIR ${CMAKE_BINARY_DIR}/assets/textures/gen)
file(MAKE_DIRECTORY "${TEXTURES_OUT_DIR}")

foreach(TEXTURE ${SOURCE_TEXTURES})
    get_filename_component(NAME ${TEXTURE} NAME_WE)
    set(OUT_KTX2 "${TEXTURES_OUT_DIR}/${NAME}.ktx2")

    add_custom_command(
        OUTPUT ${OUT_KTX2}
        COMMAND toktx --bcmp --t2 --genmipmap --assign_oetf linear ${OUT_KTX2} ${TEXTURE}
        DEPENDS ${TEXTURE}
        COMMENT "Converting ${TEXTURE} -> ${OUT_KTX2}"
    )

    list(APPEND ASSET_TEXTURES ${OUT_KTX2})
endforeach()

add_custom_target(GNVETextures DEPENDS ${ASSET_TEXTURES})

install(
    DIRECTORY "${TEXTURES_OUT_DIR}/"
    DESTINATION assets/textures
    FILES_MATCHING PATTERN "*.ktx2"
)
