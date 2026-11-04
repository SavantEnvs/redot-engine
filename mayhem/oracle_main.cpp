// Doctest runner over upstream's own core test cases (JSON, XML, marshalls, expressions, strings,
// config files, fuzzy search, regex). Same cases the engine's `--test` mode runs, minus the
// display/scene fixtures the headless fuzz build doesn't carry.
#define DOCTEST_CONFIG_IMPLEMENT
#define DOCTEST_CONFIG_NO_EXCEPTIONS_BUT_WITH_ALL_ASSERTS
#include "harnesses/common.h"

#include "modules/regex/tests/test_regex.h"
#include "tests/core/io/test_config_file.h"
#include "tests/core/io/test_json.h"
#include "tests/core/io/test_marshalls.h"
#include "tests/core/io/test_xml_parser.h"
#include "tests/core/math/test_expression.h"
#include "tests/test_utils.h"
#include "tests/core/string/test_string.h"

int main(int argc, char **argv) {
	mayhem_engine_init();
	CoreGlobals::print_error_enabled = true;
	doctest::Context context(argc, argv);
	int rc = context.run();
	unregister_core_driver_types();
	unregister_core_types();
	return rc;
}
