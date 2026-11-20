// Preprocessor only (the `slang -E` path, Driver::runPreprocessor): macros, includes,
// conditionals, pragmas and (legacy) protect envelopes, printed back with comments/directives.
#include <cstdint>
#include <string_view>

#include "slang/diagnostics/Diagnostics.h"
#include "slang/parsing/Lexer.h"
#include "slang/parsing/Preprocessor.h"
#include "slang/syntax/SyntaxPrinter.h"
#include "slang/text/SourceManager.h"
#include "slang/util/Bag.h"
#include "slang/util/BumpAllocator.h"

using namespace slang;
using namespace slang::parsing;
using namespace slang::syntax;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    SourceManager sourceManager;
    BumpAllocator alloc;
    Diagnostics diagnostics;
    std::string_view text(reinterpret_cast<const char*>(data), size);

    LexerOptions lexerOptions;
    lexerOptions.enableLegacyProtect = true;
    Bag options;
    options.set(lexerOptions);

    Preprocessor preprocessor(sourceManager, alloc, diagnostics, options);
    preprocessor.pushSource(sourceManager.assignText(text));

    SyntaxPrinter output(sourceManager);
    output.setIncludeComments(true);
    output.setIncludeDirectives(true);
    while (true) {
        Token token = preprocessor.next();
        output.print(token);
        if (token.kind == TokenKind::EndOfFile)
            break;
    }
    (void)output.str();
    return 0;
}
