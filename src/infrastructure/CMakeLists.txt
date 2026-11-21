add_library(calc_infrastructure STATIC
    JsonHistoryRepository.h JsonHistoryRepository.cpp
)

target_include_directories(calc_infrastructure PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(calc_infrastructure PUBLIC calc_application Qt6::Core)
calc_set_warnings(calc_infrastructure)
