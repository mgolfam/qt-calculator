add_executable(learning-calculator WIN32 MACOSX_BUNDLE
    main.cpp
    ${PROJECT_SOURCE_DIR}/resources/resources.qrc
)

set_target_properties(learning-calculator PROPERTIES
    AUTORCC ON
    MACOSX_BUNDLE_BUNDLE_NAME "Learning Calculator"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.golfam.learningcalculator"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
)

target_link_libraries(learning-calculator PRIVATE calc_ui calc_infrastructure)
target_compile_definitions(learning-calculator PRIVATE CALC_VERSION="${PROJECT_VERSION}")
calc_set_warnings(learning-calculator)

install(TARGETS learning-calculator
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
