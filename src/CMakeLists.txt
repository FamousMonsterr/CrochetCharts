file(GLOB crochet_srcs CONFIGURE_DEPENDS "*.cpp")
file(GLOB crochet_uis CONFIGURE_DEPENDS "*.ui")

set(crochet_resources
    ../crochet.qrc
    ../stitches/stitches.qrc
    ../translations/translations.qrc
    )

if(WIN32)
    #generate the embeded icon for the exe file.
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
    set(crochet_win "${CMAKE_SOURCE_DIR}/crochet.rc")

    set(QT_WIN_LIBS "C://qt//4.8.5//bin")
    set(MINGW_LIBS "C://MinGW-4.4//bin")
    
    set(QT_LIBS_WINDOWS "${QT_WIN_LIBS}//QtSvg4.dll" "${QT_WIN_LIBS}//QtCore4.dll" "${QT_WIN_LIBS}//QtGui4.dll"
                        "${QT_WIN_LIBS}//QtNetwork4.dll" "${QT_WIN_LIBS}//QtXml4.dll")
    set(QT_DEPS_WINDOWS "${MINGW_LIBS}//libgcc_s_dw2-1.dll" "${MINGW_LIBS}//mingwm10.dll"
                        "${CMAKE_SOURCE_DIR}/resources/qt.conf")

    set(QT_PLUGINS_WINDOWS "${QT_PLUGINS_DIR}/imageformats" "${QT_PLUGINS_DIR}/accessible" "${QT_PLUGINS_DIR}/iconengines")

else()

    set(crochet_nix)

endif()

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_executable(${EXE_NAME} ${crochet_srcs} ${crochet_uis} ${crochet_resources}
            ${crochet_version} ${crochet_win} ${crochet_mac} ${crochet_nix})
else()
    add_executable(${EXE_NAME} WIN32 MACOSX_BUNDLE ${crochet_srcs} ${crochet_uis} ${crochet_resources}
            ${crochet_version} ${crochet_win} ${crochet_mac} ${crochet_nix})
endif()

target_include_directories(${EXE_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${EXE_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Svg
    Qt5::Network
    Qt5::Xml
    Qt5::PrintSupport
)

if(APPLE)
    find_program(MACDEPLOYQT_EXECUTABLE
        NAMES macdeployqt
        HINTS /usr/local/opt/qt@5/bin /opt/homebrew/opt/qt@5/bin
    )

    set(MACOS_BUNDLE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.app")

    if(MACDEPLOYQT_EXECUTABLE)
        add_custom_target(bundle_macos
            COMMAND "${CMAKE_SOURCE_DIR}/utils/prepare_macos_bundle.sh"
                    "${MACOS_BUNDLE_PATH}"
                    "${MACDEPLOYQT_EXECUTABLE}"
            DEPENDS ${EXE_NAME}
            USES_TERMINAL
            COMMENT "Deploying Qt bundle and installing sanitized macOS launcher"
        )
    endif()

    #install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} DESTINATION ../MacOS)

    if(APP_STORE)
        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "APPLE_APP_STORE=1")
    endif()

    if(MACDEPLOYQT_EXECUTABLE)
        install(CODE "
            execute_process(
                COMMAND \"${CMAKE_SOURCE_DIR}/utils/prepare_macos_bundle.sh\"
                        \"${MACOS_BUNDLE_PATH}\"
                        \"${MACDEPLOYQT_EXECUTABLE}\"
                WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\"
                RESULT_VARIABLE _bundle_result)
            if(NOT _bundle_result EQUAL 0)
                message(FATAL_ERROR \"prepare_macos_bundle.sh failed with exit code \${_bundle_result}\")
            endif()
            file(COPY \"@CMAKE_SOURCE_DIR@/images/@PROJECT_MACOSX_ICON@\"
                DESTINATION \"@CMAKE_CURRENT_BINARY_DIR@/@PROJECT_NAME@.app/Contents/Resources\")
            file(COPY \"@CMAKE_CURRENT_BINARY_DIR@/@PROJECT_NAME@.app\"
                DESTINATION \"@CMAKE_BINARY_DIR@/_CPack_Packages/Darwin/@CPACK_GENERATOR@/@PROJECT_NAME@-@VERSION_SHORT@\")
        " COMPONENT Runtime)
    endif()

else()

    install(TARGETS ${EXE_NAME} BUNDLE DESTINATION . COMPONENT Runtime RUNTIME DESTINATION bin COMPONENT Runtime)
    if(WIN32)
        set_property(TARGET ${EXE_NAME} PROPERTY OUTPUT_NAME "${PROJECT_NAME}")
    endif()
endif()

if(WIN32)
    install(PROGRAMS ${QT_LIBS_WINDOWS} ${QT_DEPS_WINDOWS} DESTINATION bin)
    install(DIRECTORY ${QT_PLUGINS_WINDOWS} DESTINATION plugins)
endif()
