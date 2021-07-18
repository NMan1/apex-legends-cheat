#pragma once
#include <ntifs.h>
#include "..\utils\memory\memory.h"
#include "..\utils\math\vectors.h"
#include "..\utils\color.h"

namespace sdk {
	extern HANDLE pid;

	extern PEPROCESS process;

	extern uintptr_t module_base;

	struct view_matrix_t {
		float matrix[16];
	};

	struct bone_t {
		BYTE pad[0xCC];
		float x;
		BYTE pad2[0xC];
		float y;
		BYTE pad3[0xC];
		float z;
	};

	uintptr_t get_local_player();

	bool is_player(uintptr_t player);

	bool is_alive(uintptr_t player);

	int get_health(uintptr_t player);

	int get_max_health(uintptr_t player);

	int get_team(uintptr_t player);

	vec3_t get_view_angles(uintptr_t player);

	void set_view_angles(uintptr_t player, vec3_t angle);

	vec3_t get_aim_punch(uintptr_t player);

	vec3_t get_origin(uintptr_t player);

	vec3_t get_bone_pos(uintptr_t ent, int id);

	bool world_to_screen(float* view_matrix, vec3_t world, vec2_t& screen);

	void enable_glow(uintptr_t ent, color clr);
}