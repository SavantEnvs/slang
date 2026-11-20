// Full SystemVerilog front end: parse a compilation unit, elaborate it, and then run the
// analysis passes (data flow, driver tracking) exactly as the `slang` CLI's Driver::runAnalysis.
#include <cstdint>
#include <string_view>

#include "common.h"
#include "slang/analysis/AnalysisManager.h"
#include "slang/diagnostics/DiagnosticEngine.h"
#include "slang/diagnostics/TextDiagnosticClient.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/text/SourceManager.h"

using namespace slang;
using namespace slang::ast;
using namespace slang::syntax;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    SourceManager sourceManager;
    std::string_view text(reinterpret_cast<const char*>(data), size);
    auto tree = SyntaxTree::fromFileInMemory(text, sourceManager);

    DiagnosticEngine diagEngine(sourceManager);
    diagEngine.setErrorLimit(10);
    auto client = std::make_shared<TextDiagnosticClient>();
    diagEngine.addClient(client);

    Compilation compilation(fuzzCompilationOptions());
    compilation.addSyntaxTree(tree);
    for (auto& diag : compilation.getAllDiagnostics())
        diagEngine.issue(diag);

    compilation.freeze();
    analysis::AnalysisManager analysisManager;
    analysisManager.analyze(compilation);
    for (auto& diag : analysisManager.getDiagnostics())
        diagEngine.issue(diag);
    compilation.unfreeze();

    (void)client->getString();
    return 0;
}
