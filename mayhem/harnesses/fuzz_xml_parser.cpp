#include "common.h"

#include "core/io/xml_parser.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	Vector<uint8_t> buf;
	buf.resize(size);
	if (size) {
		memcpy(buf.ptrw(), data, size);
	}
	Ref<XMLParser> parser;
	parser.instantiate();
	if (parser->open_buffer(buf) != OK) {
		return -1;
	}
	while (parser->read() == OK) {
		switch (parser->get_node_type()) {
			case XMLParser::NODE_ELEMENT: {
				parser->get_node_name();
				for (int i = 0; i < parser->get_attribute_count(); i++) {
					parser->get_attribute_name(i);
					parser->get_attribute_value(i);
				}
				parser->is_empty();
			} break;
			case XMLParser::NODE_ELEMENT_END:
				parser->get_node_name();
				break;
			default:
				parser->get_node_data();
				break;
		}
		parser->get_current_line();
	}
	return 0;
}
