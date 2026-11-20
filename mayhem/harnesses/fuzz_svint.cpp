// SVInt::fromString: parses a SystemVerilog integer literal ("8'hFF", "-16'sd42", "4'b1x0z",
// "123") into an arbitrary-precision 4-state integer. It is the public text entry point of
// slang's numeric library (also what the svint user-defined literal and pyslang use). Per its
// contract it reports malformed input by throwing std::invalid_argument; nothing else is caught.
// Every value that parses is printed back in each base (SVInt::toString / writeTo).
#include <cstdint>
#include <stdexcept>
#include <string_view>

#include "slang/numeric/SVInt.h"

using namespace slang;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string_view text(reinterpret_cast<const char*>(data), size);
    SVInt value;
    try {
        value = SVInt::fromString(text);
    }
    catch (const std::invalid_argument&) {
        return 0;
    }
    (void)value.toString();
    for (auto base : {LiteralBase::Binary, LiteralBase::Octal, LiteralBase::Decimal,
                      LiteralBase::Hex}) {
        (void)value.toString(base, true);
        (void)value.toString(base, false, SVInt::DefaultStringAbbreviationThresholdBits);
    }
    return 0;
}
