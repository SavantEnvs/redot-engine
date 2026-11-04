#include "common.h"

#include "core/io/json.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	String text = mayhem_utf8(data, size);

	JSON json;
	if (json.parse(text, true) != OK) {
		return -1;
	}
	Variant parsed = json.get_data();
	String out = JSON::stringify(parsed, "\t", true, true);
	String compact = JSON::stringify(parsed);

	JSON again;
	if (again.parse(out) == OK) {
		(void)JSON::stringify(again.get_data());
	}
	Variant native = JSON::from_native(parsed, false);
	JSON::to_native(native, false);
	return 0;
}
