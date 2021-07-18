#include "features.h"
#include "..\renderer\renderer.h"

void features::esp(uintptr_t local_player) {
	auto view_renderer = memory::read<uintptr_t>(sdk::process, sdk::module_base + 0x41e3588);
	if (!view_renderer)
		return;

	auto view_matrix_ = memory::read<uintptr_t>(sdk::process, view_renderer + 0x1b3bd0);
	if (!view_matrix_)
		return;

	auto view_matrix = memory::read<sdk::view_matrix_t>(sdk::process, view_matrix_);

	for (int i = 0; i <= 100; i++) {
		auto entity = memory::read<uintptr_t>(sdk::process, (sdk::module_base + 0x1911448) + ((uintptr_t)i << 5));
		if (!entity || entity == local_player)
			continue;

		if (!sdk::is_player(entity))
			continue;

		if (!sdk::is_alive(entity))
			continue;

		if (sdk::get_team(entity) == sdk::get_team(local_player))
			continue;

		sdk::enable_glow(entity, { 124, 223, 100 });

		if (sdk::get_origin(local_player).distance_to(sdk::get_origin(entity)) * 0.01905f > 200) // 200m
			continue;

		auto head_pos = sdk::get_bone_pos(entity, 8);
		vec2_t head_pos_2d{};
		if (!sdk::world_to_screen(view_matrix.matrix, head_pos, head_pos_2d))
			continue;
		
		auto origin_pos = memory::read<vec3_t>(sdk::process, entity + 0x014c);
		vec2_t origin_pos_2d{};
		if (!sdk::world_to_screen(view_matrix.matrix, origin_pos, origin_pos_2d))
			continue;

		const float height = origin_pos_2d.y - head_pos_2d.y;
		const float width = height / 2.f;
		const float y = head_pos_2d.y;
		const float x = origin_pos_2d.x - (width / 2.f);

		render::draw_box(x, y, width, height, 1, { 0, 0, 0 });

		const auto health_height = (height - 2) * ((float)sdk::get_health(entity) / (float)sdk::get_max_health(entity));
		render::draw_filled_box(x - 6, (y + height) - health_height, 3, health_height, { 222, 49, 99 });
		render::draw_box(x - 6 - 1, y, 4, height, 1, { 0, 0, 0 });
	}
}