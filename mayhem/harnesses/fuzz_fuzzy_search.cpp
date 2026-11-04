#include "common.h"

#include "core/string/fuzzy_search.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	if (size < 2) {
		return -1;
	}
	// byte 0: option bits; remaining: "query\n target1\n target2\n ..."
	uint8_t opts = data[0];
	String text = mayhem_utf8(data + 1, size - 1);
	Vector<String> lines = text.split("\n");
	if (lines.size() < 2) {
		return -1;
	}
	FuzzySearch search;
	search.max_misses = opts & 3;
	search.allow_subsequences = opts & 4;
	search.start_offset = (opts >> 3) & 7;
	search.set_query(lines[0], opts & 0x80);
	// Every engine caller skips an empty query; a token-less one reads alloca(0) in maybe_apply_score_bonus().
	if (lines[0].split(" ", false).is_empty()) {
		return -1;
	}

	PackedStringArray targets;
	for (int i = 1; i < lines.size(); i++) {
		targets.push_back(lines[i]);
	}
	Vector<FuzzySearchResult> results;
	search.search_all(targets, results);
	FuzzySearchResult one;
	search.search(lines[1], one);
	return 0;
}
