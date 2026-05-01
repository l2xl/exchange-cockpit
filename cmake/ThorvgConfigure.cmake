# Helper invoked by ThorvgBuild.cmake at build time to (re)configure the meson build dir.
# Handles three states:
#   - fresh build dir              -> meson setup ${BUILD_DIR} ${SRC_DIR} <opts>
#   - already configured           -> meson setup --reconfigure ${BUILD_DIR}  (regenerates build.ninja)
#   - configured + build.ninja ok  -> still safe to call --reconfigure, idempotent

if(EXISTS "${BUILD_DIR}/meson-info/intro-buildoptions.json")
    execute_process(
        COMMAND "${MESON}" setup --reconfigure "${BUILD_DIR}"
        WORKING_DIRECTORY "${SRC_DIR}"
        RESULT_VARIABLE rc
    )
else()
    execute_process(
        COMMAND "${MESON}" setup "${BUILD_DIR}" "${SRC_DIR}"
                --buildtype=release
                --default-library=static
                -Dengines=cpu
                -Dloaders=
                -Dsavers=
                -Dbindings=
                -Dtools=
                -Dextra=
                -Dthreads=false
                -Dtests=false
                -Dlog=false
        RESULT_VARIABLE rc
    )
endif()

if(NOT rc EQUAL 0)
    message(FATAL_ERROR "meson setup failed (exit code ${rc})")
endif()
