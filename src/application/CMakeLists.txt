add_library(calc_application STATIC
    CalculatorService.h CalculatorService.cpp
    Evaluation.h
    HistoryRepository.h
    PlotFunction.h PlotFunction.cpp
)

target_include_directories(calc_application PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(calc_application PUBLIC calc_core)
calc_set_warnings(calc_application)
