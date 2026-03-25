find_package(Qt5 REQUIRED COMPONENTS Test)

set(crochet_test_srcs
    main.cpp
    testsettings.cpp
    teststitchlibrary.cpp
    teststitchset.cpp
    ../src/appinfo.cpp
    ../src/debug.cpp
    ../src/settings.cpp
    ../src/stitch.cpp
    ../src/stitchlibrary.cpp
    ../src/stitchset.cpp
    ${CMAKE_BINARY_DIR}/version.cpp
    ../crochet.qrc
    ../stitches/stitches.qrc
)

add_executable(crochetcharts_tests ${crochet_test_srcs})

target_include_directories(crochetcharts_tests PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(crochetcharts_tests PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Svg
    Qt5::Test
    Qt5::Xml
)

add_test(NAME crochetcharts_tests COMMAND crochetcharts_tests)
