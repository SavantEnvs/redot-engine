#include "common.h"

#include "core/math/expression.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	String text = mayhem_utf8(data, size);

	Expression expr;
	Vector<String> names;
	names.push_back("x");
	names.push_back("y");
	if (expr.parse(text, names) != OK) {
		return -1;
	}
	Array inputs;
	inputs.push_back(1);
	inputs.push_back(Vector2(2, 3));
	expr.execute(inputs, nullptr, false);
	return 0;
}
