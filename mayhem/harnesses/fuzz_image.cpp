#define MAYHEM_CUSTOM_INIT
#include "common.h"

#include "core/io/image.h"
#include "core/io/image_loader.h"
#include "drivers/png/image_loader_png.h"
#include "modules/bmp/image_loader_bmp.h"
#include "modules/jpg/image_loader_libjpeg_turbo.h"
#include "modules/svg/register_types.h"
#include "modules/tga/image_loader_tga.h"
#include "modules/webp/image_loader_webp.h"

static Ref<ImageLoaderPNG> png_loader;
static Ref<ImageLoaderLibJPEGTurbo> jpg_loader;
static Ref<ImageLoaderWebP> webp_loader;
static Ref<ImageLoaderTGA> tga_loader;
static Ref<ImageLoaderBMP> bmp_loader;

extern "C" int LLVMFuzzerInitialize(int *, char ***) {
	mayhem_engine_init();
	png_loader.instantiate();
	jpg_loader.instantiate();
	webp_loader.instantiate();
	tga_loader.instantiate();
	bmp_loader.instantiate();
	// As the engine does: starts ThorVG's software engine (SwCanvas::gen() is null without it), then
	// creates and registers the SVG loader.
	initialize_svg_module(MODULE_INITIALIZATION_LEVEL_SCENE);
	return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	if (size < 2) {
		return -1;
	}
	// byte 0 selects the codec; the rest is the file.
	uint8_t kind = data[0] % 6;
	Vector<uint8_t> buf;
	buf.resize(size - 1);
	memcpy(buf.ptrw(), data + 1, size - 1);

	Ref<Image> img;
	img.instantiate();
	Error err = ERR_UNAVAILABLE;
	switch (kind) {
		case 0:
			err = img->load_png_from_buffer(buf);
			break;
		case 1:
			err = img->load_jpg_from_buffer(buf);
			break;
		case 2:
			err = img->load_webp_from_buffer(buf);
			break;
		case 3:
			err = img->load_tga_from_buffer(buf);
			break;
		case 4:
			err = img->load_bmp_from_buffer(buf);
			break;
		case 5:
			err = img->load_svg_from_buffer(buf);
			break;
	}
	if (err != OK || img->is_empty()) {
		return -1;
	}
	// Exercise the decoded pixels so a bad width/height/format shows up.
	if (img->get_width() * (int64_t)img->get_height() <= (1 << 22)) {
		(void)img->get_pixel(0, 0);
		(void)img->get_pixel(img->get_width() - 1, img->get_height() - 1);
		(void)img->get_used_rect();
	}
	return 0;
}
