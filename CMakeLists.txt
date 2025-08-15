cmake_minimum_required(VERSION 3.13)
set(PICO_FLASH_SIZE_BYTES 16777216)

set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE "MinSizeRel")

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(gamevision C CXX ASM)
SET(BUILD_NAME "${PROJECT_NAME}")


SET(CC65_HOME "e:/cc65")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-ffast-math -feliminate-unused-debug-types -ffunction-sections -fdata-sections -O2)

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}")

pico_sdk_init()


# INCLUDE FILES THAT SHOULD BE COMPILED:
file(GLOB_RECURSE SRC "src/*.cpp" "src/*.c")

message(STATUS "Add source files:")
foreach (SRC_FILE IN LISTS SRC)
	message(STATUS "${SRC_FILE}")
endforeach ()
message(STATUS "")

add_executable(${PROJECT_NAME} ${SRC})




add_custom_target(build_watara_rom ALL
		COMMAND "${CC65_HOME}/bin/cl65" -t supervision -o main.sv main.c &&
				python3 ../utils/bin2c.py main.sv ../src/watara_rom.h
		WORKING_DIRECTORY "../cc65"
		COMMENT "Building WATARA rom"
)

add_dependencies(${PROJECT_NAME} build_watara_rom)
pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2_FILE})
target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
pico_set_boot_stage2(${PROJECT_NAME} slower_boot2)
pico_set_float_implementation(${PROJECT_NAME} none) # size optimizations
pico_set_double_implementation(${PROJECT_NAME} none) # size optimizations
pico_set_printf_implementation(${PROJECT_NAME} none)
set(FAMILY rp2040)
set(BOARD pico_sdk)

pico_set_program_name(${PROJECT_NAME} "GameBoy emulator for Watara Supervision")
pico_set_program_version(${PROJECT_NAME} "WIP")

target_include_directories(${PROJECT_NAME} PRIVATE src )

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${BUILD_NAME}")

target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        pico_runtime
        pico_multicore
		-Wl,--wrap=atexit # size optimizations
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
		PICO_FLASH_SIZE_BYTES=16777216
		ENABLE_SOUND=1
		PICO_PRINTF_ALWAYS_INCLUDED=0
		PICO_PANIC_FUNCTION=
)


target_link_options(${PROJECT_NAME} PRIVATE -Xlinker --print-memory-usage --data-sections)
pico_add_extra_outputs(${PROJECT_NAME})



