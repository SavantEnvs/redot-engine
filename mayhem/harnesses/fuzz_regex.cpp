#include "common.h"

#include "modules/regex/regex.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	// pattern and subject separated by the first NUL byte
	size_t split = 0;
	while (split < size && data[split] != 0) {
		split++;
	}
	if (split == size) {
		return -1;
	}
	String pattern = mayhem_utf8(data, split);
	String subject = mayhem_utf8(data + split + 1, size - split - 1);

	Ref<RegEx> re;
	re.instantiate();
	if (re->compile(pattern, false) != OK) {
		return -1;
	}
	re->get_group_count();
	re->get_names();
	TypedArray<RegExMatch> all = re->search_all(subject);
	Ref<RegExMatch> m = re->search(subject);
	re->sub(subject, "$0-$1", true);
	return 0;
}
