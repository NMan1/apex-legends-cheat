#pragma once
#include "..\utils\imports.h"
#include "..\utils\color.h"

namespace render {
	extern gdi_select_brush_t gdi_select_brush;

	extern pal_blt_t nt_gdi_pal_blt;

	extern nt_user_get_dc_t nt_user_get_dc;

	extern nt_gdi_create_solid_brush_t nt_gdi_create_solid_brush;

	extern release_dc_t release_dc;

	extern delete_object_app_t delete_object_app;

	bool init();

	void draw_box(int x, int y, int w, int h, int thickness, color clr);

	void draw_filled_box(int x, int y, int w, int h, color clr);
}