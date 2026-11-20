// Library map files (`slang --libmap`): SyntaxTree::fromLibraryMapText -> Parser::parseLibraryMap,
// then print the tree back to text.
#include <cstdint>
#include <string_view>

#include "slang/syntax/SyntaxPrinter.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/text/SourceManager.h"

using namespace slang;
using namespace slang::syntax;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    SourceManager sourceManager;
    std::string_view text(reinterpret_cast<const char*>(data), size);
    auto tree = SyntaxTree::fromLibraryMapText(text, sourceManager);
    (void)tree->diagnostics();
    auto printed = SyntaxPrinter::printFile(*tree);
    (void)printed;
    return 0;
}
