
set(MODULE_NAME "TestFreeRDPCodec")
set(MODULE_PREFIX "TEST_FREERDP_CODEC")

disable_warnings_for_directory(${CMAKE_CURRENT_BINARY_DIR})

set(${MODULE_PREFIX}_DRIVER ${MODULE_NAME}.c)

set(${MODULE_PREFIX}_TESTS
	TestFreeRDPRegion.c
	TestFreeRDPCodecMppc.c
	TestFreeRDPCodecNCrush.c
	TestFreeRDPCodecXCrush.c
	TestFreeRDPCodecZGfx.c
	TestFreeRDPCodecPlanar.c
    TestFreeRDPCodecCopy.c
	TestFreeRDPCodecClear.c
	TestFreeRDPCodecInterleaved.c
	TestFreeRDPCodecProgressive.c
	TestFreeRDPCodecRemoteFX.c)

create_test_sourcelist(${MODULE_PREFIX}_SRCS
	${${MODULE_PREFIX}_DRIVER}
	${${MODULE_PREFIX}_TESTS})

add_definitions(-DCMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DCMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_executable(${MODULE_NAME} ${${MODULE_PREFIX}_SRCS})

target_link_libraries(${MODULE_NAME} freerdp winpr)

set_target_properties(${MODULE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${TESTING_OUTPUT_DIRECTORY}")

foreach(test ${${MODULE_PREFIX}_TESTS})
	get_filename_component(TestName ${test} NAME_WE)
	add_test(${TestName} ${TESTING_OUTPUT_DIRECTORY}/${MODULE_NAME} ${TestName})
endforeach()

set_property(TARGET ${MODULE_NAME} PROPERTY FOLDER "FreeRDP/Test")

set(FUZZERS
	TestFuzzCodecs.c
)

include (AddFuzzerTest)
add_fuzzer_test("${FUZZERS}" "freerdp winpr")
