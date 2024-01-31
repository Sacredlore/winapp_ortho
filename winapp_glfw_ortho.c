/*
 ============================================================================
 Name        : winapp_glfw_ortho.c
 Author      : Sacredlore
 Version     :
 Created on  :
 Description : OpenGL Application
 ============================================================================
 */

#include "winapp_glfw_ortho.h"

int frame_width = 960 + 1, frame_height = 540 + 1;
float frame_offset_x = 0.0f, frame_offset_y = 0.0f;
float frame_scale_x = 1.0f, frame_scale_y = 1.0f;
int axis_direction = X_AXIS_RIGHT | Y_AXIS_DOWN;
int frame_alter_action = FRAME_NO_ALTER;

int16_t mouse_x = 0, mouse_y = 0; // mouse current position
int16_t prev_mouse_x = 0, prev_mouse_y = 0; // mouse previous position
int16_t mouse_dx = 0, mouse_dy = 0; // mouse distance
int16_t mouse_click_x = 0, mouse_click_y = 0; // mouse click position

int16_t ws_key = 0, ad_key = 0, qe_key = 0;
int16_t h_arrows_key = 0, v_arrows_key = 0, page_up_down_key = 0;
int16_t comma_period_key = 0;

int8_t ortho_grid_step = 10;

bool glcd_play = true;
rectangle glcd_rect = { 0, 0, 319, 239 };
uint8_t pager_min = 0, pager_max = 0;

struct {
	gui_titlebar titlebar;
	gui_textbox textboxes[5];
	gui_readings readings[5];
	gui_pager pager;
} gui =
{
	{ GUI_TITLEBAR, { 0, 0, 0, 0 }, GLCD_FONT_8X16T, "TITLEBAR1"},
	{
		{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 1, GLCD_FONT_5X7M, ALIGN_MID_LEFT, "label1"},
		{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 1, GLCD_FONT_5X7M, ALIGN_MID_RIGHT, "LABEL2"},
		{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 1, GLCD_FONT_5X7M, ALIGN_MID_LEFT, "qwerty"},
		{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 1, GLCD_FONT_5X7M, ALIGN_MID_RIGHT, "asdfgh"},
		{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 1, GLCD_FONT_5X7M, ALIGN_MID_LEFT, "zxcvbn"}
	},
	{
		{ GUI_READINGS, { 0, 0, 0, 0 }, 1, GLCD_FONT_3X6PRIZM, "READING1:", "000.000"},
		{ GUI_READINGS, { 0, 0, 0, 0 }, 1, GLCD_FONT_3X6PRIZM, "READING2:", "000.000"},
		{ GUI_READINGS, { 0, 0, 0, 0 }, 1, GLCD_FONT_8X16, "READING3:", "000.000"},
		{ GUI_READINGS, { 0, 0, 0, 0 }, 1, GLCD_FONT_8X8, "READING4:", "000.000"},
		{ GUI_READINGS, { 0, 0, 0, 0 }, 1, GLCD_FONT_8X8, "READING5:", "000.000"}
	},
	{ GUI_PAGER, { 0, 0, 0, 0 }, 1, GLCD_FONT_8X8, { "PAGE0", "PAGE1", "PAGE2", "PAGE3" } },
};

void glcd_set_pixel(int16_t x0, int16_t y0, uint16_t color) {
	uint16_t red, green, blue;

	red = RGB565_R(color);
	green = RGB565_G(color);
	blue = RGB565_B(color);

	glBegin(GL_POINTS);
	glColor4f(RGB_1_0(red), RGB_1_0(green), RGB_1_0(blue), 1.0f);
	glVertex2f((float) x0, (float) y0);
	glEnd();
}

void glcd_draw_hline(int16_t x0, int16_t y0, int16_t length, uint16_t color) {
	for (int i = 0; i < length; i++) {
		glcd_set_pixel(x0 + i, y0, color);
	}
}

void glcd_draw_vline(int16_t x0, int16_t y0, int16_t length, uint16_t color) {
	for (int i = 0; i < length; i++) {
		glcd_set_pixel(x0, y0 + i, color);
	}
}

long gcd_euclidis(long a, long b) { // global common divisor with operators
	long nod = 1L;
	long tmp;

	if (a == 0L)
		return b;
	if (b == 0L)
		return a;
	if (a == b)
		return a;
	if (a == 1L || b == 1L)
		return 1L;

	while (a != 0 && b != 0) {
		if (a % 2L == 0L && b % 2L == 0L) {
			nod *= 2L;
			a /= 2L;
			b /= 2L;
			continue;
		}
		if (a % 2L == 0L && b % 2L != 0L) {
			a /= 2L;
			continue;
		}
		if (a % 2L != 0L && b % 2L == 0L) {
			b /= 2L;
			continue;
		}
		if (a > b) {
			tmp = a;
			a = b;
			b = tmp;
		}
		tmp = a;
		a = (b - a) / 2L;
		b = tmp;
	}
	if (a == 0)
		return nod * b;
	else
		return nod * a;
}

long gcd_euclidis_bitshift(long a, long b) { // global common divisor with bits shift
	long nod = 1L;
	long tmp;

	if (a == 0L)
		return b;
	if (b == 0L)
		return a;
	if (a == b)
		return a;
	if (a == 1L || b == 1L)
		return 1L;

	while (a != 0 && b != 0) {
		if (((a & 1L) | (b & 1L)) == 0L) {
			nod <<= 1L;
			a >>= 1L;
			b >>= 1L;
			continue;
		}
		if (((a & 1L) == 0L) && (b & 1L)) {
			a >>= 1L;
			continue;
		}
		if ((a & 1L) && ((b & 1L) == 0L)) {
			b >>= 1L;
			continue;
		}
		if (a > b) {
			tmp = a;
			a = b;
			b = tmp;
		}
		tmp = a;
		a = (b - a) >> 1L;
		b = tmp;
	}
	if (a == 0)
		return nod * b;
	else
		return nod * a;
}

void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) { // Bresenham's line algorithm
	int dx = abs(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1 - y0);
	int sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2;

	while (x0 != x1 || y0 != y1) {
		glcd_set_pixel(x0, y0, color);

		int e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		}
	}
}

void draw_circle(int16_t x0, int16_t y0, int16_t radius, uint16_t color) { // midpoint circle algorithm
	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x = 0;
	int y = radius;

	glcd_set_pixel(x0, y0 + radius, color);
	glcd_set_pixel(x0, y0 - radius, color);
	glcd_set_pixel(x0 + radius, y0, color);
	glcd_set_pixel(x0 - radius, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x + 1;
		glcd_set_pixel(x0 + x, y0 + y, color);
		glcd_set_pixel(x0 - x, y0 + y, color);
		glcd_set_pixel(x0 + x, y0 - y, color);
		glcd_set_pixel(x0 - x, y0 - y, color);
		glcd_set_pixel(x0 + y, y0 + x, color);
		glcd_set_pixel(x0 - y, y0 + x, color);
		glcd_set_pixel(x0 + y, y0 - x, color);
		glcd_set_pixel(x0 - y, y0 - x, color);
	}
}

void font_size(uint8_t font_id, uint8_t *char_width, uint8_t *char_height) {
	switch (font_id) {
	case GLCD_FONT_8X16:
		*char_height = 16;
		*char_width = 8;
		break;
	case GLCD_FONT_8X14:
		*char_height = 14;
		*char_width = 8;
		break;
	case GLCD_FONT_8X12:
		*char_height = 12;
		*char_width = 8;
		break;
	case GLCD_FONT_8X8:
		*char_height = 8;
		*char_width = 8;
		break;
	case GLCD_FONT_5X7S:
		*char_height = 5 + 2; // + 2 (compensation)
		*char_width = 7;
		break;
	}
}

void draw_character8x16t(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 16; y++) {
		w = font8x16t[c][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(x + x0, y + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character8x16(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 16; y++) {
		w = font8x16[c][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(x + x0, y + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character8x14(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 14; y++) {
		w = font8x14[c][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(x + x0, y + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character8x12(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 12; y++) {
		w = font8x12[c][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(x + x0, y + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character8x8t(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 8; y++) {
		w = font8x8t[c][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(x + x0, y + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character8x8(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 8; y++) {
		w = font8x8[c][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(x + x0, y + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character8x8o(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 8; y++) {
		w = font8x8[c][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(x + x0, y + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character5x7s(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 5; y++) {
		w = font5x7s[c - 0x20][y];
		for (x = 0; x < 7; x++) {
			if (w & 1) { // character
				glcd_set_pixel(y + x0, x + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(y + x0, x + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character3x5wendy(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 3; y++) {
		w = font3x5wendy[c - 0x20][y];
		for (x = 0; x < 5; x++) {
			if (w & 1) { // character
				glcd_set_pixel(y + x0, x + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(y + x0, x + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character5x7m(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 5; y++) {
		w = font5x7m[c - 0x20][y];
		for (x = 0; x < 7; x++) {
			if (w & 1) { // character
				glcd_set_pixel(y + x0, x + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(y + x0, x + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character6x8zpix(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 6; y++) {
		w = font6x8zpix[c - 0x20][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(y + x0, x + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(y + x0, x + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character4x8tama(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 4; y++) {
		w = font5x8tama[c - 0x20][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(y + x0, x + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(y + x0, x + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}


void draw_character3x5prizm(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 3; y++) {
		w = font3x5prizm[c - 0x20][y];
		for (x = 0; x < 6; x++) {
			if (w & 1) { // character
				glcd_set_pixel(y + x0, x + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
//				glcd_set_pixel(y + x0, x + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_text(char text[], uint8_t font_id, int16_t x, int16_t y) {
	uint8_t length;

	length = (uint8_t) strlen(text);
	switch (font_id) {
	case GLCD_FONT_8X16:
		for (int i = 0; i < length; i++) {
			draw_character8x16(text[i], x + (i * 8), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X16T:
		for (int i = 0; i < length; i++) {
			draw_character8x16t(text[i], x + (i * 8), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X14:
		for (int i = 0; i < length; i++) {
			draw_character8x14(text[i], x + (i * 8), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X12:
		for (int i = 0; i < length; i++) {
			draw_character8x12(text[i], x + (i * 8), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X8T:
		for (int i = 0; i < length; i++) {
			draw_character8x8t(text[i], x + (i * 8), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X8:
		for (int i = 0; i < length; i++) {
			draw_character8x8(text[i], x + (i * 8), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X8O:
		for (int i = 0; i < length; i++) {
			draw_character8x8o(text[i], x + (i * 8), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_5X7S:
		for (int i = 0; i < length; i++) {
			draw_character5x7s(text[i], x + (i * 6), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_3X5W:
		for (int i = 0; i < length; i++) {
			draw_character3x5wendy(text[i], x + (i * 6), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_5X7M:
		for (int i = 0; i < length; i++) {
			draw_character5x7m(text[i], x + (i * 6), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_6X8ZPIX:
		for (int i = 0; i < length; i++) {
			draw_character6x8zpix(text[i], x + (i * 7), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_4X8TAMA:
		for (int i = 0; i < length; i++) {
			draw_character4x8tama(text[i], x + (i * (4 + 1)), y); //  x + i * spacing
		}
		break;
	case GLCD_FONT_3X6PRIZM:
		for (int i = 0; i < length; i++) {
			draw_character3x5prizm(text[i], x + (i * 4), y); //  x + i * spacing
		}
		break;
	}
}

void outline_text(uint8_t length, uint8_t font_id, rectangle *rect) {
	rect->x0 = 0;
	rect->y0 = 0;
	rect->x1 = 0;
	rect->y1 = 0;

	switch (font_id) { // rectangle touches the edges of the letters
	case GLCD_FONT_8X16T:
		rect->x1 = (length * 8) - 2; // length * spacing - compensation
		rect->y1 = 16 - 1;           // height
		break;
	case GLCD_FONT_8X16:
		rect->x1 = (length * 8) - 2; // length * spacing - compensation
		rect->y1 = 16 - 1;           // height
		break;
	case GLCD_FONT_8X14:
		rect->x1 = (length * 8) - 2; // length * spacing - compensation
		rect->y1 = 14 - 1;           // height
		break;
	case GLCD_FONT_8X12:
		rect->x1 = (length * 8) - 2; // length * spacing - compensation
		rect->y1 = 12 - 1;           // height
		break;
	case GLCD_FONT_8X8T:
		rect->x1 = (length * 8) - 2; // length * spacing - compensation
		rect->y1 = 8 - 1;            // height
		break;
	case GLCD_FONT_8X8:
		rect->x1 = (length * 8) - 2; // length * spacing - compensation
		rect->y1 = 8 - 1;            // height
		break;
	case GLCD_FONT_8X8O:
		rect->x1 = (length * 8) - 2; // length * spacing - compensation
		rect->y1 = 8 - 1;            // height
		break;
	case GLCD_FONT_5X7S:
		rect->x1 = (length * 6) - 2; // length * spacing - compensation
		rect->y1 = 7 - 1;            // height
		break;
	case GLCD_FONT_3X5W:
		rect->x1 = (length * 6) - 4; // length * spacing - compensation
		rect->y1 = 5 - 1;            // height
		break;
	case GLCD_FONT_5X7M:
		rect->x1 = (length * 6) - 2; // length * spacing - compensation
		rect->y1 = 7 - 1;            // height
		break;
	case GLCD_FONT_6X8ZPIX:
		rect->x1 = (length * 7) - 2; // length * spacing - compensation
		rect->y1 = 8 - 1;            // height
		break;
	case GLCD_FONT_4X8TAMA:
		rect->x1 = (length * (4 + 1)) - 2; // length * spacing - compensation
		rect->y1 = 8 - 1;            // height
		break;
	case GLCD_FONT_3X6PRIZM:
		rect->x1 = (length * 4) - 2; // length * spacing - compensation
		rect->y1 = 6 - 1;            // height
		break;
	}
}

void draw_plain_rectangle(rectangle rect_a, uint16_t color) {
	rectangle rect_b;

	rect_b.x0 = min(rect_a.x0, rect_a.x1);
	rect_b.y0 = min(rect_a.y0, rect_a.y1);
	rect_b.x1 = max(rect_a.x0, rect_a.x1);
	rect_b.y1 = max(rect_a.y0, rect_a.y1);

	glcd_draw_vline(rect_b.x0, rect_b.y0, rect_b.y1 - rect_b.y0 + 1, color); // left
	glcd_draw_hline(rect_b.x0, rect_b.y0, rect_b.x1 - rect_b.x0 + 1, color); // top
	glcd_draw_vline(rect_b.x1, rect_b.y0, rect_b.y1 - rect_b.y0 + 1, color); // right
	glcd_draw_hline(rect_b.x0, rect_b.y1, rect_b.x1 - rect_b.x0 + 1, color); // bottom
}

uint16_t rectangle_get_width(rectangle rect_a) {
	return labs(max(rect_a.x0, rect_a.x1) - min(rect_a.x0, rect_a.x1)) + 1;
}

uint16_t rectangle_get_height(rectangle rect_a) {
	return labs(max(rect_a.y0, rect_a.y1) - min(rect_a.y0, rect_a.y1)) + 1;
}

void draw_painted_rectangle(rectangle rect_a, uint16_t color) {
	rectangle rect_b;
	uint16_t width, height;

	rect_b.x0 = min(rect_a.x0, rect_a.x1);
	rect_b.y0 = min(rect_a.y0, rect_a.y1);
	rect_b.x1 = max(rect_a.x0, rect_a.x1);
	rect_b.y1 = max(rect_a.y0, rect_a.y1);

	width = rectangle_get_width(rect_b);
	height = rectangle_get_height(rect_b);

	if (width < height) // optimize fill direction
		while (width--)
			glcd_draw_vline(rect_b.x0 + width, rect_b.y0, height, color);
	else
		while (height--)
			glcd_draw_hline(rect_b.x0, rect_b.y0 + height, width, color);
}

bool rectangle_contains_point(int16_t x0, int16_t y0, rectangle rect_a) {
	rectangle rect_b;

	rect_b.x0 = min(rect_a.x0, rect_a.x1);
	rect_b.y0 = min(rect_a.y0, rect_a.y1);
	rect_b.x1 = max(rect_a.x0, rect_a.x1);
	rect_b.y1 = max(rect_a.y0, rect_a.y1);

//	if (x0 > rect_b.x0 && x0 < rect_b.x1 && y0 > rect_b.y0 && y0 < rect_b.y1) // without contiguity
//		return true;

	if (x0 >= rect_b.x0 && x0 <= rect_b.x1 && y0 >= rect_b.y0 && y0 <= rect_b.y1) // with contiguity
		return true;

	return false;
}

void scale_rectangle(rectangle rect_a, rectangle *rect_b, int16_t scale_x, int16_t scale_y) {
	rectangle rect_c;

	rect_c.x0 = min(rect_a.x0, rect_a.x1);
	rect_c.y0 = min(rect_a.y0, rect_a.y1);
	rect_c.x1 = max(rect_a.x0, rect_a.x1);
	rect_c.y1 = max(rect_a.y0, rect_a.y1);

	rect_b->x0 = rect_c.x0 - scale_x;
	rect_b->y0 = rect_c.y0 - scale_y;
	rect_b->x1 = rect_c.x1 + scale_x;
	rect_b->y1 = rect_c.y1 + scale_y;
}

void offset_rectangle(rectangle rect_a, rectangle *rect_b, int16_t offset_x, int16_t offset_y) {
	rect_b->x0 = rect_a.x0 + offset_x;
	rect_b->y0 = rect_a.y0 + offset_y;
	rect_b->x1 = rect_a.x1 + offset_x;
	rect_b->y1 = rect_a.y1 + offset_y;
}

void union_rectangle(rectangle rect_a, rectangle rect_b, rectangle *rect_c) {
	rect_c->x0 = min(rect_a.x0, rect_b.x0);
	rect_c->y0 = min(rect_a.y0, rect_b.y0);
	rect_c->x1 = max(rect_a.x1, rect_b.x1);
	rect_c->y1 = max(rect_a.y1, rect_b.y1);
}

void between_rectangle(rectangle rect_a, rectangle rect_b, rectangle *rect_c) {
	rect_c->x0 = max(rect_a.x0, rect_b.x0);
	rect_c->y0 = max(rect_a.y0, rect_b.y0);
	rect_c->x1 = min(rect_a.x1, rect_b.x1);
	rect_c->y1 = min(rect_a.y1, rect_b.y1);
}

bool rectangles_intersect(rectangle rect_a, rectangle rect_b, rectangle *rect_c) {
	int16_t intersect_left = max(rect_a.x0, rect_b.x0);
	int16_t intersect_top = max(rect_a.y0, rect_b.y0);
	int16_t intersect_right = min(rect_a.x1, rect_b.x1);
	int16_t intersect_bottom = min(rect_a.y1, rect_b.y1);

	int16_t intersect_width  = intersect_right - intersect_left;
	int16_t intersect_height = intersect_bottom - intersect_top;

    if (intersect_width >= 0 && intersect_height >= 0) { // with contiguity
    	if (rect_c != NULL) {
    		rect_c->x0 = max(rect_a.x0, rect_b.x0); // left
    		rect_c->y0 = max(rect_a.y0, rect_b.y0); // top
    		rect_c->x1 = min(rect_a.x1, rect_b.x1); // right
    		rect_c->y1 = min(rect_a.y1, rect_b.y1); // bottom
    	}
    	return true;
    }

    return false;
}

bool rectangles_fit(rectangle rect_a, rectangle rect_b) {
	if (rectangle_contains_point(rect_b.x0, rect_b.y0, rect_a) &&
			rectangle_contains_point(rect_b.x1, rect_b.y1, rect_a))
		return true;

	return false;
}

bool rectangles_equal(rectangle rect_a, rectangle rect_b) {
	if (rectangle_get_width(rect_a) == rectangle_get_width(rect_b) &&
			rectangle_get_height(rect_a) == rectangle_get_height(rect_b))
		return true;

	return false;
}

bool rectangles_fit_equal(rectangle rect_a, rectangle rect_b) {
	rectangle rect_c;
	rectangle rect_d;

	rect_c.x0 = min(rect_a.x0, rect_a.x1);
	rect_c.y0 = min(rect_a.y0, rect_a.y1);
	rect_c.x1 = max(rect_a.x0, rect_a.x1);
	rect_c.y1 = max(rect_a.y0, rect_a.y1);

	rect_d.x0 = min(rect_b.x0, rect_b.x1);
	rect_d.y0 = min(rect_b.y0, rect_b.y1);
	rect_d.x1 = max(rect_b.x0, rect_b.x1);
	rect_d.y1 = max(rect_b.y0, rect_b.y1);

	if (rect_c.x0 == rect_d.x0 && rect_c.y0 == rect_d.y0
			&& rect_c.x1 == rect_d.x1 && rect_c.y1 == rect_d.y1)
		return true;

	return false;
}

void stretch_rectangle_left(rectangle rect_a, rectangle *rect_b, int16_t stretch) {
	rectangle rect_c;

	rect_c.x0 = min(rect_a.x0, rect_a.x1);
	rect_c.y0 = min(rect_a.y0, rect_a.y1);
	rect_c.x1 = max(rect_a.x0, rect_a.x1);
	rect_c.y1 = max(rect_a.y0, rect_a.y1);

	rect_b->x0 = rect_c.x0 + stretch;
	rect_b->y0 = rect_c.y0;
	rect_b->x1 = rect_c.x1;
	rect_b->y1 = rect_c.y1;
}

void stretch_rectangle_top(rectangle rect_a, rectangle *rect_b, int16_t stretch) {
	rectangle rect_c;

	rect_c.x0 = min(rect_a.x0, rect_a.x1);
	rect_c.y0 = min(rect_a.y0, rect_a.y1);
	rect_c.x1 = max(rect_a.x0, rect_a.x1);
	rect_c.y1 = max(rect_a.y0, rect_a.y1);

	rect_b->x0 = rect_c.x0;
	rect_b->y0 = rect_c.y0 + stretch;
	rect_b->x1 = rect_c.x1;
	rect_b->y1 = rect_c.y1;
}

void stretch_rectangle_right(rectangle rect_a, rectangle *rect_b, int16_t stretch) {
	rectangle rect_c;

	rect_c.x0 = min(rect_a.x0, rect_a.x1);
	rect_c.y0 = min(rect_a.y0, rect_a.y1);
	rect_c.x1 = max(rect_a.x0, rect_a.x1);
	rect_c.y1 = max(rect_a.y0, rect_a.y1);

	rect_b->x0 = rect_c.x0;
	rect_b->y0 = rect_c.y0;
	rect_b->x1 = rect_c.x1 + stretch;
	rect_b->y1 = rect_c.y1;
}

void stretch_rectangle_bottom(rectangle rect_a, rectangle *rect_b, int16_t stretch) {
	rectangle rect_c;

	rect_c.x0 = min(rect_a.x0, rect_a.x1);
	rect_c.y0 = min(rect_a.y0, rect_a.y1);
	rect_c.x1 = max(rect_a.x0, rect_a.x1);
	rect_c.y1 = max(rect_a.y0, rect_a.y1);

	rect_b->x0 = rect_c.x0;
	rect_b->y0 = rect_c.y0;
	rect_b->x1 = rect_c.x1;
	rect_b->y1 = rect_c.y1 + stretch;
}

void rectangle_set_width(rectangle rect_a, uint16_t width, rectangle *rect_b) {
	uint16_t width_a  = rectangle_get_width(rect_a);

	if (width > 0) {
		if (rectangle_get_width(rect_a) > width)
			stretch_rectangle_right(rect_a, rect_b, width_a - width);

		if (rectangle_get_width(rect_a) < width)
			stretch_rectangle_left(rect_a, rect_b, width_a - width);
	}
}

void rectangle_set_height(rectangle rect_a, uint16_t height, rectangle *rect_b) {
	uint16_t height_a  = rectangle_get_height(rect_a);

	if (height > 0) {
		if (rectangle_get_height(rect_a) > height)
			stretch_rectangle_bottom(rect_a, rect_b, height_a - height);

		if (rectangle_get_height(rect_a) < height)
			stretch_rectangle_top(rect_a, rect_b, height_a - height);
	}
}

void align_rectangle_in(rectangle rect_a, rectangle rect_b, uint8_t align, rectangle *rect_c) { // align rect_b to rect_a
	uint16_t a_width, a_height;
	uint16_t b_width, b_height;
	uint16_t rect_a_min_x, rect_a_min_y;

	a_width = rectangle_get_width(rect_a);
	a_height = rectangle_get_height(rect_a);
	b_width = rectangle_get_width(rect_b);
	b_height = rectangle_get_height(rect_b);

	rect_a_min_x = min(rect_a.x0, rect_a.x1);
	rect_a_min_y = min(rect_a.y0, rect_a.y1);

	switch (align) {
	case ALIGN_CENTER:
		rect_c->x0 = rect_a_min_x + a_width / 2 - b_width / 2;
		rect_c->y0 = rect_a_min_y + a_height / 2 - b_height / 2;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_MID_LEFT:
		rect_c->x0 = rect_a_min_x;
		rect_c->y0 = rect_a_min_y + a_height / 2 - b_height / 2;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_MID_TOP:
		rect_c->x0 = rect_a_min_x + a_width / 2 - b_width / 2;
		rect_c->y0 = rect_a_min_y;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_MID_RIGHT:
		rect_c->x0 = rect_a_min_x + a_width - b_width;
		rect_c->y0 = rect_a_min_y + a_height / 2 - b_height / 2;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_MID_BOTTOM:
		rect_c->x0 = rect_a_min_x + a_width / 2 - b_width / 2;
		rect_c->y0 = rect_a_min_y + a_height - b_height;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_LEFT_TOP:
		rect_c->x0 = rect_a_min_x;
		rect_c->y0 = rect_a_min_y;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_LEFT_BOTTOM:
		rect_c->x0 = rect_a_min_x;
		rect_c->y0 = rect_a_min_y + a_height - b_height;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_RIGHT_TOP:
		rect_c->x0 = rect_a_min_x + a_width - b_width;
		rect_c->y0 = rect_a_min_y;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_RIGHT_BOTTOM:
		rect_c->x0 = rect_a_min_x + a_width - b_width;
		rect_c->y0 = rect_a_min_y + a_height - b_height;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	}
}

void align_rectangle_out(rectangle rect_a, rectangle rect_b, uint8_t align, rectangle *rect_c) { // align rect_b to rect_a
	uint16_t a_width, a_height;
	uint16_t b_width, b_height;
	uint16_t rect_a_min_x, rect_a_min_y;

	a_width = rectangle_get_width(rect_a);
	a_height = rectangle_get_height(rect_a);
	b_width = rectangle_get_width(rect_b);
	b_height = rectangle_get_height(rect_b);

	rect_a_min_x = min(rect_a.x0, rect_a.x1);
	rect_a_min_y = min(rect_a.y0, rect_a.y1);

	switch (align) {
	case ALIGN_CENTER:
		rect_c->x0 = rect_a_min_x + a_width / 2 - b_width / 2;
		rect_c->y0 = rect_a_min_y + a_height / 2 - b_height / 2;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_MID_LEFT:
		rect_c->x0 = (rect_a_min_x - b_width) + 1; // contiguity
		rect_c->y0 = rect_a_min_y + a_height / 2 - b_height / 2;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_MID_TOP:
		rect_c->x0 = rect_a_min_x + a_width / 2 - b_width / 2;
		rect_c->y0 = (rect_a_min_y - b_height) + 1; // contiguity
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_MID_RIGHT:
		rect_c->x0 = (rect_a_min_x + a_width) - 1; // contiguity
		rect_c->y0 = rect_a_min_y + a_height / 2 - b_height / 2;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_MID_BOTTOM:
		rect_c->x0 = rect_a_min_x + a_width / 2 - b_width / 2;
		rect_c->y0 = (rect_a_min_y + a_width) - 1; // contiguity
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_LEFT_TOP:
		rect_c->x0 = rect_a_min_x;
		rect_c->y0 = (rect_a_min_y - b_height) + 1; // contiguity
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_LEFT_BOTTOM:
		rect_c->x0 = rect_a_min_x;
		rect_c->y0 = (rect_a_min_y + a_height) - 1; // contiguity
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_RIGHT_TOP:
		rect_c->x0 = rect_a_min_x + a_width - b_width;
		rect_c->y0 = (rect_a_min_y - b_height) + 1; // contiguity
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	case ALIGN_RIGHT_BOTTOM:
		rect_c->x0 = rect_a_min_x + a_width - b_width;
		rect_c->y0 = (rect_a_min_y + a_height) - 1; // contiguity;
		rect_c->x1 = rect_c->x0 + b_width - 1;
		rect_c->y1 = rect_c->y0 + b_height - 1;
		break;
	}
}

void align_rectangle_contiguity(uint8_t align, rectangle *rect_a) {
	switch (align) {
	case ALIGN_MID_LEFT:
		offset_rectangle(*rect_a, rect_a, 1, 0); // offset to compensate left align
		break;
	case ALIGN_MID_TOP:
		offset_rectangle(*rect_a, rect_a, 0, 1); // offset to compensate top align
		break;
	case ALIGN_MID_RIGHT:
		offset_rectangle(*rect_a, rect_a, -1, 0); // offset to compensate right align
		break;
	case ALIGN_MID_BOTTOM:
		offset_rectangle(*rect_a, rect_a, 0, -1); // offset to compensate bottom align
		break;
	}
}

void gui_set_text(rectangle bound, uint8_t align, uint8_t font_id, char text[]) {
	rectangle area, outline; // component work area and outline for text

	scale_rectangle(bound, &area, -1, -1); // scale to component area
	outline_text((uint8_t) strlen(text), font_id, &outline); // calculate text outline
	align_rectangle_in(area, outline, align, &outline); // align text outline
	align_rectangle_contiguity(align, &outline); // correction of align function contiguity
	draw_text(text, font_id, outline.x0, outline.y0); // draw text
}

void gui_clear_text(rectangle bound, uint8_t align, uint8_t font_id, char text[]) {
	rectangle area, outline; // component work area and outline for text

	scale_rectangle(bound, &area, -1, -1); // scale to component area
	outline_text((uint8_t) strlen(text), font_id, &outline); // outline for text that was
	align_rectangle_in(area, outline, align, &outline); // align text outline
	align_rectangle_contiguity(align, &outline); // correction of align function contiguity
	draw_painted_rectangle(outline, GLCD_COMPONENT_BACKGROUND_COLOR); // fill text outline
}

//void titlebar_set_text(gui_titlebar *titlebar, char text[]) {
//	int8_t c = 0; // return value of string compare
//
//	c = strncmp(text, titlebar->text, (uint8_t) sizeof(titlebar->text)); // compare specified text and text that was
//	if (c > 0 || c == 0) { // specified text is longer or equal
//		strncpy(titlebar->text, text, (uint8_t) sizeof(titlebar->text)); // copy specified text to place
//		gui_set_text(titlebar->bound, ALIGN_CENTER, titlebar->font_id, titlebar->text);
//	} else if (c < 0) { // specified text is shorter
//		gui_clear_text(titlebar->bound, ALIGN_CENTER, titlebar->font_id, titlebar->text);
//		strncpy(titlebar->text, text, (uint8_t) sizeof(titlebar->text)); // copy specified text to place
//		gui_set_text(titlebar->bound, ALIGN_CENTER, titlebar->font_id, titlebar->text);
//	}
//}

void titlebar_update_text(gui_titlebar *titlebar) {
	int8_t c = 0; // return value of string compare

	c = strncmp(gui.pager.names[gui.pager.page], titlebar->text, (uint8_t) sizeof(titlebar->text)); // compare specified text and text that was
	if (c > 0 || c == 0) { // specified text is longer or equal
		strncpy(titlebar->text, gui.pager.names[gui.pager.page], (uint8_t) sizeof(titlebar->text)); // copy specified text to place
		gui_set_text(titlebar->bound, ALIGN_CENTER, titlebar->font_id, titlebar->text);
	} else if (c < 0) { // specified text is shorter
		gui_clear_text(titlebar->bound, ALIGN_CENTER, titlebar->font_id, titlebar->text);
		strncpy(titlebar->text, gui.pager.names[gui.pager.page], (uint8_t) sizeof(titlebar->text)); // copy specified text to place
		gui_set_text(titlebar->bound, ALIGN_CENTER, titlebar->font_id, titlebar->text);
	}
}

void  textbox_set_text(gui_textbox *textbox, char text[]) {
	int8_t c = 0; // return value of string compare

	if (textbox->page == gui.pager.page) {
		c = strncmp(text, textbox->text, (uint8_t) sizeof(textbox->text)); // compare specified text and text that was
		if (c > 0 || c == 0) { // specified text is longer or equal
			strncpy(textbox->text, text, (uint8_t) sizeof(textbox->text)); // copy specified text to place
			gui_set_text(textbox->bound, textbox->align, textbox->font_id, textbox->text);
		} else if (c < 0) { // specified text is shorter
			gui_clear_text(textbox->bound, textbox->align, textbox->font_id, textbox->text);
			strncpy(textbox->text, text, (uint8_t) sizeof(textbox->text)); // copy specified text to place
			gui_set_text(textbox->bound, textbox->align, textbox->font_id, textbox->text);
		}
	}
}

void textbox_set_align(gui_textbox *textbox, uint8_t align) {
	rectangle area, outline; // component work area and outline for texts

	if (textbox->align == align) {
		// specified align is already there
	} else {
		scale_rectangle(textbox->bound, &area, -1, -1); // scale component area
		draw_painted_rectangle(area, GLCD_COMPONENT_BACKGROUND_COLOR); // fill entire component area
		outline_text((uint8_t) strlen(textbox->text), textbox->font_id, &outline); // outline
		align_rectangle_in(area, outline, align, &outline); // align text outline
		align_rectangle_contiguity(align, &outline); // correction of align function contiguity
		draw_text(textbox->text, textbox->font_id, outline.x0, outline.y0); // draw text
		textbox->align = align;
	}
}

void readings_set_text(gui_readings *readings, char text[]) {
	int8_t c; // return value of string compare

	if (readings->page == gui.pager.page) {
		c = strncmp(text, readings->text, (uint8_t) sizeof(readings->text)); // compare specified text and text that was
		if (c > 0 || c == 0) { // specified text is longer or equal
			strncpy(readings->text, text, (uint8_t) sizeof(readings->text)); // copy specified text to place
			gui_set_text(readings->bound, ALIGN_MID_LEFT, readings->font_id, readings->text);
		} else if (c < 0) { // specified text is shorter
			gui_clear_text(readings->bound, ALIGN_MID_LEFT, readings->font_id, readings->text);
			strncpy(readings->text, text, (uint8_t) sizeof(readings->text)); // copy specified text to place
			gui_set_text(readings->bound, ALIGN_MID_LEFT, readings->font_id, readings->text);
		}
	}
}

void readings_set_value(gui_readings *readings, char value[]) {
	int8_t c; // return value of string compare

	if (readings->page == gui.pager.page) {
		c = strncmp(value, readings->value, (uint8_t) sizeof(readings->value)); // compare specified value and value that was
		if (c > 0 || c == 0) { // specified text is longer or equal
			strncpy(readings->value, value, (uint8_t) sizeof(readings->value)); // copy specified value to place
			gui_set_text(readings->bound, ALIGN_MID_RIGHT, readings->font_id, readings->value);
		} else if (c < 0) { // specified text is shorter
			gui_clear_text(readings->bound, ALIGN_MID_RIGHT, readings->font_id, readings->value);
			strncpy(readings->value, value, (uint8_t) sizeof(readings->value)); // copy specified value to place
			gui_set_text(readings->bound, ALIGN_MID_RIGHT, readings->font_id, readings->value);
		}
	}
}

void pager_compute_pages(gui_pager *pager) {
	pager_min = 1;
	pager_max = (sizeof(pager->names) / sizeof(char *));
	pager_max -= 1;
}

void pager_update_text(gui_pager *pager) {
	static char s_temp[10] = "Page 0/0";

	s_temp[5] = pager->page + '0', s_temp[7] = pager_max + '0'; // ASCII code of integer

	gui_clear_text(pager->bound, ALIGN_CENTER, pager->font_id, s_temp);
	gui_set_text(pager->bound, ALIGN_CENTER, pager->font_id, s_temp);
}

void gui_draw_component_image(void *gui_component) {
	rectangle area; // component work area
	uint8_t type;

	type = *(uint8_t *) gui_component;
	switch(type) {
	case GUI_TITLEBAR:
		gui_titlebar *titlebar = (gui_titlebar *) gui_component;
		draw_plain_rectangle(titlebar->bound, GLCD_BORDER_COLOR); // draw border
		scale_rectangle(titlebar->bound, &area, -1, -1); // scale component area
		draw_painted_rectangle(area, GLCD_COMPONENT_BACKGROUND_COLOR); // fill component area
		gui_set_text(titlebar->bound, ALIGN_CENTER, titlebar->font_id, titlebar->text);
		break;
	case GUI_TEXTBOX:
		gui_textbox *textbox = (gui_textbox *) gui_component;
		draw_plain_rectangle(textbox->bound, GLCD_BORDER_COLOR); // draw border
		scale_rectangle(textbox->bound, &area, -1, -1); // scale component area
		draw_painted_rectangle(area, GLCD_COMPONENT_BACKGROUND_COLOR); // fill entire component area
		gui_set_text(textbox->bound, textbox->align, textbox->font_id, textbox->text);
		break;
	case GUI_READINGS:
		gui_readings *readings = (gui_readings *) gui_component;
		draw_plain_rectangle(readings->bound, GLCD_BORDER_COLOR); // draw border
		scale_rectangle(readings->bound, &area, -1, -1); // scale component area
		draw_painted_rectangle(area, GLCD_COMPONENT_BACKGROUND_COLOR); // fill entire component area
		gui_set_text(readings->bound, ALIGN_MID_LEFT, readings->font_id, readings->text);
		gui_set_text(readings->bound, ALIGN_MID_RIGHT, readings->font_id, readings->value);
		break;
	case GUI_PAGER:
		gui_pager *pager = (gui_pager *) gui_component;
		scale_rectangle(pager->bound, &pager->bound, -50, 0); // scale to rectangle with text
		draw_plain_rectangle(pager->bound, GLCD_BORDER_COLOR); // draw border
		scale_rectangle(pager->bound, &area, -1, -1); // scale component area
		draw_painted_rectangle(area, GLCD_COMPONENT_BACKGROUND_COLOR); // fill area

		stretch_rectangle_right(pager->bound, &area, -((50 + rectangle_get_width(pager->bound)) - 1));
		draw_plain_rectangle(area, GLCD_BORDER_COLOR); // draw border
		scale_rectangle(area, &area, -1, -1); // scale component area
		draw_painted_rectangle(area, GLCD_COMPONENT_BACKGROUND_COLOR); // fill area
		gui_set_text(area, ALIGN_CENTER, pager->font_id, "\x11");

		stretch_rectangle_left(pager->bound, &area, +((50 + rectangle_get_width(pager->bound)) - 1));
		draw_plain_rectangle(area, GLCD_BORDER_COLOR); // draw border
		scale_rectangle(area, &area, -1, -1); // scale component area
		draw_painted_rectangle(area, GLCD_COMPONENT_BACKGROUND_COLOR); // fill area
		gui_set_text(area, ALIGN_CENTER, pager->font_id, "\x10");
		break;
	}
}

void gui_compute_components(rectangle glcd_rect) {
	rectangle outline;

	draw_painted_rectangle(glcd_rect, GLCD_BACKGROUND_COLOR); // background
	draw_plain_rectangle(glcd_rect, GLCD_BORDER_COLOR); // screen border

	outline_text((uint8_t) sizeof(gui.titlebar.text), gui.titlebar.font_id, &outline); // titlebar
	rectangle_set_width(gui.titlebar.bound, rectangle_get_width(glcd_rect), &gui.titlebar.bound);
	rectangle_set_height(gui.titlebar.bound, 2 + 2 + rectangle_get_height(outline), &gui.titlebar.bound);
	align_rectangle_in(glcd_rect, gui.titlebar.bound, ALIGN_MID_TOP, &gui.titlebar.bound);

	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) { // labels
		outline_text((uint8_t) sizeof(gui.textboxes[i].text), gui.textboxes[i].font_id, &outline);
		rectangle_set_width(gui.textboxes[i].bound, 2 + 2 + rectangle_get_width(outline), &gui.textboxes[i].bound);
		rectangle_set_height(gui.textboxes[i].bound, 2 + 2 + rectangle_get_height(outline), &gui.textboxes[i].bound);
	}

	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_readings)); i++) { // readings
		outline_text(((uint8_t) sizeof(gui.readings[i].text) + (uint8_t) sizeof(gui.readings[i].value)), gui.readings[i].font_id, &outline);
		rectangle_set_width(gui.readings[i].bound, 2 + 2 + rectangle_get_width(outline), &gui.readings[i].bound);
		rectangle_set_height(gui.readings[i].bound, 2 + 2 + rectangle_get_height(outline), &gui.readings[i].bound);
	}

	outline_text((uint8_t) sizeof(gui.pager.names), gui.pager.font_id, &outline); // pager
	rectangle_set_width(gui.pager.bound, rectangle_get_width(glcd_rect), &gui.pager.bound);
	rectangle_set_height(gui.pager.bound, 2 + 2 + rectangle_get_height(outline), &gui.pager.bound);
	align_rectangle_in(glcd_rect, gui.pager.bound, ALIGN_MID_BOTTOM, &gui.pager.bound);
}

void gui_layout_components(rectangle glcd_rect) {
	uint8_t hflow, vflow, margin = 3;
	uint16_t block_width, block_height;
	rectangle components_area, rect_block, rect_align;

	between_rectangle(gui.titlebar.bound, gui.pager.bound, &components_area); // create components area
	scale_rectangle(components_area, &components_area, -5, -5); // scale components area

	memset(&rect_block, 0, sizeof(rect_block)); // TODO imprecision in rectangle_set_width rectangle_set_height
	block_width = 0, block_height = 0, hflow = 0, vflow = 0;
	stretch_rectangle_bottom(components_area, &rect_align, -(rectangle_get_height(components_area)/2));
	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) {
		if (gui.textboxes[i].page  == gui.pager.page)
			if (block_width < rectangle_get_width(gui.textboxes[i].bound))
				block_width = rectangle_get_width(gui.textboxes[i].bound);
	}
	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) {
		if (gui.textboxes[i].page  == gui.pager.page)
			block_height += rectangle_get_height(gui.textboxes[i].bound) + margin;
	}

	rectangle_set_width(rect_block, block_width, &rect_block);
	rectangle_set_height(rect_block, block_height -3, &rect_block);
	align_rectangle_in(rect_align, rect_block, ALIGN_CENTER, &rect_block);

	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) { // layout labels
		if (gui.textboxes[i].page  == gui.pager.page) {
			align_rectangle_in(rect_block, gui.textboxes[i].bound, ALIGN_MID_TOP, &gui.textboxes[i].bound);
			offset_rectangle(gui.textboxes[i].bound, &gui.textboxes[i].bound, vflow, hflow);
			hflow += rectangle_get_height(gui.textboxes[i].bound) + margin;
		}
	}

	memset(&rect_block, 0, sizeof(rect_block)); // TODO imprecision in rectangle_set_width rectangle_set_height
	block_width = 0, block_height = 0, hflow = 0, vflow = 0;
	stretch_rectangle_top(components_area, &rect_align, (rectangle_get_height(components_area)/2));
	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_readings)); i++) {
		if (gui.readings[i].page  == gui.pager.page)
			if (block_width < rectangle_get_width(gui.readings[i].bound))
				block_width = rectangle_get_width(gui.readings[i].bound);
	}
	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_readings)); i++) {
		if (gui.readings[i].page  == gui.pager.page)
			block_height += rectangle_get_height(gui.readings[i].bound) + margin;
	}


	rectangle_set_width(rect_block, block_width, &rect_block);
	rectangle_set_height(rect_block, block_height - 3, &rect_block);
	align_rectangle_in(rect_align, rect_block, ALIGN_CENTER, &rect_block);

	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_readings)); i++) { // layout readings
		if (gui.readings[i].page == gui.pager.page) {
			align_rectangle_in(rect_block, gui.readings[i].bound, ALIGN_MID_TOP, &gui.readings[i].bound);
			offset_rectangle(gui.readings[i].bound, &gui.readings[i].bound, vflow, hflow);
			hflow += rectangle_get_height(gui.readings[i].bound) + margin;
		}
	}
}

void gui_activate_components(rectangle glcd_rect) {
	gui_draw_component_image(&gui.titlebar);
	titlebar_update_text(&gui.titlebar);
	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) {
		if (gui.textboxes[i].page == gui.pager.page) // check page
			gui_draw_component_image(&gui.textboxes[i]);
	}

	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_readings)); i++) {
		if (gui.readings[i].page == gui.pager.page) // check page
				gui_draw_component_image(&gui.readings[i]);
	}
	gui_draw_component_image(&gui.pager);
	pager_update_text(&gui.pager);
	pager_compute_pages(&gui.pager);
}

void glcd_demo(void) {
	float clocks;
	char s_temp[256];

	if (glcd_play) {
		clock_t begin = clock();

		gui_compute_components(glcd_rect);
		gui_layout_components(glcd_rect);
		gui_activate_components(glcd_rect);

		readings_set_text(&gui.readings[0], "mouse_x:");
		sprintf(s_temp, "%d", mouse_x);
		readings_set_value(&gui.readings[0], s_temp);

		readings_set_text(&gui.readings[1], "mouse_y:");
		sprintf(s_temp, "%d", mouse_y);
		readings_set_value(&gui.readings[1], s_temp);

		clock_t end = clock();

		clocks = (float) (end - begin) / CLOCKS_PER_SEC;
		sprintf(s_temp, "%.3f", clocks);
		readings_set_value(&gui.readings[2], s_temp);
	}
}

void draw_ortho_frame_info(void) {
	char string_info[256];

	sprintf(string_info, "ortho_grid_step: %d", ortho_grid_step);
	draw_text(string_info, GLCD_FONT_8X8, 20, 20);
	sprintf(string_info, "frame_scale_x: %.3f", frame_scale_x);
	draw_text(string_info, GLCD_FONT_8X8, 20, 30);
	sprintf(string_info, "frame_scale_y: %.3f", frame_scale_y);
	draw_text(string_info, GLCD_FONT_8X8, 20, 40);

	sprintf(string_info, "mouse_x: %d", mouse_x);
	draw_text(string_info, GLCD_FONT_8X8, 20, 50);
	sprintf(string_info, "mouse_y: %d", mouse_y);
	draw_text(string_info, GLCD_FONT_8X8, 20, 60);
}

void draw_ortho_gridlines(int grid_horizontal, int grid_vertical, int grid_step) {
	float radius_exterior = 200.0f;
//	float radius_interior = 150.0f;
	float axis_extension = 30.0f;

//	float grid_circle[24][2];
//	float grid_circle_sectors = sizeof(grid_circle)/sizeof(float)/2;

//	int circular_index = 0;
//	int loop_counter = grid_circle_sectors;

//	if(verify_grid_rectangular(grid_horizontal, grid_vertical, grid_step)) {
//		glColor4f(0.1f, 0.1f, 0.1f, 0.9f); // grid lines parallel to x-axis
//		for (int i = -grid_vertical; i <= grid_vertical; i += grid_step) {
//			glBegin(GL_LINES);
//			glVertex2f((float) -grid_horizontal, (float) i);
//			glVertex2f((float) grid_horizontal, (float) i);
//			glEnd();
//		}
//
//		glColor4f(0.1f, 0.1f, 0.1f, 0.9f); // grid lines parallel to y-axis
//		for (int i = -grid_horizontal; i <= grid_horizontal; i += grid_step) {
//			glBegin(GL_LINES);
//			glVertex2f((float) i, (float) -grid_vertical);
//			glVertex2f((float) i, (float) grid_vertical);
//			glEnd();
//		}
//	}

//	for (int i = 0; i < grid_circle_sectors; i++) { // compute circle coordinates
//		grid_circle[i][0] = cos(M_PI * 2.0f / grid_circle_sectors * i); // x-coordinate
//		grid_circle[i][1] = sin(M_PI * 2.0f / grid_circle_sectors * i); // y-coordinate
//	}
//
//	glColor4f(0.0f, 0.8f, 1.0f, 0.5f); // exterior circle
//	glBegin(GL_LINE_LOOP);
//	for (int i = 0; i < grid_circle_sectors; i++) {
//		glVertex2f(grid_circle[i][0] * radius_exterior, grid_circle[i][1] * radius_exterior);
//	}
//	glEnd();
//
//	glColor4f(0.0f, 0.8f, 1.0f, 0.5f); // interior circle
//	glBegin(GL_LINE_LOOP);
//	for (int i = 0; i < grid_circle_sectors; i++) {
//		glVertex2f(grid_circle[i][0] * radius_interior, grid_circle[i][1] * radius_interior);
//	}
//	glEnd();
//
//	glColor4f(0.0f, 0.8f, 1.0f, 0.5f); // clock lines
//	glBegin(GL_LINES);
//	for (int i = 0; i < grid_circle_sectors; i++) {
//		glVertex2f(grid_circle[i][0] * radius_exterior, grid_circle[i][1] * radius_exterior);
//		glVertex2f(grid_circle[i][0] * radius_interior, grid_circle[i][1] * radius_interior);
//	}
//	glEnd();

//	glColor4f(0.0f, 0.8f, 1.0f, 0.2f); // polygonal coating
//	glBegin(GL_QUAD_STRIP);
//	for (int i = 0; i < loop_counter + 1; i++) {
//		glVertex2f(grid_circle[circular_index][0] * radius_interior, grid_circle[circular_index][1] * radius_interior);
//		glVertex2f(grid_circle[circular_index][0] * radius_exterior, grid_circle[circular_index][1] * radius_exterior);
//		circular_index = ((circular_index + 1) % loop_counter);
//	}
//	glEnd();

	glBegin(GL_LINES); // coordinate axis lines
	glColor4f(0.0f, 0.0f, 1.0f, 0.5f); // x-axis
	glVertex2f(-radius_exterior - axis_extension, 0.0f);
	glVertex2f(radius_exterior + axis_extension, 0.0f);
	glColor4f(0.0f, 1.0f, 0.0f, 0.5f); // y-axis
	glVertex2f(0.0f, -radius_exterior - axis_extension);
	glVertex2f(0.0f, radius_exterior + axis_extension);
	glEnd();

	glBegin(GL_POINTS); // coordinate system origin
	glColor3f(1.0f, 1.0f, 1.0f);
	glVertex2f(0.0f, 0.0f);
	glEnd();
}

void render_ortho_frame() {
	float red = RGB_1_0(RGB565_R(EVE_ONLINE_WINDOW_BACKGROUND));
	float green = RGB_1_0(RGB565_G(EVE_ONLINE_WINDOW_BACKGROUND));
	float blue = RGB_1_0(RGB565_B(EVE_ONLINE_WINDOW_BACKGROUND));

	glClearColor(red, green, blue, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	switch (axis_direction) {
	case (X_AXIS_RIGHT | Y_AXIS_DOWN):
		gluOrtho2D( // y-down, x-right
				0.0f - ((float) frame_width * frame_scale_x) / 2 + frame_offset_x, // left
				((float) frame_width * frame_scale_x) / 2 + frame_offset_x, // right
				((float) frame_height * frame_scale_x) / 2 + frame_offset_y, // bottom
				0.0f - ((float) frame_height * frame_scale_x) / 2 + frame_offset_y); // top
		break;
	case (X_AXIS_RIGHT | Y_AXIS_UP):
		gluOrtho2D( // y-up, x-right
				0.0f - ((float) frame_width * frame_scale_x) / 2 + frame_offset_x, // left
				((float) frame_width * frame_scale_x) / 2 + frame_offset_x, // right
				0.0f - ((float) frame_height * frame_scale_x) / 2 + frame_offset_y, // bottom
				((float) frame_height * frame_scale_x) / 2 + frame_offset_y); // top
		break;
	case (X_AXIS_LEFT | Y_AXIS_DOWN):
		gluOrtho2D( // y-down, x-left
				((float) frame_width * frame_scale_x) / 2 + frame_offset_x, // left
				0.0f - ((float) frame_width * frame_scale_x) / 2 + frame_offset_x, // right
				((float) frame_height * frame_scale_x) / 2 + frame_offset_y, // bottom
				0.0f - ((float) frame_height * frame_scale_x) / 2 + frame_offset_y); // top
		break;
	case (X_AXIS_LEFT | Y_AXIS_UP):
		gluOrtho2D( // y-up, x-left
				((float) frame_width * frame_scale_x) / 2 + frame_offset_x, // left
				0.0f - ((float) frame_width * frame_scale_x) / 2 + frame_offset_x, // right
				0.0f - ((float) frame_height * frame_scale_x) / 2 + frame_offset_y, // bottom
				((float) frame_height * frame_scale_x) / 2 + frame_offset_y); // top
		break;
	}

	glMatrixMode(GL_MODELVIEW);
	draw_ortho_gridlines(320, 240, ortho_grid_step);
	glcd_demo();
//	draw_ortho_frame_info();
	if (frame_alter_action == FRAME_ALTER_ROTATION)
		draw_circle(mouse_x, mouse_y, 2, ILI9341_WHITE);
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	// wasd
	if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		ws_key++;
	}
	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		ad_key--;
	}
	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		ws_key--;
	}
	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		ad_key++;
	}
	// qe
	if (key == GLFW_KEY_Q && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		qe_key--;
	}
	if (key == GLFW_KEY_E && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		qe_key++;
	}

	// arrows
	if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		h_arrows_key--;
	}
	if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		h_arrows_key++;
	}
	if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		v_arrows_key--;
	}
	if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		v_arrows_key++;
	}

	// pages
	if (key == GLFW_KEY_PAGE_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		page_up_down_key++;
	}
	if (key == GLFW_KEY_PAGE_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		page_up_down_key--;
	}

	// 1234567890
	if (key == GLFW_KEY_1 && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		frame_scale_x = 1.0f;
		frame_scale_y = 1.0f;
	}
	if (key == GLFW_KEY_2 && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		frame_offset_x = 0.0f;
		frame_offset_y = 0.0f;
	}
	if (key == GLFW_KEY_3 && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		ortho_grid_step--;
	}
	if (key == GLFW_KEY_4 && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		ortho_grid_step++;
	}
	if (key == GLFW_KEY_5 && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		//
	}
	if (key == GLFW_KEY_6 && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		//
	}
	if (key == GLFW_KEY_7 && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		axis_direction = X_AXIS_LEFT | Y_AXIS_UP;
	}
	if (key == GLFW_KEY_8 && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		axis_direction = X_AXIS_LEFT | Y_AXIS_DOWN;
	}
	if (key == GLFW_KEY_9 && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		axis_direction = X_AXIS_RIGHT | Y_AXIS_UP;
	}
	if (key == GLFW_KEY_0 && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		axis_direction = X_AXIS_RIGHT | Y_AXIS_DOWN;
	}

	if (mods == GLFW_MOD_SHIFT) {
		if (key == GLFW_KEY_COMMA && (action == GLFW_REPEAT || action == GLFW_PRESS)) { // ','
			comma_period_key--;

			gui.pager.page--;
			if (gui.pager.page < pager_min)
				gui.pager.page = pager_max;
		}
		if (key == GLFW_KEY_PERIOD && (action == GLFW_REPEAT || action == GLFW_PRESS)) { // '.'
			comma_period_key++;

			gui.pager.page++;
			if (gui.pager.page > pager_max)
				gui.pager.page = pager_min;
		}
	}
}

void adjust_mouse_coordinates(int16_t *mouse_x, int16_t *mouse_y) {
	if (*mouse_x < (frame_width / 2)) {
		*mouse_x = -(frame_width / 2 - *mouse_x);
	} else {
		*mouse_x = (*mouse_x - frame_width / 2);
	}

	if (*mouse_y < (frame_height / 2)) {
		*mouse_y = -(frame_height / 2 - *mouse_y);
	} else {
		*mouse_y = +(*mouse_y - frame_height / 2);
	}
}

void cursor_position_callback(GLFWwindow* window, double x, double y) {
	mouse_x = x, mouse_y = y;

	mouse_dx = prev_mouse_x - mouse_x;
	mouse_dy = prev_mouse_y - mouse_y;
	prev_mouse_x = mouse_x;
	prev_mouse_y = mouse_y;

	adjust_mouse_coordinates(&mouse_x, &mouse_y);

	if (frame_alter_action == FRAME_ALTER_ROTATION) {

	}

	if (frame_alter_action == FRAME_ALTER_DISTANCE) {
		frame_scale_x += (float) (mouse_dx + mouse_dy) / 1000;
		frame_scale_y += (float) (mouse_dx + mouse_dy) / 1000;
	}

	if (frame_alter_action == FRAME_ALTER_POSITION) {
		frame_offset_x += (float) mouse_dx;
		frame_offset_y += (float) mouse_dy;
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		frame_alter_action = FRAME_ALTER_ROTATION;

		if (rectangle_contains_point(mouse_x, mouse_y, gui.textboxes[0].bound)) {
			textbox_set_text(&gui.textboxes[0], "YES!");
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		frame_alter_action = FRAME_NO_ALTER;

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		frame_alter_action = FRAME_ALTER_DISTANCE;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		frame_alter_action = FRAME_NO_ALTER;

	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
		frame_alter_action = FRAME_ALTER_POSITION;
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
		frame_alter_action = FRAME_NO_ALTER;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	frame_width = width;
	frame_height = height;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

//	gluOrtho2D(
//			0.0f - (float) frame_width / 2 - 1, // left
//			(float) frame_width / 2, // right
//			0.0f - (float) frame_height / 2 - 1, // bottom
//			(float) frame_height / 2 // top
//	);

//	gluOrtho2D(
//			0.0f - (float) frame_width / 2 - 1, // left
//			(float) frame_width / 2, // right
//			(float) frame_height / 2 - 1, // bottom
//			0.0f - (float) frame_height / 2 // top
//	);

	glMatrixMode(GL_MODELVIEW);
}

void window_size_callback(GLFWwindow* window, int width, int height) {
	framebuffer_size_callback(window, width, height);
}

void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_SAMPLES, 0);

	window = glfwCreateWindow(frame_width, frame_height, "OpenGL Application", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetKeyCallback(window, keyboard_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // 1, 2, 3

	glDisable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glDisable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glDisable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);                                // enables transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // best for transparency points and lines
	//glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE);      // best for polygons
	glDisable(GL_CULL_FACE);

	glfwGetFramebufferSize(window, &frame_width, &frame_height);
	framebuffer_size_callback(window, frame_width, frame_height);

//	gui_compute_coordinates(glcd_rect);
//	gui_layout_components(glcd_rect);
//	gui_draw_components(glcd_rect);

	while (!glfwWindowShouldClose(window)) {
		render_ortho_frame();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}
