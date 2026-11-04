#include "common.h"

#include "core/io/config_file.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	String text = mayhem_utf8(data, size);

	Ref<ConfigFile> cf;
	cf.instantiate();
	if (cf->parse(text, false) != OK) {
		return -1;
	}
	Vector<String> sections = cf->get_sections();
	for (const String &section : sections) {
		Vector<String> keys = cf->get_section_keys(section);
		for (const String &key : keys) {
			cf->get_value(section, key);
		}
	}
	cf->encode_to_text();
	return 0;
}
