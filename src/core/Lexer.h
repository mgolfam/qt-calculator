#pragma once

#include "core/Token.h"

#include <string_view>
#include <vector>

namespace calc::core {

/// Stage 1 of evaluation: splits raw text into tokens.
///
/// Accepts ASCII operators as well as the pretty forms `×  ÷  −  π`, and `**` as an
/// alias for `^`. The returned list always ends with a TokenType::End token.
class Lexer {
public:
    /// @throws CalcError (ErrorKind::Syntax) for characters it does not understand.
    static std::vector<Token> tokenize(std::string_view input);
};

}  // namespace calc::core
