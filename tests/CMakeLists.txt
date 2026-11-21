function(calc_add_test name)
    add_executable(${name} ${name}.cpp TestHelpers.h)
    set_target_properties(${name} PROPERTIES
        AUTOMOC ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
    target_link_libraries(${name} PRIVATE ${ARGN} Qt6::Test)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    calc_set_warnings(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

calc_add_test(tst_lexer calc_core)
calc_add_test(tst_parser calc_core)
calc_add_test(tst_evaluator calc_core)
calc_add_test(tst_base_converter calc_core)
calc_add_test(tst_calculator_service calc_application)
calc_add_test(tst_json_history_repository calc_infrastructure)
