// Parse-only: SyntaxTree::fromText lets the parser guess the node kind (Parser::parseGuess),
// then print the tree back to text and JSON (SyntaxPrinter, CSTSerializer).
#include <cstdint>
#include <string_view>

#include "slang/syntax/CSTSerializer.h"
#include "slang/syntax/SyntaxPrinter.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/text/Json.h"
#include "slang/text/SourceManager.h"

using namespace slang;
using namespace slang::syntax;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    SourceManager sourceManager;
    std::string_view text(reinterpret_cast<const char*>(data), size);
    auto tree = SyntaxTree::fromText(text, sourceManager);
    (void)tree->diagnostics();
    auto printed = SyntaxPrinter::printFile(*tree);
    (void)printed;

    JsonWriter writer;
    CSTSerializer serializer(writer, CSTJsonMode::SimpleTrivia);
    serializer.serialize(*tree);
    (void)writer.view();
    return 0;
}
