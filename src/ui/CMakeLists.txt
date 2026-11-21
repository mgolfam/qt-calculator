add_library(calc_ui STATIC
    MainWindow.h MainWindow.cpp
    Strings.h
    Theme.h Theme.cpp

    widgets/AstDiagramWidget.h widgets/AstDiagramWidget.cpp
    widgets/DisplayWidget.h widgets/DisplayWidget.cpp
    widgets/GraphWidget.h widgets/GraphWidget.cpp
    widgets/KeypadWidget.h widgets/KeypadWidget.cpp

    panels/BaseConverterPanel.h panels/BaseConverterPanel.cpp
    panels/GraphPanel.h panels/GraphPanel.cpp
    panels/HistoryPanel.h panels/HistoryPanel.cpp
    panels/ReferencePanel.h panels/ReferencePanel.cpp
    panels/StepsPanel.h panels/StepsPanel.cpp
    panels/TokensPanel.h panels/TokensPanel.cpp
    panels/TreePanel.h panels/TreePanel.cpp
)

set_target_properties(calc_ui PROPERTIES AUTOMOC ON)
target_include_directories(calc_ui PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(calc_ui PUBLIC calc_application Qt6::Widgets)
calc_set_warnings(calc_ui)
