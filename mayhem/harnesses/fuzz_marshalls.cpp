#include "common.h"

#include "core/io/marshalls.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	if (size < 1) {
		return -1;
	}
	// First byte picks whether object decoding is allowed; the rest is the encoded variant.
	bool allow_objects = data[0] & 1;
	Variant v;
	int consumed = 0;
	if (decode_variant(v, data + 1, (int)size - 1, &consumed, allow_objects) != OK) {
		return -1;
	}
	Vector<uint8_t> encoded;
	int len = 0;
	if (encode_variant(v, nullptr, len, allow_objects) == OK && len > 0 && len < (1 << 24)) {
		encoded.resize(len);
		encode_variant(v, encoded.ptrw(), len, allow_objects);
	}
	return 0;
}
