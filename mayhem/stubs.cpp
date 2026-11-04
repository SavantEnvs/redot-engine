// The fuzz targets link only libcore, libdrivers and the codec modules; building the whole engine
// (scene + servers) takes tens of minutes per configuration. Core and the PNG driver still carry a few
// references into servers/scene (Engine::set_max_fps, RemoteDebugger, the PNG saver's ImageTexture
// cast). None is reachable from the parsers under test, so inert definitions satisfy the linker.
#include "servers/display/display_server.h"
#include "scene/property_utils.h"
#include "scene/resources/packed_scene.h"
#include "servers/rendering/rendering_device.h"
#include "servers/rendering/rendering_server.h"

DisplayServer *DisplayServer::singleton = nullptr;
bool DisplayServer::consume_main_window_close_request() { return false; }
RenderingDevice *RenderingDevice::get_singleton() { return nullptr; }
void RenderingDevice::_set_max_fps(int) {}

RenderingServer *RenderingServer::get_singleton() { return nullptr; }
Ref<SceneState> PackedScene::get_state() const { return Ref<SceneState>(); }
int SceneState::get_node_count() const { return 0; }
StringName SceneState::get_node_type(int) const { return StringName(); }
int SceneState::get_node_property_count(int) const { return 0; }
Variant SceneState::get_node_property_value(int, int) const { return Variant(); }

Variant PropertyUtils::get_property_default_value(const Object *, const StringName &, bool *, const Vector<SceneState::PackState> *, bool, const Node *, bool *) { return Variant(); }

char mayhem_ti_packed_scene[64] asm("_ZTI11PackedScene");
char mayhem_ti_rendering_server[64] asm("_ZTI15RenderingServer");
char mayhem_ti_display_server[64] asm("_ZTI13DisplayServer");
char mayhem_ti_image_texture[64] asm("_ZTI12ImageTexture");
char mayhem_ti_rendering_device[64] asm("_ZTI15RenderingDevice");
