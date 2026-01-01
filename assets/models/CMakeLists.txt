file(GLOB_RECURSE SOURCE_MODELS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.gltf")

set(MODELS_OUT_DIR ${CMAKE_BINARY_DIR}/assets/models/gen)
file(MAKE_DIRECTORY "${MODELS_OUT_DIR}")

foreach(MODEL ${SOURCE_MODELS})
    get_filename_component(NAME ${MODEL} NAME_WE)
    set(OUT_MODEL "${MODELS_OUT_DIR}/${NAME}.glb")

    add_custom_command(
        OUTPUT ${OUT_MODEL}
        COMMAND gltfpack -i "${MODEL}" -o "${OUT_MODEL}" -tc -vpf
        DEPENDS ${MODEL}
        COMMENT "Optimizing ${MODEL} -> ${OUT_MODEL}"
    )

    list(APPEND ASSET_MODELS ${OUT_MODEL})
endforeach()

add_custom_target(GNVEModels DEPENDS ${ASSET_MODELS})

install(
    DIRECTORY "${MODELS_OUT_DIR}/"
    DESTINATION assets/models
    FILES_MATCHING PATTERN "*.glb"
)
