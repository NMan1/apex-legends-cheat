#include "features.h"

void features::rcs(uintptr_t local_player) {
	auto view_angles = sdk::get_view_angles(local_player);
	auto punch = sdk::get_aim_punch(local_player) * 2;

	punch.x *= 0.5;
	punch.y *= 0.5;
	punch.z = 0;

	auto rcs_angle = view_angles - punch;
	rcs_angle.normalize();
	sdk::set_view_angles(local_player, rcs_angle);
}