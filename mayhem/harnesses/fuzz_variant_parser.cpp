#include "common.h"

#include "core/io/resource.h"
#include "core/variant/variant_parser.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	String text = mayhem_utf8(data, size);

	VariantParser::StreamString stream;
	stream.s = text;

	String err_str;
	int err_line = 0;
	Variant value;
	if (VariantParser::parse(&stream, value, err_str, err_line) != OK) {
		return -1;
	}
	String written;
	VariantWriter::write_to_string(value, written);
	return 0;
}
