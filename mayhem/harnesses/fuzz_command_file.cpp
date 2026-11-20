// Command files (`slang -f <file>` / `-F` / `-C`): Driver::processCommandFiles reads the file
// and hands its text to Driver::parseCommandLine with exactly these ParseOptions (comments,
// environment-variable expansion, no program name, duplicates allowed), then the tool runs
// Driver::processOptions. This harness does the same on the input bytes, over the full set of
// standard options a slang tool registers. It stops before parsing sources, so nothing is written.
#include <cstdint>
#include <string_view>

#include "slang/driver/Driver.h"
#include "slang/util/OS.h"

using namespace slang;
using namespace slang::driver;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    // --color-diagnostics flips these process-wide flags: reset them for every input.
    OS::setStderrColorsEnabled(false);
    OS::setStdoutColorsEnabled(false);

    std::string_view text(reinterpret_cast<const char*>(data), size);
    Driver driver;
    driver.addStandardArgs();

    CommandLine::ParseOptions parseOpts;
    parseOpts.expandEnvVars = true;
    parseOpts.ignoreProgramName = true;
    parseOpts.supportComments = true;
    parseOpts.ignoreDuplicates = true;
    if (driver.parseCommandLine(text, parseOpts))
        (void)driver.processOptions(/* checkFiles */ false);
    return 0;
}
