// Shared helpers for the slang Mayhem harnesses.
#pragma once
#include "slang/ast/Compilation.h"

// Bounds taken from upstream's own fuzz entry point (tools/driver/slang_main.cpp), plus
// lower constexpr limits so one input can't keep the evaluator busy for millions of steps.
inline slang::ast::CompilationOptions fuzzCompilationOptions() {
    slang::ast::CompilationOptions options;
    options.maxInstanceDepth = 16;
    options.maxCheckerInstanceDepth = 16;
    options.maxDefParamSteps = 32;
    options.maxDefParamBlocks = 1024;
    options.maxGenerateSteps = 128;
    options.maxConstexprSteps = 100000;
    options.maxConstexprDepth = 64;
    options.errorLimit = 10;
    return options;
}
