enable_testing()
add_executable(memfs-tests 
    "${PROJECT_SOURCE_DIR}/tests/test-basic.cpp"
)

target_link_libraries(memfs-tests PUBLIC
    memfs
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(memfs-tests)
