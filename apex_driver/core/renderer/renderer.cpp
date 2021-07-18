#include "renderer.h"
#include "..\utils\memory\memory.h"
#include "..\utils\color.h"

#define PATCOPY (DWORD)0x00F00021
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

namespace render {
	gdi_select_brush_t gdi_select_brush = NULL;

	pal_blt_t nt_gdi_pal_blt = NULL;

	nt_user_get_dc_t nt_user_get_dc = NULL;

	nt_gdi_create_solid_brush_t nt_gdi_create_solid_brush = NULL;

	release_dc_t release_dc = NULL;

	delete_object_app_t delete_object_app = NULL;

	gre_ext_text_out_w_internal_t gre_ext_text_out_w_internal = NULL;

	bool init() {
		gdi_select_brush = (gdi_select_brush_t)memory::get_system_module_export("\\SystemRoot\\System32\\win32kfull.sys", "NtGdiSelectBrush");
		if (!gdi_select_brush) {
			print("failed to get gdi_select_brush\n");
			return false;
		}

		nt_gdi_pal_blt = (pal_blt_t)memory::get_system_module_export("\\SystemRoot\\System32\\win32kfull.sys", "NtGdiPatBlt");
		if (!nt_gdi_pal_blt) {
			print("failed to get nt_gdi_pal_blt\n");
			return false;
		}

		nt_user_get_dc = (nt_user_get_dc_t)memory::get_system_module_export("\\SystemRoot\\System32\\win32kbase.sys", "NtUserGetDC");
		if (!nt_user_get_dc) {
			print("failed to get nt_user_get_dc\n");
			return false;
		}

		nt_gdi_create_solid_brush = (nt_gdi_create_solid_brush_t)memory::get_system_module_export("\\SystemRoot\\System32\\win32kfull.sys", "NtGdiCreateSolidBrush");
		if (!nt_gdi_create_solid_brush) {
			print("failed to get nt_gdi_create_solid_brush\n");
			return false;
		}

		release_dc = (release_dc_t)memory::get_system_module_export("\\SystemRoot\\System32\\win32kbase.sys", "NtUserReleaseDC");
		if (!release_dc) {
			print("failed to get release_dc\n");
			return false;
		}

		delete_object_app = (delete_object_app_t)memory::get_system_module_export("\\SystemRoot\\System32\\win32kbase.sys", "NtGdiDeleteObjectApp");
		if (!delete_object_app) {
			print("failed to get delete_object_app\n");
			return false;
		}
		
		// text rendering stuff, never finished it coulkdnt be fucked, look up gre_ext_text_out_w_internal if u want to finish it 
		//{
		//	uintptr_t address = (uintptr_t)memory::get_system_module_export("\\SystemRoot\\System32\\win32kfull.sys", "NtGdiExtTextOutW");
		//	if (!address) {
		//		print("failed to get gre_ext_text_out_w_internal address\n");
		//		return false;
		//	}

		//	//address = (uintptr_t)memory::pattern_scan((PVOID)address, "\x8B\x54\x24\x78\xE8\xFF\xFF\xFF\xFF\x8B\xF0", "xxxxx????xx");
		//	//if (!address) {
		//	//	print("failed to get scan gre_ext_text_out_w_internal address\n");
		//	//	return false;
		//	//}

		//	//address += 5;
		//	//gre_ext_text_out_w_internal = (gre_ext_text_out_w_internal_t)(address + *reinterpret_cast<INT32*>(address) + sizeof(INT32));

		//	print("address %p\n", address);
		//}

		return true;
	}

	bool frame_rect(HDC hdc, const RECT* lprc, HBRUSH hbr, int thickness) {
		RECT r = *lprc;

		HBRUSH old_brush = gdi_select_brush(hdc, hbr);
		if (!old_brush)
			return false;

		nt_gdi_pal_blt(hdc, r.left, r.top, thickness, r.bottom - r.top, PATCOPY);
		nt_gdi_pal_blt(hdc, r.right - thickness, r.top, thickness, r.bottom - r.top, PATCOPY);
		nt_gdi_pal_blt(hdc, r.left, r.top, r.right - r.left, thickness, PATCOPY);
		nt_gdi_pal_blt(hdc, r.left, r.bottom - thickness, r.right - r.left, thickness, PATCOPY);

		gdi_select_brush(hdc, old_brush);
		return true;
	}
	
	bool filled_rect(HDC hdc, const RECT* lprc, HBRUSH hbr) {
		RECT r = *lprc;

		HBRUSH old_brush = gdi_select_brush(hdc, hbr);
		if (!old_brush)
			return false;

		nt_gdi_pal_blt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, PATCOPY);

		gdi_select_brush(hdc, old_brush);
		return true;
	}

	void draw_box(int x, int y, int w, int h, int thickness, color clr) {
		HDC hdc = nt_user_get_dc(NULL);
		if (!hdc)
			return;

		HBRUSH brush = nt_gdi_create_solid_brush(RGB(clr.r, clr.g, clr.b), NULL);
		if (!brush)
			return;

		RECT rect{ x, y, x + w, y + h };
		frame_rect(hdc, &rect, brush, thickness);
		release_dc(hdc);
		delete_object_app(brush);
	}
	
	void draw_filled_box(int x, int y, int w, int h, color clr) {
		HDC hdc = nt_user_get_dc(NULL);
		if (!hdc)
			return;

		HBRUSH brush = nt_gdi_create_solid_brush(RGB(clr.r, clr.g, clr.b), NULL);
		if (!brush)
			return;

		RECT rect{ x, y, x + w, y + h };
		filled_rect(hdc, &rect, brush);
		release_dc(hdc);
		delete_object_app(brush);
	}
}