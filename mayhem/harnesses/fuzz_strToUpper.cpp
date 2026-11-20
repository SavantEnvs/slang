// Ported from the legacy mayhemheroes integration (target `strtoupper`): slang::strToUpper.
// The legacy harness only upper-cased the input, which leaves ~40 edges of trivial code. The
// same bytes now also go through the other parsers in slang/util/String.h (integer and
// floating-point parsing in each base, splitting, case folding and edit distance) so the target
// exercises the whole string-utility surface under the same name.
#include <cstdint>
#include <string>
#include <string_view>

#include "slang/util/String.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string str(reinterpret_cast<const char*>(data), size);
    std::string orig = str;
    slang::strToUpper(str);
    if (str.size() != orig.size())
        __builtin_trap();
    std::string lower = orig;
    slang::strToLower(lower);
    if (lower.size() != orig.size())
        __builtin_trap();

    std::string_view view(orig);
    for (int base : {2, 8, 10, 16}) {
        size_t pos = 0;
        (void)slang::strToInt(view, &pos, base);
        if (pos > view.size())
            __builtin_trap();
    }
    size_t pos = 0;
    (void)slang::strToDouble(view, &pos);
    if (pos > view.size())
        __builtin_trap();

    char delim = size ? orig[0] : ',';
    for (auto part : slang::splitString(view.substr(size ? 1 : 0), delim))
        (void)part.size();

    // editDistance is quadratic: compare two halves, bounded like slang's own callers
    // (spelling suggestions over short identifiers).
    auto half = view.substr(0, std::min<size_t>(view.size(), 512));
    (void)slang::editDistance(half.substr(0, half.size() / 2), half.substr(half.size() / 2));
    (void)slang::editDistance(half.substr(0, half.size() / 2), half.substr(half.size() / 2), 4);
    return 0;
}
