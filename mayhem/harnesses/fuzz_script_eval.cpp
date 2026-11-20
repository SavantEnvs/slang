// ScriptSession: each input line is parsed with Parser::parseGuess (declaration, statement or
// expression), bound, and constant-evaluated in one session, like slang's REPL/test scripts.
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

#include "slang/ast/ScriptSession.h"
#include "slang/numeric/ConstantValue.h"
#include "slang/util/Bag.h"
#include "common.h"

using namespace slang;
using namespace slang::ast;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string_view text(reinterpret_cast<const char*>(data), size);
    Bag options;
    options.set(fuzzCompilationOptions());
    ScriptSession session(std::move(options));

    size_t pos = 0;
    while (pos <= text.size()) {
        size_t nl = text.find('\n', pos);
        if (nl == std::string_view::npos)
            nl = text.size();
        auto line = text.substr(pos, nl - pos);
        if (!line.empty()) {
            // ScriptSession::eval documents that it throws std::runtime_error for a node kind it
            // does not support evaluating (e.g. a module declaration); that's a rejected line, not
            // a bug. Internal assertion failures (AssertionException, a std::logic_error) still crash.
            try {
                ConstantValue cv = session.eval(line);
                std::string s = cv.toString();
                (void)s;
            }
            catch (const std::runtime_error&) {
            }
        }
        pos = nl + 1;
    }
    (void)session.getDiagnostics();
    return 0;
}
