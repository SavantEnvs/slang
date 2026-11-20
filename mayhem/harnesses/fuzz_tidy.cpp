// slang-tidy: compile + analyze a design (as tools/tidy's main does via Driver::runAnalysis),
// then run every registered lint check over it with the default configuration.
#include <cstdint>
#include <string_view>

#include "TidyConfig.h"
#include "TidyFactory.h"
#include "common.h"
#include "slang/analysis/AnalysisManager.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/text/SourceManager.h"

using namespace slang;
using namespace slang::ast;
using namespace slang::syntax;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    SourceManager sourceManager;
    std::string_view text(reinterpret_cast<const char*>(data), size);
    auto tree = SyntaxTree::fromFileInMemory(text, sourceManager);

    Compilation compilation(fuzzCompilationOptions());
    compilation.addSyntaxTree(tree);
    for (auto& diag : compilation.getAllDiagnostics()) {
        if (diag.isError())
            return 0; // slang-tidy stops on compilation errors, too
    }
    compilation.freeze();
    analysis::AnalysisManager analysisManager;
    analysisManager.analyze(compilation);
    compilation.unfreeze();

    // The Registry is process-global: reset its config and source manager for every input.
    TidyConfig config;
    Registry::setConfig(config);
    Registry::setSourceManager(compilation.getSourceManager());
    for (const auto& name : Registry::getRegisteredChecks()) {
        auto check = Registry::create(name);
        (void)check->check(compilation.getRoot(), analysisManager);
        (void)check->getDiagnostics();
    }
    return 0;
}
