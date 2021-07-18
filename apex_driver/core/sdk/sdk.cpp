#include "sdk.h"

namespace sdk {
	HANDLE pid{};

	PEPROCESS process{};

	uintptr_t module_base{};

	uintptr_t get_local_player() {
		return memory::read<uintptr_t>(process, module_base + 0x1cc0ce8);
	}

	bool is_player(uintptr_t player) {
		return (memory::read<uintptr_t>(process, player + 0x0589) == 125780153691248);
	}

	bool is_alive(uintptr_t player) {
		return memory::read<int>(process, player + 0x26c8) == 0 && memory::read<int>(process, player + 0x0798) == 0;
	}

	int get_health(uintptr_t player) {
		return memory::read<int>(process, player + 0x0438);
	}

	int get_max_health(uintptr_t player) {
		return memory::read<int>(process, player + 0x0578);
	}

	int get_team(uintptr_t player) {
		return memory::read<int>(process, player + 0x0448);
	}

	vec3_t get_view_angles(uintptr_t player) {
		return memory::read<vec3_t>(process, player + 0x2518);
	}

	void set_view_angles(uintptr_t player, vec3_t angle) {
		memory::write<vec3_t>(process, player + 0x2518, angle);
	}

	vec3_t get_aim_punch(uintptr_t player) {
		return memory::read<vec3_t>(process, player + 0x2430);
	}

	vec3_t get_origin(uintptr_t player) {
		return memory::read<vec3_t>(process, player + 0x14C);
	}

	vec3_t get_bone_pos(uintptr_t player, int id) {
		vec3_t origin_pos = get_origin(player);
		uintptr_t bones_array = memory::read<uintptr_t>(process, player + 0xf38);
		vec3_t bone_pos{};

		bone_pos.x = memory::read<float>(process, bones_array + 0xCC + (id * 0x30)) + origin_pos.x;
		bone_pos.y = memory::read<float>(process, bones_array + 0xDC + (id * 0x30)) + origin_pos.y;
		bone_pos.z = memory::read<float>(process, bones_array + 0xEC + (id * 0x30)) + origin_pos.z;

		return bone_pos;
	}

	bool world_to_screen(float* view_matrix, vec3_t world, vec2_t& screen) {
		float* m_vMatrix = view_matrix;
		float w = m_vMatrix[12] * world.x + m_vMatrix[13] * world.y + m_vMatrix[14] * world.z + m_vMatrix[15];

		if (w < 0.01f)
			return false;

		screen.x = m_vMatrix[0] * world.x + m_vMatrix[1] * world.y + m_vMatrix[2] * world.z + m_vMatrix[3];
		screen.y = m_vMatrix[4] * world.x + m_vMatrix[5] * world.y + m_vMatrix[6] * world.z + m_vMatrix[7];

		float invw = 1.0f / w;
		screen.x *= invw;
		screen.y *= invw;

		float x = 1920 / 2;
		float y = 1080 / 2;

		x += 0.5 * screen.x * 1920 + 0.5;
		y -= 0.5 * screen.y * 1080 + 0.5;

		screen.x = x;
		screen.y = y;

		if (screen.x > 1920 || screen.x < 0 || screen.y > 1080 || screen.y < 0)
			return false;

		return true;
	}

	void enable_glow(uintptr_t player, color clr) {
		if (memory::read<int>(process, player + 0x1D8) == 2) {
			return;
		}
			
		struct glow_settings { BYTE glow_mode, border_mode, border_size, transparent_level; };
		memory::write<int>(process, player + 0x3c8, 1);
		memory::write<int>(process, player + 0x3d0, 2); 
		memory::write<glow_settings>(process, player + 0x2C4, { 101, 101, 46, 90 });
		memory::write<float>(process, player + 0x1D0, 61.f);  // r
		memory::write<float>(process, player + 0x1D4, 2.f);  // g
		memory::write<float>(process, player + 0x1D8, 2.f);  // b
	}
}