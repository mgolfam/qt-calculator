add_library(calc_core STATIC
    Ast.h Ast.cpp
    AstPrinter.h AstPrinter.cpp
    BaseConverter.h BaseConverter.cpp
    Environment.h
    Errors.h
    Evaluator.h Evaluator.cpp
    Lexer.h Lexer.cpp
    MathLibrary.h MathLibrary.cpp
    NumberFormat.h NumberFormat.cpp
    Parser.h Parser.cpp
    Token.h Token.cpp
)

target_include_directories(calc_core PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_compile_features(calc_core PUBLIC cxx_std_17)
calc_set_warnings(calc_core)
