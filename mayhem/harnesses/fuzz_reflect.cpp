// slang-reflect: compile a design and, when it elaborates without errors (as tools/reflect's
// main does), generate C++ headers for its packages' structs/enums/unions/localparams.
#include <cstdint>
#include <memory>
#include <string_view>

#include "SvTypeReflector.h"
#include "common.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/text/SourceManager.h"

using namespace slang;
using namespace slang::ast;
using namespace slang::syntax;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    SourceManager sourceManager;
    std::string_view text(reinterpret_cast<const char*>(data), size);
    auto tree = SyntaxTree::fromFileInMemory(text, sourceManager);

    auto compilation = std::make_unique<Compilation>(fuzzCompilationOptions());
    compilation->addSyntaxTree(tree);
    for (auto& diag : compilation->getAllDiagnostics()) {
        if (diag.isError())
            return 0;
    }

    for (bool noSystemC : {false, true}) {
        // The reflector consumes the compilation; reflect a fresh one the second time.
        if (!compilation) {
            compilation = std::make_unique<Compilation>(fuzzCompilationOptions());
            compilation->addSyntaxTree(tree);
            (void)compilation->getAllDiagnostics();
        }
        SvTypeReflector reflector(std::move(compilation), /*verbose*/ false, noSystemC);
        reflector.reflect();
        (void)reflector.emit();
    }
    return 0;
}
