# Layers, from the inside out. Each layer may only depend on the ones above it:
#
#   core            pure C++17: lexer, parser, AST, evaluator, math library  (no Qt)
#   application     use cases: CalculatorService, history port, plotting     (no Qt)
#   infrastructure  adapters for the ports, e.g. JSON history on disk        (Qt Core)
#   ui              Qt Widgets presentation                                  (Qt Widgets)
#   main            composition root: wires everything together
add_subdirectory(core)
add_subdirectory(application)
add_subdirectory(infrastructure)
add_subdirectory(ui)
add_subdirectory(main)
