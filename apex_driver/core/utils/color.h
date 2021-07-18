#pragma once
#include <ntifs.h>

struct color {
	int a, r, g, b;
	color() = default;

	color(int r, int g, int b, int a = 255) {
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	color(ULONG color) {
		this->a = (color >> 24) & 0xff;
		this->r = (color >> 16) & 0xff;
		this->g = (color >> 8) & 0xff;
		this->b = (color & 0xff);
	}

	color from_uint(ULONG uint) {
		return color(uint);
	}

	static color black(int a = 255) { return { 0, 0, 0, a }; }
	static color white(int a = 255) { return { 255, 255, 255, a }; }
	static color red(int   a = 255) { return { 255, 0, 0, a }; }
	static color green(int a = 255) { return { 0, 255, 0, a }; }
	static color blue(int  a = 255) { return { 0, 0, 255, a }; }
};