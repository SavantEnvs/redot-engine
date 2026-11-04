// Shared engine bring-up for the core-only fuzz harnesses (no display, no scene, no servers).
#pragma once

#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/core_globals.h"
#include "core/object/class_db.h"
#include "core/register_core_types.h"

#include "drivers/register_driver_types.h"
#include "drivers/unix/dir_access_unix.h"
#include "drivers/unix/file_access_unix.h"
#include "drivers/unix/os_unix.h"

#include <cstdint>
#include <cstdlib>

// register_core_types() consults the OS singleton; OS_Unix from libdrivers supplies everything but the main loop.
class MayhemOS : public OS_Unix {
	virtual void initialize() override {}
	virtual void initialize_joypads() override {}
	virtual void set_main_loop(MainLoop *) override {}
	virtual void delete_main_loop() override {}
	virtual MainLoop *get_main_loop() const override { return nullptr; }
	virtual void finalize() override {}
	virtual bool _check_internal_feature_support(const String &) override { return false; }
};

static inline void mayhem_engine_init() {
	static bool done = false;
	if (done) {
		return;
	}
	done = true;
	static MayhemOS os;
	memnew(Engine);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_RESOURCES);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_RESOURCES);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_USERDATA);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_FILESYSTEM);
	register_core_types();
	memnew(ProjectSettings);
	register_core_driver_types();
	// Error macros print to stderr on every rejected input; muting them keeps exec/s high.
	CoreGlobals::print_error_enabled = false;
	CoreGlobals::print_line_enabled = false;
}

#if !defined(MAYHEM_CUSTOM_INIT) && !defined(DOCTEST_CONFIG_IMPLEMENT)
extern "C" int LLVMFuzzerInitialize(int *, char ***) {
	mayhem_engine_init();
	return 0;
}
#endif

static inline String mayhem_utf8(const uint8_t *p_data, size_t p_size) {
	String s;
	s.append_utf8(reinterpret_cast<const char *>(p_data), (int)p_size);
	return s;
}
