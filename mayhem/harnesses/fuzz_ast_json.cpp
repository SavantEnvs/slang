// Elaborate a design and serialize the whole AST (`slang --ast-json`, detailed types),
// which visits every symbol/expression/type lazily, plus the CST JSON (`--cst-json`).
#include <cstdint>
#include <string_view>

#include "common.h"
#include "slang/ast/ASTSerializer.h"
#include "slang/ast/symbols/CompilationUnitSymbols.h"
#include "slang/syntax/CSTSerializer.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/text/Json.h"
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
    (void)compilation.getAllDiagnostics();

    JsonWriter writer;
    ASTSerializer serializer(compilation, writer);
    serializer.setDetailedTypeInfo(true);
    serializer.serialize(compilation.getRoot());
    (void)writer.view();

    JsonWriter cstWriter;
    CSTSerializer cst(cstWriter);
    cst.serialize(*tree);
    (void)cstWriter.view();
    return 0;
}
