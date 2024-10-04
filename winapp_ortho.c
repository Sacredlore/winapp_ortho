/*
 ============================================================================
 Name        : winapp_glfw_ortho.c
 Author      : Sacredlore
 Version     :
 Created on  :
 Description : OpenGL Application
 ============================================================================
 */

#include "winapp_ortho.h"
#include "winapp_bitmaps.h"

const char szApplicationName[] = "WinAPI OpenGL Application";
const char szWndClassName[] = "WinAPI_OpenGL_WindowClass";
const char szStatusbarClassName[] = "WinAPI_OpenGL_StatusbarClass";
const char szWndBitmapClassName[] = "WinAPI_LoadBitmap_WindowClass";

HWND hMainWindow, hBitmapWindow, hWndEdit, hStatusBar;
HMENU hMainMenu, hFileMenu, hViewMenu;
HFONT hFont;
HGLRC hglrc;
int iStatusWidths[] = { 100, 200, -1 };
OPENFILENAME ofn; // common dialog box structure
TCHAR szFile[260] = { 0 }; // if using TCHAR macros


unsigned char *bmp_bytes;
uint8_t palette[16 * 4];

char *lord_of_light =
		"It is said that fifty-three years after his liberation he returned from \
the Golden Cloud, to take up once again the gauntlet of Heaven, to oppose \
the Order of Life and the gods who ordained it so. His followers had prayed \
for his return, though their prayers were sin. Prayer should not trouble one \
who has gone on to Nirvana, no matter what the circumstances of his going. \
The wearers of the saffron robe prayed, however, that He of the Sword, \
Manjusri, should come again among them, The Boddhisatva is said to have \
heard...\n\
\tHe whose desires have been throttled,\n\
\twho is independent of root,\n\
\twhose pasture is emptiness\n\
\tsignless and free\n\
\this path is as unknowable\n\
\tas that of birds across the heavens.";

int fontListBase = 1000;
int axis_direction = X_AXIS_RIGHT | Y_AXIS_DOWN;
int ortho_frame_alter = FRAME_NO_ALTER;

GLsizei frame_width, frame_height;
GLdouble frame_ox = 0.0f, frame_oy = 0.0f; // frame offset
GLdouble frame_sx = 1.0f, frame_sy = 1.0f; // frame scale

int mouse_x = 0, mouse_y = 0; // mouse current position
int mouse_px = 0, mouse_py = 0; // mouse previous position
int mouse_dx = 0, mouse_dy = 0; // mouse distance
int mouse_cx = 0, mouse_cy = 0; // mouse click position

int ws_key = 0, ad_key = 0, qe_key = 0;
int h_arrows_key = 0, v_arrows_key = 0, pages_key = 0;
int rf_key = 0, comma_period_key = 0;

uint8_t gui_frame_counter = 0;
bool gui_recompute = true;
bool gui_relayout = true;
bool gui_rerender = true;
int16_t glcd_width = GLCD_WIDTH, glcd_height = GLCD_HEIGHT;
rectangle glcd_rect = { 0, 0, GLCD_WIDTH - 1, GLCD_HEIGHT - 1 };
rectangle glcd_corner_handles[4];
rectangle glcd_mid_handles[4];
bool glcd_led1 = false; // glcd highlights
bool glcd_led2 = false;
bool glcd_led3 = false;

bool gui_drag_component = false;
rectangle *rect_drag, rect_drag_temp;
int drag_shift_x = 0, drag_shift_y = 0;
rectangle rect_temp;

char *pager_page_names[PAGER_PAGES_MAX] = { "PAGE0", "PAGE1", "PAGE2", "PAGE3" };

struct __gui {
	gui_textbox titlebar;
	gui_textbox textboxes[5];
	gui_textbox readings[5];
	gui_writer writer;
	gui_painter painter;
	gui_pager pager;
} gui =
{
	{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_CENTER, "TITLEBAR1", "VT"},
	{
		{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_MID_LEFT, "LABEL1", "V1" },
		{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_MID_LEFT, "LABEL2", "V2" },
		{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_MID_LEFT, "LABEL3", "V3" },
		{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_MID_LEFT, "LABEL4", "V4" },
		{ GUI_TEXTBOX, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_MID_LEFT, "LABEL5", "V5" }
	},
	{
		{ GUI_READINGS, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_MID_LEFT, "READING1:", "000.000"},
		{ GUI_READINGS, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_MID_LEFT, "READING2:", "000.000"},
		{ GUI_READINGS, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_MID_LEFT, "READING3:", "000.000"},
		{ GUI_READINGS, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_MID_LEFT, "READING4:", "000.000"},
		{ GUI_READINGS, { 0, 0, 0, 0 }, 2, GLCD_FONT_8X8, ALIGN_MID_LEFT, "READING5:", "000.000"}
	},
	{ GUI_WRITER, { 0, 0, 0, 0 }, 1, GLCD_FONT_8X8, ALIGN_LEFT_TOP, WRITER_WIDTH, WRITER_HEIGHT, 1, 1, "" },
	{ GUI_PAINTER, { 0, 0, 0, 0 }, 3, 280, 80, apocrypha_bmp },
	{ GUI_PAGER, { 0, 0, 0, 0 }, 3, GLCD_FONT_8X8, 0, PAGER_PAGES_MAX, pager_page_names } // TODO
};

struct {
	rectangle *bound;
	void (*cmd_event)();
} events[GUI_EVENTS_MAX];

/*
 * Tries to set pixel how in ILI9341.
 */
void glcd_set_pixel(int16_t x0, int16_t y0, uint16_t color) {
//	uint8_t r, g, b;

//	r = RGB565_R(color);
//	g = RGB565_G(color);
//	b = RGB565_B(color);

	glBegin(GL_POINTS);
//	glColor4f(RGB_1_0(r), RGB_1_0(g), RGB_1_0(g), 1.0f);
//	glColor3f(RGB_1_0(r), RGB_1_0(g), RGB_1_0(b));
	glColor3ub(RGB565_R(color), RGB565_G(color), RGB565_B(color)); // TODO transparency on/off
	//glVertex2i(x0, y0);
	glVertex2s(x0, y0);
	glEnd();
}

/*
 * Draws horizontal line in positive direction.
 */
void glcd_draw_hline(int16_t x0, int16_t y0, int16_t length, uint16_t color) {
	for (int16_t i = 0; i < length; i++) {
		glcd_set_pixel(x0 + i, y0, color);
	}
}

/*
 * Draws vertical line in positive direction.
 */
void glcd_draw_vline(int16_t x0, int16_t y0, int16_t length, uint16_t color) {
	for (int16_t i = 0; i < length; i++) {
		glcd_set_pixel(x0, y0 + i, color);
	}
}

void glcd_clear(uint16_t color) {
	draw_painted_rectangle(glcd_rect, GLCD_BACKGROUND_COLOR);
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

void draw_character8x16t(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 16; y++) {
		w = font8x16t[c][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(x + x0, y + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
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
				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
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
				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
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
				glcd_set_pixel(x + x0, y + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
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

void draw_character8x8cp437(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	c += ws_key;
	for (y = 0; y < 8; y++) {
		w = font8x8cp437[c][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
//				glcd_set_pixel(x + x0, y + y0, GLCD_NORMAL_FONT_COLOR);
				glcd_set_pixel(y + x0, x + y0, GLCD_NORMAL_FONT_COLOR);
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

void draw_character3x5w(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 3; y++) {
		w = font3x5wendy[c - 0x20][y];
		for (x = 0; x < 5; x++) {
			if (w & 1) { // character
				glcd_set_pixel(y + x0, x + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
				glcd_set_pixel(y + x0, x + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
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
				glcd_set_pixel(y + x0, x + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character6x8z(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 6; y++) {
		w = font6x8zpix[c - 0x20][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(y + x0, x + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
				glcd_set_pixel(y + x0, x + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character4x8t(uint8_t c, int16_t x0, int16_t y0) {
	uint8_t x, y, w;

	for (y = 0; y < 4; y++) {
		w = font5x8tama[c - 0x20][y];
		for (x = 0; x < 8; x++) {
			if (w & 1) { // character
				glcd_set_pixel(y + x0, x + y0, GLCD_NORMAL_FONT_COLOR);
			} else { // background
				glcd_set_pixel(y + x0, x + y0, GLCD_COMPONENT_BACKGROUND_COLOR);
			}
			w = w >> 1;
		}
	}
}

void draw_character3x5p(uint8_t c, int16_t x0, int16_t y0) {
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

void draw_text(char text[], uint8_t length, uint8_t font_id, point p) {
	switch (font_id) {
	case GLCD_FONT_8X16:
		for (int i = 0; i < length; i++) {
			draw_character8x16(text[i], p.x0 + i * 8, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X16T:
		for (int i = 0; i < length; i++) {
			draw_character8x16t(text[i], p.x0 + i * 8, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X14:
		for (int i = 0; i < length; i++) {
			draw_character8x14(text[i], p.x0 + i * 8, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X12:
		for (int i = 0; i < length; i++) {
			draw_character8x12(text[i], p.x0 + i * 8, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X8T:
		for (int i = 0; i < length; i++) {
			draw_character8x8t(text[i], p.x0 + i * 8, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X8:
		for (int i = 0; i < length; i++) {
			draw_character8x8(text[i], p.x0 + i * 8, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_8X8CP437:
		for (int i = 0; i < length; i++) {
			draw_character8x8cp437(text[i], p.x0 + i * 8, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_5X7S:
		for (int i = 0; i < length; i++) {
			draw_character5x7s(text[i], p.x0 + i * 6, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_3X5W:
		for (int i = 0; i < length; i++) {
			draw_character3x5w(text[i], p.x0 + i * 4, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_5X7M:
		for (int i = 0; i < length; i++) {
			draw_character5x7m(text[i], p.x0 + i * 6, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_6X8Z:
		for (int i = 0; i < length; i++) {
			draw_character6x8z(text[i], p.x0 + i * 7, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_4X8T:
		for (int i = 0; i < length; i++) {
			draw_character4x8t(text[i], p.x0 + i * 5, p.y0); //  x + i * spacing
		}
		break;
	case GLCD_FONT_3X6P:
		for (int i = 0; i < length; i++) {
			draw_character3x5p(text[i], p.x0 + i * 4, p.y0); //  x + i * spacing
		}
		break;
	}
}

#define draw_char(c, font_id, p) draw_text(c, 1, font_id, p)

rectangle outline_text(uint8_t length, uint8_t font_id, rectangle *outline) {
	rectangle char_outline;

	memset(outline, 0, sizeof(rectangle));
	memset(&char_outline, 0, sizeof(rectangle));
	switch (font_id) {
	case GLCD_FONT_8X16T:
		char_outline.x1 = 8 - 2;                    // spacing
		char_outline.y1 = 16 - 1;                   // height
		outline->x1 = length * char_outline.x1 - 2; // length * spacing - compensation
		outline->y1 = char_outline.y1;
		break;
	case GLCD_FONT_8X16:
		outline->x1 = length * 8 - 2; // length * spacing - compensation
		outline->y1 = 16 - 1; // height
		break;
	case GLCD_FONT_8X14:
		outline->x1 = length * 8 - 2; // length * spacing - compensation
		outline->y1 = 14 - 1; // height
		break;
	case GLCD_FONT_8X12:
		outline->x1 = length * 8 - 2; // length * spacing - compensation
		outline->y1 = 12 - 1; // height
		break;
	case GLCD_FONT_8X8T:
		char_outline.x1 = 8;                        // spacing
		char_outline.y1 = 8 - 1;                    // height
		outline->x1 = length * char_outline.x1 - 2; // length * spacing - compensation
		outline->y1 = char_outline.y1;
		break;
	case GLCD_FONT_8X8:
		char_outline.x1 = 8 - 1;      // width - compensation
		char_outline.y1 = 8 - 1;      // height - compensation
		outline->x1 = length * 8 - 2; // length * spacing - compensation
		outline->y1 = 8 - 1;          // height - compensation
		break;
	case GLCD_FONT_8X8CP437:
		char_outline.x1 = 8;                        // spacing
		char_outline.y1 = 8 - 1;                    // height
		outline->x1 = length * char_outline.x1 - 2; // length * spacing - compensation
		outline->y1 = char_outline.y1;
		break;
	case GLCD_FONT_5X7S:
		char_outline.x1 = 6;                        // spacing
		char_outline.y1 = 7 - 1;
		;                   // height
		outline->x1 = length * char_outline.x1 - 2; // length * spacing - compensation
		outline->y1 = char_outline.y1;
		break;
	case GLCD_FONT_3X5W:
		outline->x1 = length * 4 - 2; // length * spacing - compensation
		outline->y1 = 5 - 1; // height
		break;
	case GLCD_FONT_5X7M:
		outline->x1 = length * 6 - 2; // length * spacing - compensation
		outline->y1 = 7 - 1; // height
		break;
	case GLCD_FONT_6X8Z:
		outline->x1 = length * 7 - 2; // length * spacing - compensation
		outline->y1 = 8 - 1; // height
		break;
	case GLCD_FONT_4X8T:
		outline->x1 = length * 5 - 2; // length * spacing - compensation
		outline->y1 = 8 - 1; // height
		break;
	case GLCD_FONT_3X6P:
		outline->x1 = length * 4 - 2; // length * spacing - compensation
		outline->y1 = 6 - 1; // height
		break;
	}

	return char_outline;
}

#define outline_char(font_id, outline) outline_text(1, font_id, outline)

void draw_plain_rectangle(rectangle rect, uint16_t color) {
	glcd_draw_vline(rect.x0, rect.y0, rect.y1 - rect.y0, color); // left
	glcd_draw_hline(rect.x0, rect.y0, rect.x1 - rect.x0, color); // top
	glcd_draw_vline(rect.x1, rect.y0, rect.y1 - rect.y0 + 1, color); // right
	glcd_draw_hline(rect.x0, rect.y1, rect.x1 - rect.x0, color); // bottom
}

void draw_painted_rectangle(rectangle rect, uint16_t color) {
	uint16_t width, height;

	width = rectangle_get_width(rect);
	height = rectangle_get_height(rect);

	if (width > height) // optimize fill direction
		while (height--)
			glcd_draw_hline(min(rect.x0, rect.x1), min(rect.y0, rect.y1) + height, width, color);
	else
		while (width--)
			glcd_draw_vline(min(rect.x0, rect.x1) + width, min(rect.y0, rect.y1), height, color);
}

/*
 * Swaps rectangle coordinates, x0, y0 closest point to origin, x1, y1 is the farthest point to origin.
 */
void rectangle_align_coordinates(rectangle *rect) {
	if (rect->x0 > rect->x1)
		SWAP(rect->x0, rect->x1);

	if (rect->y0 > rect->y1)
		SWAP(rect->y0, rect->y1);
}

uint16_t rectangle_get_width(rectangle rect_a) {
	return max(rect_a.x0, rect_a.x1) - min(rect_a.x0, rect_a.x1) + 1; // never return zero
}

uint16_t rectangle_get_height(rectangle rect_a) {
	return max(rect_a.y0, rect_a.y1) - min(rect_a.y0, rect_a.y1) + 1; // never return zero
}

void rectangle_set_width(rectangle rect_a, int16_t width, rectangle *rect_b) {
	rect_b->x1 = rect_a.x1 + width - abs(rect_a.x1 - rect_a.x0) - 1;
}

void rectangle_set_height(rectangle rect_a, int16_t height, rectangle *rect_b) {
	rect_b->y1 = rect_a.y1 + height - abs(rect_a.y1 - rect_a.y0) - 1;
}

void copy_rectangle(rectangle rect_a, rectangle *rect_b) {
	rect_b->x0 = rect_a.x0;
	rect_b->y0 = rect_a.y0;
	rect_b->x1 = rect_a.x1;
	rect_b->y1 = rect_a.y1;
}

/*
 *  Rectangle rect, contains point with x0 and y0.
 */
bool rectangle_contains_point(rectangle rect, int16_t x0, int16_t y0, bool contiguity) {
	if (contiguity) {
		if (x0 >= rect.x0 && x0 <= rect.x1 && y0 >= rect.y0 && y0 <= rect.y1) // with contiguity
			return true;
	} else {
		if (x0 > rect.x0 && x0 < rect.x1 && y0 > rect.y0 && y0 < rect.y1) // without contiguity
			return true;
	}

	return false;
}

void scale_rectangle(rectangle rect_a, int16_t scale_x, int16_t scale_y, rectangle *rect_b) {
	rect_b->x0 = rect_a.x0 - scale_x;
	rect_b->y0 = rect_a.y0 - scale_y;
	rect_b->x1 = rect_a.x1 + scale_x;
	rect_b->y1 = rect_a.y1 + scale_y;
}

void offset_rectangle(rectangle rect_a, int16_t offset_x, int16_t offset_y, rectangle *rect_b) {
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

void between_rectangle(rectangle rect_a, rectangle rect_b, rectangle *rect_c) { // with contguity, need adjust coordinates
	rect_c->x0 = max(rect_a.x0, rect_b.x0);
	rect_c->y0 = max(rect_a.y0, rect_b.y0);
	rect_c->x1 = min(rect_a.x1, rect_b.x1);
	rect_c->y1 = min(rect_a.y1, rect_b.y1);

	rectangle_align_coordinates(rect_c);
}

bool rectangles_intersect(rectangle rect_a, rectangle rect_b, rectangle *rect_c) {
	int16_t intersect_left = max(rect_a.x0, rect_b.x0);
	int16_t intersect_top = max(rect_a.y0, rect_b.y0);
	int16_t intersect_right = min(rect_a.x1, rect_b.x1);
	int16_t intersect_bottom = min(rect_a.y1, rect_b.y1);

	int16_t intersect_width = intersect_right - intersect_left;
	int16_t intersect_height = intersect_bottom - intersect_top;

	if (intersect_width >= 0 && intersect_height >= 0) { // with contiguity
		rect_c->x0 = max(rect_a.x0, rect_b.x0); // left
		rect_c->y0 = max(rect_a.y0, rect_b.y0); // top
		rect_c->x1 = min(rect_a.x1, rect_b.x1); // right
		rect_c->y1 = min(rect_a.y1, rect_b.y1); // bottom
		return true;
	}

	return false;
}

/*
 *  Rectangle rect_a, fits in rectangle rect_b.
 */
bool rectangles_fit(rectangle rect_a, rectangle rect_b, bool contiguity) {
	if (rectangle_contains_point(rect_b, rect_a.x0, rect_a.y0, contiguity)
			&& rectangle_contains_point(rect_b, rect_a.x1, rect_a.y1, contiguity))
		return true;

	return false;
}

bool rectangles_equal(rectangle rect_a, rectangle rect_b) {
	if (rectangle_get_width(rect_a) == rectangle_get_width(rect_b)
			&& rectangle_get_height(rect_a) == rectangle_get_height(rect_b))
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

	if (rect_c.x0 == rect_d.x0 && rect_c.y0 == rect_d.y0 && rect_c.x1 == rect_d.x1 && rect_c.y1 == rect_d.y1)
		return true;

	return false;
}

void stretch_rectangle_left(rectangle rect_a, int16_t stretch, rectangle *rect_b) {
	rect_b->x0 = min(rect_a.x0, rect_a.x1) + stretch;
	rect_b->y0 = min(rect_a.y0, rect_a.y1);
	rect_b->x1 = max(rect_a.x0, rect_a.x1);
	rect_b->y1 = max(rect_a.y0, rect_a.y1);
}

void stretch_rectangle_top(rectangle rect_a, int16_t stretch, rectangle *rect_b) {
	rect_b->x0 = min(rect_a.x0, rect_a.x1);
	rect_b->y0 = min(rect_a.y0, rect_a.y1) + stretch;
	rect_b->x1 = max(rect_a.x0, rect_a.x1);
	rect_b->y1 = max(rect_a.y0, rect_a.y1);
}

void stretch_rectangle_right(rectangle rect_a, int16_t stretch, rectangle *rect_b) {
	rect_b->x0 = min(rect_a.x0, rect_a.x1);
	rect_b->y0 = min(rect_a.y0, rect_a.y1);
	rect_b->x1 = max(rect_a.x0, rect_a.x1) + stretch;
	rect_b->y1 = max(rect_a.y0, rect_a.y1);
}

void stretch_rectangle_bottom(rectangle rect_a, int16_t stretch, rectangle *rect_b) {
	rect_b->x0 = min(rect_a.x0, rect_a.x1);
	rect_b->y0 = min(rect_a.y0, rect_a.y1);
	rect_b->x1 = max(rect_a.x0, rect_a.x1);
	rect_b->y1 = max(rect_a.y0, rect_a.y1) + stretch;
}

void rectangle_corner_points(rectangle rect, point points[4]) {
	points[0].x0 = rect.x0;
	points[0].y0 = rect.y0;

	points[1].x0 = rect.x1;
	points[1].y0 = rect.y0;

	points[2].x0 = rect.x1;
	points[2].y0 = rect.y1;

	points[3].x0 = rect.x0;
	points[3].y0 = rect.y1;
}

void rectangle_mid_points(rectangle rect, point points[4]) {
	points[0].x0 = rect.x0 + ((rect.x1 - rect.x0) / 2);
	points[0].y0 = rect.y0;

	points[1].x0 = rect.x1;
	points[1].y0 = rect.y1 - ((rect.y1 - rect.y0) / 2);

	points[2].x0 = rect.x1 - ((rect.x1 - rect.x0) / 2);
	points[2].y0 = rect.y1;

	points[3].x0 = rect.x0;
	points[3].y0 = rect.y0 + ((rect.y1 - rect.y0) / 2);
}

point rectangle_point(rectangle rect, uint8_t point_id) {
	point rect_point;

	switch (point_id) {
	case RECT_CP: // center point
		rect_point.x0 = rect.x0 + ((rect.x1 - rect.x0) / 2);
		rect_point.y0 = rect.y0 + ((rect.y1 - rect.y0) / 2);
		break;
	case RECT_TL: // top left point
		rect_point.x0 = rect.x0;
		rect_point.y0 = rect.y0;
		break;
	case RECT_TR: // top right point
		rect_point.x0 = rect.x1;
		rect_point.y0 = rect.y0;
		break;
	case RECT_BR: // bottom right point
		rect_point.x0 = rect.x1;
		rect_point.y0 = rect.y1;
		break;
	case RECT_BL: // bottom left point
		rect_point.x0 = rect.x0;
		rect_point.y0 = rect.y1;
		break;
	case RECT_LM: // left mid point
		rect_point.x0 = rect.x0;
		rect_point.y0 = rect.y0 + ((rect.y1 - rect.y0) / 2);
		break;
	case RECT_TM: // top mid point
		rect_point.x0 = rect.x0 + ((rect.x1 - rect.x0) / 2);
		rect_point.y0 = rect.y0;
		break;
	case RECT_RM: // right mid point
		rect_point.x0 = rect.x1;
		rect_point.y0 = rect.y1 - ((rect.y1 - rect.y0) / 2);
		break;
	case RECT_BM: // bottom mid point
		rect_point.x0 = rect.x1 - ((rect.x1 - rect.x0) / 2);
		rect_point.y0 = rect.y1;
		break;
	}

	return rect_point;
}

uint16_t points_distance(point point_a, point point_b) {
	return (uint16_t) sqrt((point_b.x0 - point_a.x0) * (point_b.x0 - point_a.x0) + (point_b.y0 - point_a.y0) * (point_b.y0 - point_a.y0));
}

/*
 *  Aligns rect_a to rect_b. It is not always possible to accurately align rectangles to the center.
 */
void align_rectangle_in(rectangle rect_a, rectangle rect_b, uint8_t align, rectangle *rect_c) {
	uint16_t a_width, a_height;
	uint16_t b_width, b_height;

	a_width = rectangle_get_width(rect_a);
	a_height = rectangle_get_height(rect_a);
	b_width = rectangle_get_width(rect_b);
	b_height = rectangle_get_height(rect_b);

	switch (align) {
	case ALIGN_CENTER:
		rect_c->x0 = rect_b.x0 + b_width / 2 - a_width / 2;
		rect_c->y0 = rect_b.y0 + b_height / 2 - a_height / 2;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_MID_LEFT:
		rect_c->x0 = rect_b.x0;
		rect_c->y0 = rect_b.y0 + b_height / 2 - a_height / 2;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_MID_TOP:
		rect_c->x0 = rect_b.x0 + b_width / 2 - a_width / 2;
		rect_c->y0 = rect_b.y0;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_MID_RIGHT:
		rect_c->x0 = rect_b.x0 + b_width - a_width;
		rect_c->y0 = rect_b.y0 + b_height / 2 - a_height / 2;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_MID_BOTTOM:
		rect_c->x0 = rect_b.x0 + b_width / 2 - a_width / 2;
		rect_c->y0 = rect_b.y0 + b_height - a_height;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_TOP_LEFT:
	case ALIGN_LEFT_TOP:
		rect_c->x0 = rect_b.x0;
		rect_c->y0 = rect_b.y0;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_BOTTOM_LEFT:
	case ALIGN_LEFT_BOTTOM:
		rect_c->x0 = rect_b.x0;
		rect_c->y0 = rect_b.y0 + b_height - a_height;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_TOP_RIGHT:
	case ALIGN_RIGHT_TOP:
		rect_c->x0 = rect_b.x0 + b_width - a_width;
		rect_c->y0 = rect_b.y0;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_BOTTOM_RIGHT:
	case ALIGN_RIGHT_BOTTOM:
		rect_c->x0 = rect_b.x0 + b_width - a_width;
		rect_c->y0 = rect_b.y0 + b_height - a_height;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	}
}

/*
 *  Removes contiguity.
 */
void align_rectangle_in_contiguity(uint8_t align, rectangle *rect) {
	if (align != ALIGN_CENTER) {
		switch (align) {
		case ALIGN_MID_LEFT:
			offset_rectangle(*rect, 1, 0, rect);
			break;
		case ALIGN_MID_TOP:
			offset_rectangle(*rect, 0, 1, rect);
			break;
		case ALIGN_MID_RIGHT:
			offset_rectangle(*rect, -1, 0, rect);
			break;
		case ALIGN_MID_BOTTOM:
			offset_rectangle(*rect, 0, -1, rect);
			break;
		case ALIGN_LEFT_TOP:
			offset_rectangle(*rect, 1, 1, rect);
			break;
		case ALIGN_LEFT_BOTTOM:
			offset_rectangle(*rect, 1, -1, rect);
			break;
		case ALIGN_RIGHT_TOP:
			offset_rectangle(*rect, -1, 1, rect);
			break;
		case ALIGN_RIGHT_BOTTOM:
			offset_rectangle(*rect, -1, -1, rect);
			break;
		}
	}
}

/*
 *  Aligns rect_a to rect_b. It is not possible to align out rectangles to the center.
 */
void align_rectangle_out(rectangle rect_a, rectangle rect_b, uint8_t align, rectangle *rect_c) {
	uint16_t a_width, a_height;
	uint16_t b_width, b_height;

	a_width = rectangle_get_width(rect_a);
	a_height = rectangle_get_height(rect_a);
	b_width = rectangle_get_width(rect_b);
	b_height = rectangle_get_height(rect_b);

	switch (align) {
	case ALIGN_MID_LEFT:
		rect_c->x0 = rect_b.x0 - a_width;
		rect_c->y0 = rect_b.y0 + b_height / 2 - a_height / 2;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_MID_TOP:
		rect_c->x0 = rect_b.x0 + b_width / 2 - a_width / 2;
		rect_c->y0 = rect_b.y0 - a_height;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_MID_RIGHT:
		rect_c->x0 = rect_b.x0 + b_width;
		rect_c->y0 = rect_b.y0 + b_height / 2 - a_height / 2;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_MID_BOTTOM:
		rect_c->x0 = rect_b.x0 + b_width / 2 - a_width / 2;
		rect_c->y0 = rect_b.y0 + b_width;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_LEFT_BOTTOM:
		rect_c->x0 = rect_b.x0 - a_width;
		rect_c->y0 = rect_b.y0 + b_height - a_height;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_LEFT_TOP:
		rect_c->x0 = rect_b.x0 - a_width;
		rect_c->y0 = rect_b.y0;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_TOP_LEFT:
		rect_c->x0 = rect_b.x0;
		rect_c->y0 = rect_b.y0 - a_height;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_TOP_RIGHT:
		rect_c->x0 = rect_b.x0 + b_width - a_width;
		rect_c->y0 = rect_b.y0 - a_height;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_RIGHT_TOP:
		rect_c->x0 = rect_b.x0 + b_width;
		rect_c->y0 = rect_b.y0;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_RIGHT_BOTTOM:
		rect_c->x0 = rect_b.x0 + b_width;
		;
		rect_c->y0 = rect_b.y0 + b_height - a_height;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_BOTTOM_LEFT:
		rect_c->x0 = rect_b.x0;
		rect_c->y0 = rect_b.y0 + b_height;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	case ALIGN_BOTTOM_RIGHT:
		rect_c->x0 = rect_b.x0 + b_width - a_width;
		rect_c->y0 = rect_b.y0 + b_height;
		rect_c->x1 = rect_c->x0 + a_width - 1;
		rect_c->y1 = rect_c->y0 + a_height - 1;
		break;
	}
}

/*
 *  Creates contiguity.
 */
void align_rectangle_out_contiguity(uint8_t align, rectangle *rect) {
	if (align != ALIGN_CENTER) {
		switch (align) {
		case ALIGN_MID_LEFT:
			offset_rectangle(*rect, 1, 0, rect);
			break;
		case ALIGN_MID_TOP:
			offset_rectangle(*rect, 0, 1, rect);
			break;
		case ALIGN_MID_RIGHT:
			offset_rectangle(*rect, -1, 0, rect);
			break;
		case ALIGN_MID_BOTTOM:
			offset_rectangle(*rect, 0, -1, rect);
			break;
		case ALIGN_LEFT_TOP:
			offset_rectangle(*rect, 0, 1, rect);
			break;
		case ALIGN_LEFT_BOTTOM:
			offset_rectangle(*rect, 0, -1, rect);
			break;
		case ALIGN_RIGHT_TOP:
			offset_rectangle(*rect, 0, 1, rect);
			break;
		case ALIGN_RIGHT_BOTTOM:
			offset_rectangle(*rect, 0, -1, rect);
			break;
		}
	}
}

bool align_rectangle_point(rectangle rect_a, point p, rectangle *rect_b) {
	uint16_t a_width, a_height;

	a_width = rectangle_get_width(rect_a);
	if (!(a_width & 0x01))
		return false;

	a_height = rectangle_get_height(rect_a);
	if (!(a_height & 0x01))
		return false;

	a_width = a_width / 2;
	a_height = a_height / 2;

	rect_b->x0 = p.x0 - a_width;
	rect_b->y0 = p.y0 - a_height;
	rect_b->x1 = p.x0 + a_width;
	rect_b->y1 = p.y0 + a_height;

	return true;
}

//void gui_clear_text_low(rectangle bound, uint8_t align, uint8_t font_id, char text[]) {
//	rectangle area, outline; // component work area and outline for text
//
//	scale_rectangle(bound, -1, -1, &area); // scale to component area
//	outline_text((uint8_t) strlen(text), font_id, &outline); // outline for text that was
//	align_rectangle_in(outline, area, align, &outline); // align text outline
//	align_rectangle_in_contiguity(align, &outline); // correction of align function contiguity
//	draw_painted_rectangle(outline, GLCD_BACKGROUND_COLOR); // fill text outline
//}

void gui_set_text_low(rectangle bound, uint8_t align, uint8_t font_id, char text[]) {
	rectangle area, outline; // component work area and outline for text

	scale_rectangle(bound, -2, -2, &area); // scale to component area
	outline_text((uint8_t) strlen(text), font_id, &outline); // calculate text outline
	align_rectangle_in(outline, area, align, &outline); // align text outline
//	align_rectangle_in_contiguity(align, &outline); // correction of align function contiguity
	draw_text(text, (uint8_t) strlen(text), font_id, rectangle_point(outline, RECT_TL)); // draw text
}

void gui_set_text(gui_component *gui_component, char text[], uint8_t length_old, uint8_t length_new) {
	rectangle area, text_outline, char_outline, char_offset;

	scale_rectangle(*gui_component->bound, -2, -2, &area); // scale to component area
	if (length_new > length_old || length_new < length_old || gui_component->p_temp == text) { // compare specified text and text that was and the first lunch case
		strcpy(gui_component->p_temp, text);
		if (length_old != 0) { // clear old text
			outline_text(length_old, gui_component->font_id, &text_outline);
			align_rectangle_in(text_outline, area, gui_component->align, &text_outline); // align text outline
			draw_painted_rectangle(text_outline, GLCD_COMPONENT_BACKGROUND_COLOR); // fill text outline
		}
		if (length_new != 0) {
			outline_text(length_new, gui_component->font_id, &text_outline);
			align_rectangle_in(text_outline, area, gui_component->align, &text_outline); // align text outline
			draw_text(gui_component->p_temp, length_new, gui_component->font_id, rectangle_point(text_outline, RECT_TL)); // draw text
		}
	} else {
		char_outline = outline_text(length_new, gui_component->font_id, &text_outline);
		align_rectangle_in(text_outline, area, gui_component->align, &text_outline); // align text outline first
		align_rectangle_in(char_outline, text_outline, ALIGN_MID_LEFT, &char_outline); // then align character outline to text outline

		for (length_new = 0; length_new < length_old; length_new++) { // if texts are the same length, try to replace not matching symbols
			if (gui_component->p_temp[length_new] != text[length_new]) {
				offset_rectangle(char_outline, (rectangle_get_width(char_outline)) * length_new, 0, &char_offset);
				draw_painted_rectangle(char_offset, GLCD_COMPONENT_BACKGROUND_COLOR); // fill character outline
				gui_component->p_temp[length_new] = text[length_new];
				draw_char(&text[length_new], gui_component->font_id, rectangle_point(char_offset, RECT_TL)); // draw text
			} else {
				if (text[length_new] == '.') {
					offset_rectangle(char_outline, (rectangle_get_width(char_outline)) * length_new, 0, &char_offset);
					draw_char(&text[length_new], gui_component->font_id, rectangle_point(char_offset, RECT_TL)); // draw text
				}
			}
		}
	}
}

void event_glcd_handle_top_left() {
	glcd_rect.x0 -= 10;
	glcd_rect.y0 -= 10;
	gui_compute_components();
	gui_layout_components();
}

void event_glcd_handle_top_right() {
	glcd_rect.x1 += 10;
	glcd_rect.y0 -= 10;
	gui_compute_components();
	gui_layout_components();
}

void event_glcd_handle_bottom_right() {
	glcd_rect.x1 += 10;
	glcd_rect.y1 += 10;
	gui_compute_components();
	gui_layout_components();
}

void event_glcd_handle_bottom_left() {
	glcd_rect.x0 -= 10;
	glcd_rect.y1 += 10;
	gui_compute_components();
	gui_layout_components();
}

void event_glcd_handle_mid_left() {
	SWAP(glcd_rect.x1, glcd_rect.y1);
	gui_compute_components();
	gui_layout_components();
}

void event_glcd_handle_mid_top() {
	SWAP(glcd_rect.x1, glcd_rect.y1);
	gui_compute_components();
	gui_layout_components();
}

void event_glcd_handle_mid_right() {
	SWAP(glcd_rect.x1, glcd_rect.y1);
	gui_compute_components();
	gui_layout_components();
}

void event_glcd_handle_mid_bottom() {
	SWAP(glcd_rect.x1, glcd_rect.y1);
	gui_compute_components();
	gui_layout_components();
}

void event_pager_left_button() {
	if (gui.pager.page <= gui.pager.page_min)
		gui.pager.page = gui.pager.page_max;
	gui.pager.page--;
	gui_layout_components();
}

void event_pager_right_button() {
	gui.pager.page++;
	if (gui.pager.page >= gui.pager.page_max)
		gui.pager.page = gui.pager.page_min;
	gui_layout_components();
}

void gui_process_events() {
	for (uint8_t i = 0; i < GUI_EVENTS_MAX; i++)
		if (events[i].cmd_event != NULL && events[i].bound != NULL)
			if (rectangle_contains_point(*events[i].bound, mouse_x, mouse_y, true)) {
				(*events[i].cmd_event)();
				break;
			}
}

bool gui_register_event(rectangle *bound, void *cmd_event) {
	bool retv = false;

	for (uint8_t i = 0; i < GUI_EVENTS_MAX; i++)
		if (events[i].cmd_event == NULL && events[i].bound == NULL) {
			events[i].cmd_event = cmd_event;
			events[i].bound = bound;
			retv = true;
			break;
		}

	return retv;
}

void textbox_set_text(gui_textbox *textbox, char text[]) {
	gui_component gui_component;

	gui_component.bound = &textbox->bound;
	gui_component.p_temp = textbox->text;
	gui_component.font_id = textbox->font_id;
	if (textbox->page == gui.pager.page) {
		if (!(textbox->align == ALIGN_CENTER)) {
			gui_component.align = ALIGN_MID_LEFT;
			gui_set_text(&gui_component, text, (uint8_t) strlen(gui_component.p_temp), (uint8_t) strlen(text));
		} else {
			// TODO see where to draw text, update high light bar, see the text fits in texbox
			gui_component.align = ALIGN_CENTER;
			gui_set_text(&gui_component, text, (uint8_t) strlen(gui_component.p_temp), (uint8_t) strlen(text));
		}
	}
}

void textbox_set_value(gui_textbox *textbox, char value[]) {
	gui_component gui_component;

	gui_component.bound = &textbox->bound;
	gui_component.p_temp = textbox->value;
	gui_component.font_id = textbox->font_id;
	if (textbox->page == gui.pager.page) {
		if (!(textbox->align == ALIGN_CENTER)) {
			gui_component.align = ALIGN_MID_RIGHT;
			gui_set_text(&gui_component, value, (uint8_t) strlen(gui_component.p_temp), (uint8_t) strlen(value));
		} else {
			// TODO see where to draw value see the text fits in texbox
		}
	}
}

void gui_render_textbox(gui_textbox *textbox) {
	rectangle area; // component work area

	draw_plain_rectangle(textbox->bound, GLCD_BORDER_COLOR); // draw border
	scale_rectangle(textbox->bound, -1, -1, &area); // scale component area
	draw_painted_rectangle(area, GLCD_COMPONENT_BACKGROUND_COLOR); // fill component area
	textbox_set_text(textbox, textbox->text);
	textbox_set_value(textbox, textbox->value);
}

void gui_render_pager(gui_pager *pager) {
	rectangle area; // component work area

	scale_rectangle(pager->bound, -50, 0, &area); // scale to rectangle with text
	draw_plain_rectangle(area, GLCD_BORDER_COLOR); // draw border
	scale_rectangle(area, -1, -1, &area); // scale component area
	draw_painted_rectangle(area, GLCD_COMPONENT_BACKGROUND_COLOR); // fill area
	gui_set_text_low(area, ALIGN_CENTER, pager->font_id, pager_page_names[pager->page]);

	stretch_rectangle_right(pager->bound, -((50 + rectangle_get_width(pager->bound)) - 1), &pager->left_button);
	align_rectangle_in(pager->left_button, glcd_rect, ALIGN_LEFT_BOTTOM, &pager->left_button);
	draw_plain_rectangle(pager->left_button, GLCD_BORDER_COLOR); // draw border
	scale_rectangle(pager->left_button, -1, -1, &pager->left_button); // scale component area
	draw_painted_rectangle(pager->left_button, GLCD_COMPONENT_BACKGROUND_COLOR); // fill area
	gui_set_text_low(pager->left_button, ALIGN_CENTER, pager->font_id, "\x11");

	stretch_rectangle_left(pager->bound, +((50 + rectangle_get_width(pager->bound)) - 1), &pager->right_button);
	align_rectangle_in(pager->right_button, glcd_rect, ALIGN_RIGHT_BOTTOM, &pager->right_button);
	draw_plain_rectangle(pager->right_button, GLCD_BORDER_COLOR); // draw border
	scale_rectangle(pager->right_button, -1, -1, &pager->right_button); // scale component area
	draw_painted_rectangle(pager->right_button, GLCD_COMPONENT_BACKGROUND_COLOR); // fill area
	gui_set_text_low(pager->right_button, ALIGN_CENTER, pager->font_id, "\x10");
}

//void titlebar_update_text(gui_titlebar *titlebar) {
//	int8_t c = 0; // return value of string compare
//
//	c = strncmp(gui.pager.names[gui.pager.page], titlebar->text, (uint8_t) sizeof(titlebar->text)); // compare specified text and text that was
//	if (c > 0 || c == 0) { // specified text is longer or equal
//		strncpy(titlebar->text, gui.pager.names[gui.pager.page], (uint8_t) sizeof(titlebar->text)); // copy specified text to place
////		gui_set_text(titlebar->bound, ALIGN_CENTER, titlebar->font_id, titlebar->text);
//	} else if (c < 0) { // specified text is shorter
////		gui_clear_text(titlebar->bound, ALIGN_CENTER, titlebar->font_id, titlebar->text);
//		strncpy(titlebar->text, gui.pager.names[gui.pager.page], (uint8_t) sizeof(titlebar->text)); // copy specified text to place
////		gui_set_text(titlebar->bound, ALIGN_CENTER, titlebar->font_id, titlebar->text);
//	}
//}

//void  textbox_set_text(gui_textbox *textbox, char text[]) {
//	int8_t c = 0; // return value of string compare
//
//	if (textbox->page == gui.pager.page) {
//		c = strncmp(text, textbox->text, (uint8_t) sizeof(textbox->text)); // compare specified text and text that was
//		if (c > 0 || c == 0) { // specified text is longer or equal
//			strncpy(textbox->text, text, (uint8_t) sizeof(textbox->text)); // copy specified text to place
////			gui_set_text(textbox->bound, textbox->align, textbox->font_id, textbox->text);
//		} else if (c < 0) { // specified text is shorter
////			gui_clear_text(textbox->bound, textbox->align, textbox->font_id, textbox->text);
//			strncpy(textbox->text, text, (uint8_t) sizeof(textbox->text)); // copy specified text to place
////			gui_set_text(textbox->bound, textbox->align, textbox->font_id, textbox->text);
//		}
//	}
//}

uint16_t writer_set_text(gui_writer *writer, char text[]) {
	rectangle area, outline; // component work area and outline for character
	uint8_t char_state = STATE_VERIFY_CHAR, char_tab = 0;
	uint8_t c_lines = 0, c_line_chars = 0;
	int16_t length_remaining = 0;

	if (writer->page == gui.pager.page) {
		scale_rectangle(writer->bound, -2, -2, &area); // scale to component area
		outline_char(writer->font_id, &outline); // calculate outline of single character
		align_rectangle_in(outline, area, ALIGN_TOP_LEFT, &outline); // align text outline

		while (*text) {
			switch (char_state) {
			case STATE_NEW_LINE:
				char_state = STATE_VERIFY_CHAR;
				c_lines++;
				if (c_lines + 1 > writer->height) {
					length_remaining = strlen(text);
					return length_remaining;
				}
				offset_rectangle(outline, //
						(rectangle_get_width(outline) + writer->spacing) * (-c_line_chars), //
						rectangle_get_height(outline) + writer->interline, //
						&outline);
				c_line_chars = 0;
				break;
			case STATE_VERIFY_LINE:
				char_state = STATE_CHAR_DRAW;
				if ((c_line_chars + (strchr(text, ' ') - text)) > writer->width)
					char_state = STATE_NEW_LINE;
				if (c_line_chars + 1 > writer->width)
					char_state = STATE_NEW_LINE;
				break;
			case STATE_CHAR_DRAW:
				char_state = STATE_CHAR_NEXT;
				if (char_tab == 0)
					draw_char(text++, writer->font_id, rectangle_point(outline, RECT_TL)); // draw character
				break;
			case STATE_CHAR_NEXT:
				char_state = STATE_VERIFY_CHAR;
				offset_rectangle(outline, //
						rectangle_get_width(outline) + writer->spacing, //
						0, //
						&outline);
				c_line_chars++;
				if (char_tab) {
					char_tab--;
					char_state = STATE_VERIFY_LINE;
				}
				break;
			case STATE_VERIFY_CHAR:
				char_state = STATE_VERIFY_LINE;
				switch (*text) {
				case '\n':
					text++; // skip escape character
					char_state = STATE_NEW_LINE;
					break;
				case '\t':
					text++; // skip escape character
					char_tab = 5;
					break;
				case ' ':
					if (c_line_chars == 0)
						text++; // skip space if it is first character on line
					break;
				}
			}
		}
	}

	return length_remaining;
}

void gui_render_writer(gui_writer *writer) {
	rectangle area; // component work area

	draw_plain_rectangle(writer->bound, GLCD_BORDER_COLOR); // draw border
	scale_rectangle(writer->bound, -1, -1, &area); // scale component area
	draw_painted_rectangle(area, ILI9341_NAVY); // fill area
	writer_set_text(&gui.writer, writer->text);
}

void gui_draw_bitmap(uint8_t c_bitmap[], int16_t x0, int16_t y0) {
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	uint16_t stride_width;
	uint16_t color16 = 0;
	uint8_t *p_bitmap;

	memcpy(&bitmapFileHeader, c_bitmap, sizeof(BITMAPFILEHEADER));
	memcpy(&bitmapInfoHeader, c_bitmap + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));

	stride_width = (bitmapInfoHeader.biBitCount * bitmapInfoHeader.biWidth / 32) * 4; // scanline
	p_bitmap = c_bitmap + bitmapFileHeader.bfOffBits;

	for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
		for (uint32_t j = 0; j < stride_width; j += 2) { // bitmap has the little endian byte order
			color16 = (uint16_t) ((p_bitmap[(i * stride_width) + j + 1]) << 8);
			color16 |= (uint16_t) (p_bitmap[(i * stride_width) + j + 0]);
			glcd_set_pixel((j / 2) + x0, (bitmapInfoHeader.biHeight - i) + y0 - 1, color16);
		}
	}
}

void gui_render_painter(gui_painter *painter, uint8_t bitmap[]) {
	rectangle area; // component work area

	draw_plain_rectangle(painter->bound, GLCD_BORDER_COLOR); // draw border
	scale_rectangle(painter->bound, 1, 1, &area); // scale component area
	draw_painted_rectangle(area, GLCD_COMPONENT_BACKGROUND_COLOR); // fill area
	gui_draw_bitmap(bitmap, painter->bound.x0, painter->bound.y0);
}

void pager_compute_pages(gui_pager *pager) {
	pager->page_min = 1;
	pager->page_max = (sizeof(pager->page_max) / sizeof(char *));
	pager->page_max -= 1;
}

void pager_update_text(gui_pager *pager) {
//	static char s_temp[10] = "Page 0/0";
//
//	s_temp[5] = pager->page + '0'; // ASCII code of integer
//	s_temp[7] = pager_max + '0';

//	gui_clear_text(pager->bound, ALIGN_CENTER, pager->font_id, s_temp);
//	gui_set_text(pager->bound, ALIGN_CENTER, pager->font_id, s_temp);
}

void gui_compute_components(void) {
	rectangle outline;

	outline_char(gui.titlebar.font_id, &outline); // textbox with page title
	memset(&gui.titlebar.bound, 0, sizeof(gui.titlebar.bound));
	rectangle_set_width(gui.titlebar.bound, rectangle_get_width(glcd_rect), &gui.titlebar.bound);
	rectangle_set_height(gui.titlebar.bound, 2 + 2 + rectangle_get_height(outline), &gui.titlebar.bound);
	align_rectangle_in(gui.titlebar.bound, glcd_rect, ALIGN_MID_TOP, &gui.titlebar.bound);

	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) { // labels
		outline_text(sizeof(gui.textboxes[i].text), gui.textboxes[i].font_id, &outline);
		memset(&gui.textboxes[i].bound, 0, sizeof(gui.textboxes[i].bound));
		rectangle_set_width(gui.textboxes[i].bound, rectangle_get_width(outline), &gui.textboxes[i].bound);
		rectangle_set_height(gui.textboxes[i].bound, 2 + 2 + rectangle_get_height(outline), &gui.textboxes[i].bound);
	}

	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_textbox)); i++) { // readings
		outline_text((sizeof(gui.readings[i].text) + sizeof(gui.readings[i].value)), gui.readings[i].font_id, &outline);
		memset(&gui.readings[i].bound, 0, sizeof(gui.readings[i].bound));
		rectangle_set_width(gui.readings[i].bound, rectangle_get_width(outline), &gui.readings[i].bound);
		rectangle_set_height(gui.readings[i].bound, 2 + 2 + rectangle_get_height(outline), &gui.readings[i].bound);
	}

	outline_char(gui.pager.font_id, &outline); // pager
	rectangle_set_width(gui.pager.bound, rectangle_get_width(glcd_rect), &gui.pager.bound);
	rectangle_set_height(gui.pager.bound, 2 + 2 + rectangle_get_height(outline), &gui.pager.bound);
	align_rectangle_in(gui.pager.bound, glcd_rect, ALIGN_MID_BOTTOM, &gui.pager.bound);
	gui_register_event(&gui.pager.left_button, event_pager_left_button);
	gui_register_event(&gui.pager.right_button, event_pager_right_button);

	rectangle_set_width(gui.painter.bound, 280, &gui.painter.bound);
	rectangle_set_height(gui.painter.bound, 80, &gui.painter.bound);

	gui_register_event(&glcd_corner_handles[0], &event_glcd_handle_top_left);
	gui_register_event(&glcd_corner_handles[1], &event_glcd_handle_top_right);
	gui_register_event(&glcd_corner_handles[2], &event_glcd_handle_bottom_right);
	gui_register_event(&glcd_corner_handles[3], &event_glcd_handle_bottom_left);

	gui_register_event(&glcd_mid_handles[0], &event_glcd_handle_mid_left);
	gui_register_event(&glcd_mid_handles[1], &event_glcd_handle_mid_top);
	gui_register_event(&glcd_mid_handles[2], &event_glcd_handle_mid_right);
	gui_register_event(&glcd_mid_handles[3], &event_glcd_handle_mid_bottom);
}

void gui_layout_components(void) {
	uint8_t hflow, vflow, margin = 3;
	int16_t block_width, block_height;
	rectangle components_area, rect_block, rect_align;
	rectangle char_outline;
	uint16_t w_width, w_height;

	between_rectangle(gui.titlebar.bound, gui.pager.bound, &components_area); // create components area
	scale_rectangle(components_area, -5, -5, &components_area); // scale components area

	memset(&rect_block, 0, sizeof(rect_block));
	block_width = 0, block_height = 0, hflow = 0, vflow = 0;
	stretch_rectangle_bottom(components_area, -(rectangle_get_height(components_area) / 2), &rect_align);
	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) {
		if (gui.textboxes[i].page == gui.pager.page)
			if (block_width < rectangle_get_width(gui.textboxes[i].bound))
				block_width = rectangle_get_width(gui.textboxes[i].bound);
	}
	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) {
		if (gui.textboxes[i].page == gui.pager.page)
			block_height += rectangle_get_height(gui.textboxes[i].bound) + margin;
	}

	rectangle_set_width(rect_block, block_width, &rect_block);
	rectangle_set_height(rect_block, block_height - 3, &rect_block);
	align_rectangle_in(rect_block, rect_align, ALIGN_CENTER, &rect_block);

	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) { // layout labels
		if (gui.textboxes[i].page == gui.pager.page) {
			align_rectangle_in(gui.textboxes[i].bound, rect_block, ALIGN_MID_TOP, &gui.textboxes[i].bound);
			offset_rectangle(gui.textboxes[i].bound, vflow, hflow, &gui.textboxes[i].bound);
			hflow += rectangle_get_height(gui.textboxes[i].bound) + margin;
		}
	}

	memset(&rect_block, 0, sizeof(rect_block));
	block_width = 0, block_height = 0, hflow = 0, vflow = 0;
	stretch_rectangle_top(components_area, (rectangle_get_height(components_area) / 2), &rect_align);
	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_textbox)); i++) {
		if (gui.readings[i].page == gui.pager.page)
			if (block_width < rectangle_get_width(gui.readings[i].bound))
				block_width = rectangle_get_width(gui.readings[i].bound);
	}
	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_textbox)); i++) {
		if (gui.readings[i].page == gui.pager.page)
			block_height += rectangle_get_height(gui.readings[i].bound) + margin;
	}

	rectangle_set_width(rect_block, block_width, &rect_block);
	rectangle_set_height(rect_block, block_height - 3, &rect_block);
	align_rectangle_in(rect_block, rect_align, ALIGN_CENTER, &rect_block);

	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_textbox)); i++) { // layout readings
		if (gui.readings[i].page == gui.pager.page) {
			align_rectangle_in(gui.readings[i].bound, rect_block, ALIGN_MID_TOP, &gui.readings[i].bound);
			offset_rectangle(gui.readings[i].bound, vflow, hflow, &gui.readings[i].bound);
			hflow += rectangle_get_height(gui.readings[i].bound) + margin;
		}
	}

	outline_char(gui.writer.font_id, &char_outline);
	w_width = (rectangle_get_width(char_outline) + gui.writer.spacing);
	w_height = (rectangle_get_height(char_outline) + gui.writer.interline);

	align_rectangle_in(gui.writer.bound, components_area, ALIGN_CENTER, &rect_align);
	while (rectangles_fit(rect_align, components_area, false)) {
		gui.writer.width++;
		rectangle_set_width(rect_align, (gui.writer.width * w_width) - gui.writer.spacing + 4, &rect_align);
		align_rectangle_in(rect_align, components_area, ALIGN_CENTER, &rect_align);
	}
	rectangle_set_width(gui.writer.bound, (--gui.writer.width * w_width) - gui.writer.spacing + 4, &gui.writer.bound);
	align_rectangle_in(gui.writer.bound, components_area, ALIGN_CENTER, &rect_align);
	while (rectangles_fit(rect_align, components_area, false)) {
		gui.writer.height++;
		rectangle_set_height(rect_align, (gui.writer.height * w_height) - gui.writer.interline + 4, &rect_align);
		align_rectangle_in(rect_align, components_area, ALIGN_CENTER, &rect_align);
	}
	rectangle_set_height(gui.writer.bound, (--gui.writer.height * w_height) - gui.writer.interline + 4, &gui.writer.bound);
	align_rectangle_in(gui.writer.bound, components_area, ALIGN_CENTER, &gui.writer.bound);

	align_rectangle_in(gui.painter.bound, components_area, ALIGN_CENTER, &gui.painter.bound);
}

void gui_light_highligths(void) {
	rectangle rect = { -27, -27, -25, -25 };
	point rect_point;
	rectangle rect_stretch;
	uint16_t color;

	if (glcd_led1 == true)
		color = ILI9341_RED;
	else
		 color = ILI9341_GREEN;

	stretch_rectangle_left(gui.titlebar.bound, 10, &rect_stretch);
	rect_point = rectangle_point(rect_stretch, RECT_LM);
	draw_circle(rect_point.x0, rect_point.y0, 2, color);
	if (align_rectangle_point(rect, rect_point, &rect))
		draw_painted_rectangle(rect, color);

	if (glcd_led2 == true)
		color = ILI9341_RED;
	else
		 color = ILI9341_GREEN;

	stretch_rectangle_left(gui.titlebar.bound, 20, &rect_stretch);
	rect_point = rectangle_point(rect_stretch, RECT_LM);
	draw_circle(rect_point.x0, rect_point.y0, 2, color);
	if (align_rectangle_point(rect, rect_point, &rect))
		draw_painted_rectangle(rect, color);

	if (glcd_led3 == true)
		color = ILI9341_RED;
	else
		 color = ILI9341_GREEN;

	stretch_rectangle_left(gui.titlebar.bound, 30, &rect_stretch);
	rect_point = rectangle_point(rect_stretch, RECT_LM);
	draw_circle(rect_point.x0, rect_point.y0, 2, color);
	if (align_rectangle_point(rect, rect_point, &rect))
		draw_painted_rectangle(rect, color);
}

void draw_rectangle_handles() {
	rectangle rect_handle = { -27, -27, -21, -21 };
	point glcd_points[4];

	rectangle_corner_points(glcd_rect, glcd_points);
	for (uint8_t i = 0; i < sizeof(glcd_points) / sizeof(glcd_points[0]); i++)
		if (align_rectangle_point(rect_handle, glcd_points[i], &glcd_corner_handles[i]))
			draw_painted_rectangle(glcd_corner_handles[i], ILI9341_YELLOW);

	rectangle_mid_points(glcd_rect, glcd_points);
	for (uint8_t i = 0; i < sizeof(glcd_points) / sizeof(glcd_points[0]); i++)
		if (align_rectangle_point(rect_handle, glcd_points[i], &glcd_mid_handles[i]))
			draw_painted_rectangle(glcd_mid_handles[i], ILI9341_YELLOW);
}

void glcd_background_grid() {
//	int16_t glcd_grid_horizontal = rectangle_get_width(glcd_rect);
//	int16_t glcd_grid_vertical = rectangle_get_height(glcd_rect);
//	int8_t glcd_grid_step = 3;
//
//	for (uint16_t i = 2; i < glcd_grid_vertical - 2; i += glcd_grid_step) { // grid lines x-axis parallel
//		glcd_draw_hline(0, i, glcd_grid_horizontal + 1, RGB565(0x1111));
//	}
//	for (uint16_t i = 2; i < glcd_grid_horizontal - 2; i += glcd_grid_step) { // grid lines y-axis parallel
//		glcd_draw_vline(i, 0, glcd_grid_vertical + 1, RGB565(0x1111));
//	}
}

void gui_render_components(void) {
	draw_rectangle_handles();
	glcd_clear(GLCD_BACKGROUND_COLOR);
	draw_plain_rectangle(glcd_rect, GLCD_BORDER_COLOR);

	glcd_background_grid();
	gui_render_textbox(&gui.titlebar);
	gui_light_highligths();

	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) {
		if (gui.textboxes[i].page == gui.pager.page) // check page
			gui_render_textbox(&gui.textboxes[i]);
	}

	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_textbox)); i++) {
		if (gui.readings[i].page == gui.pager.page) // check page
			gui_render_textbox(&gui.readings[i]);
	}

	gui_render_pager(&gui.pager);

	if (gui.painter.page == gui.pager.page)
		gui_render_painter(&gui.painter, apocrypha_bmp);

	if (gui.writer.page == gui.pager.page)
		gui_render_writer(&gui.writer);
}

bool gui_begin_drag_component(void) { // operates on global variables
	if (rectangle_contains_point(gui.titlebar.bound, mouse_x, mouse_y, true)) {
		gui_drag_component = true;
		rect_drag = &glcd_rect;
		return true;
	}

	for (uint8_t i = 0; i < (sizeof(gui.textboxes) / sizeof(gui_textbox)); i++) {
		if (gui.textboxes[i].page == gui.pager.page) {
			if (rectangle_contains_point(gui.textboxes[i].bound, mouse_x, mouse_y, true)) {
				gui_drag_component = true;
//				drag_shift_x = mouse_x - min(gui.textboxes[i].bound.x0, gui.textboxes[i].bound.x1);
//				drag_shift_y = mouse_y - min(gui.textboxes[i].bound.y0, gui.textboxes[i].bound.y1);
				rect_drag = &gui.textboxes[i].bound;
				return true;
			}
		}
	}

	for (uint8_t i = 0; i < (sizeof(gui.readings) / sizeof(gui_textbox)); i++) {
		if (gui.readings[i].page == gui.pager.page) {
			if (rectangle_contains_point(gui.readings[i].bound, mouse_x, mouse_y, true)) {
				gui_drag_component = true;
//				drag_shift_x = mouse_x - min(gui.readings[i].bound.x0, gui.readings[i].bound.x1);
//				drag_shift_y = mouse_y - min(gui.readings[i].bound.y0, gui.readings[i].bound.y1);
				rect_drag = &gui.readings[i].bound;
				return true;
			}
		}
	}

	return false;
}

void gui_perform_drag_component(bool boundless) { // operates on global variables
//	if (!boundless) {
//		copy_rectangle(*rect_drag, &rect_drag_temp);
//		offset_rectangle(rect_drag_temp, &rect_drag_temp, -mouse_dx, -mouse_dy);
//		if (rectangles_fit(glcd_rect, rect_drag_temp)
//				&& point_in_rectangle(mouse_x, mouse_y, rect_drag_temp, true)) {
//			offset_rectangle(*rect_drag, rect_drag, -mouse_dx, -mouse_dy);
//			drag_shift_x = mouse_x - min(rect_drag->x0, rect_drag->x1);
//			drag_shift_y = mouse_y - min(rect_drag->y0, rect_drag->y1);
//		}
//	} else {
	offset_rectangle(*rect_drag, -mouse_dx, -mouse_dy, rect_drag);
	gui_recompute = true;
	gui_relayout = true;
//	}
}

void gui_end_drag_component() { // operates on global variables
	gui_drag_component = false;
//		drag_shift_x = 0;
//		drag_shift_y = 0;
//		gui_recompute_and_recalc = false;
}

void draw_bitmap_fs(char *filename, uint16_t x0, uint16_t y0) {
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;

//    bool v4header = false;
	BITMAPV4HEADER bitmapV4header;
//    bool v5header = false;
	BITMAPV5HEADER bitmapV5header;

	FILE *fileHandle;
	uint8_t *p_bitmap, *p_palette;
	uint8_t b_bits, b_color;
	uint32_t stride_width;
	uint16_t color16 = 0;

	fileHandle = fopen(filename, "rb"); // open file in read binary mode
	if (fileHandle == NULL) {

	}

	memset(&bitmapFileHeader, 0, sizeof(bitmapFileHeader));
	memset(&bitmapInfoHeader, 0, sizeof(bitmapInfoHeader));
	memset(&bitmapV4header, 0, sizeof(BITMAPV4HEADER));
	memset(&bitmapV5header, 0, sizeof(BITMAPV5HEADER));

	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, fileHandle);
	fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, fileHandle);
	if (bitmapFileHeader.bfType != 0x4D42) { // verify file format id id

	} else {
		if (bitmapInfoHeader.biSize != sizeof(BITMAPINFOHEADER)) { // try other headers
			fseek(fileHandle, 0, SEEK_SET);
			fread(&bitmapV4header, sizeof(bitmapV4header), 1, fileHandle);
//			v4header = true;
			if (bitmapV4header.bV4Size != sizeof(BITMAPV4HEADER)) {
				fseek(fileHandle, 0, SEEK_SET);
				fread(&bitmapV5header, sizeof(bitmapV5header), 1, fileHandle);
//				v5header = true;
			}
		}
	}

	b_bits = bitmapInfoHeader.biBitCount; // number of bits designated to color
	b_color = (uint8_t) b_bits / 8; // number of bytes designated to color
	//stride_width = (b_bits * bitmapInfoHeader.biWidth / 32) * 4; // scanline
	stride_width = ((b_bits * bitmapInfoHeader.biWidth + 31) / 32) * 4; // scanline

//    fseek(fileHandle, sizeof(BITMAPFILEHEADER) + bitmapInfoHeader.biSize, SEEK_SET);
//    fread(palette, bitmapInfoHeader.biClrUsed*4, 1, fileHandle);

	if (bitmapInfoHeader.biSizeImage != 0) {
		p_bitmap = (uint8_t *) malloc(bitmapInfoHeader.biSizeImage); // allocate memory
		if (p_bitmap == NULL) {

		}
	} else {

	}

	fseek(fileHandle, bitmapFileHeader.bfOffBits, SEEK_SET); // move file pointer to bitmap data
	fread(p_bitmap, bitmapInfoHeader.biSizeImage, 1, fileHandle); // read bitmap data

	uint8_t w, x;

	if (b_bits == 1) {
		glBegin(GL_POINTS);
		for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
			for (uint32_t j = 0; j < bitmapInfoHeader.biWidth / 8; j += 1) { // bitmap has the little endian byte order
				w = p_bitmap[(i * stride_width) + j];
				for (x = 0; x < 8; x++) {
					if (w & 1) { // character
						glColor3ub(255, 255, 255);
					} else { // background
						glColor3ub(0, 0, 0);
					}
					//glVertex2i(j*8 + x + x0, i + y0); // top down
					glVertex2i(j * 8 - x + x0 + 7, bitmapInfoHeader.biHeight - i + y0 - 1); // top up
					w = w >> 1;
				}
			}
		}
		glEnd();
	} else if (b_bits == 4) {
		p_palette = (uint8_t *) malloc(16 * 4); // allocate memory
		if (p_palette == NULL) {

		}

		fseek(fileHandle, sizeof(BITMAPFILEHEADER) + bitmapInfoHeader.biSize, SEEK_SET); // read palette
		fread(p_palette, 16 * 4, 1, fileHandle);

		glBegin(GL_POINTS);
		for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
			for (uint32_t j = 0; j < stride_width; j += 1) { // bitmap has the little endian byte order
				glColor3ub(p_palette[(p_bitmap[(i * stride_width) + j] >> 4) * 4 + 2], p_palette[(p_bitmap[(i * stride_width) + j] >> 4) * 4 + 1],
						p_palette[(p_bitmap[(i * stride_width) + j] >> 4) * 4 + 0]);
//   			glVertex2i(j / b_color + x0, i + y0); // top down
				glVertex2i(j * 2 + x0, bitmapInfoHeader.biHeight - i + y0 - 1); // top up

				glColor3ub(p_palette[(p_bitmap[(i * stride_width) + j] & 0x0F) * 4 + 2], p_palette[(p_bitmap[(i * stride_width) + j] & 0x0F) * 4 + 1],
						p_palette[(p_bitmap[(i * stride_width) + j] & 0x0F) * 4 + 0]);
//				glVertex2i(j / b_color + x0, i + y0); // top down
				glVertex2i(j * 2 + 1 + x0, bitmapInfoHeader.biHeight - i + y0 - 1); // top up
			}
		}
		glEnd();

		free(p_palette);
	} else if (b_bits == 8) {
		p_palette = (uint8_t *) malloc(bitmapInfoHeader.biClrUsed * 4); // allocate memory
		if (p_palette == NULL) {

		}

		fseek(fileHandle, sizeof(BITMAPFILEHEADER) + bitmapInfoHeader.biSize, SEEK_SET); // read palette
		fread(p_palette, bitmapInfoHeader.biClrUsed * 4, 1, fileHandle);

		glBegin(GL_POINTS);
		for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
			for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
				glColor3ub(p_palette[p_bitmap[(i * stride_width) + j] * 4 + 2], p_palette[p_bitmap[(i * stride_width) + j] * 4 + 1],
						p_palette[p_bitmap[(i * stride_width) + j] * 4 + 0]);
//   			glVertex2i(j / b_color + x0, i + y0); // top down
				glVertex2i(j / b_color + x0, bitmapInfoHeader.biHeight - i + y0 - 1); // top up
			}
		}
		glEnd();

		free(p_palette);
	} else if (b_bits == 16) {
		glBegin(GL_POINTS);
		for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
			for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
				color16 = (uint16_t) ((p_bitmap[(i * stride_width) + j + 1]) << 8);
				color16 |= (uint16_t) (p_bitmap[(i * stride_width) + j + 0]);
				glColor3ub(RGB565_R(color16), RGB565_G(color16), RGB565_B(color16));
//				glVertex2i(j / b_color + x0, i + y0); // top down
				glVertex2i(j / b_color + x0, bitmapInfoHeader.biHeight - i + y0 - 1); // top up
			}
		}
		glEnd();
	} else if (b_bits == 24) {
		glBegin(GL_POINTS);
		for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
			for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
				glColor3ub(p_bitmap[(i * stride_width) + j + 2], // R
						p_bitmap[(i * stride_width) + j + 1], // G
						p_bitmap[(i * stride_width) + j + 0]  // B
						);
//				glVertex2i(j / b_color + x0, i + y0); // top down
				glVertex2i(j / b_color + x0, bitmapInfoHeader.biHeight - i + y0 - 1); // top up
			}
		}
		glEnd();
	} else if (b_bits == 32) {
		glBegin(GL_POINTS);
		for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
			for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
				glColor3ub(p_bitmap[(i * stride_width) + j + 2], // R
						p_bitmap[(i * stride_width) + j + 1], // G
						p_bitmap[(i * stride_width) + j + 0]  // B
						);
//				glVertex2i(j / b_color + x0, i + y0); // top down
				glVertex2i(j / b_color + x0, bitmapInfoHeader.biHeight - i + y0 - 1); // top up
			}
		}
		glEnd();
	}

	free(p_bitmap);
	fclose(fileHandle); // close file
}

void draw_bitmap_ca(const uint8_t c_bitmap[], uint16_t x0, uint16_t y0) { // c array
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;

	uint8_t b_bits, b_color;
	uint32_t stride_width;
	uint16_t color16 = 0;
	uint8_t *p_bitmap;

	memcpy(&bitmapFileHeader, c_bitmap, sizeof(BITMAPFILEHEADER));
	memcpy(&bitmapInfoHeader, c_bitmap + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));
	if (bitmapFileHeader.bfType == 0x4D42) { // verify that file is bitmap

	} else {
		if (bitmapInfoHeader.biSize != sizeof(BITMAPINFOHEADER)) { // try other headers

		}
	}

	b_bits = bitmapInfoHeader.biBitCount; // number of bits designated to color
	b_color = (uint8_t) b_bits / 8; // number of bytes designated to color
	p_bitmap = (uint8_t *) c_bitmap + bitmapFileHeader.bfOffBits; // pointer to bitmap data
	stride_width = (b_bits * bitmapInfoHeader.biWidth / 32) * 4; // scanline

	if (b_bits == 8) {
		glBegin(GL_POINTS);
		for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
			for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
				glColor3ub(palette[p_bitmap[(i * stride_width) + j] * 4 + 2], palette[p_bitmap[(i * stride_width) + j] * 4 + 1],
						palette[p_bitmap[(i * stride_width) + j] * 4 + 0]);
//   			glVertex2i(j / b_color + x0, i + y0); // top down
				glVertex2i(j / b_color + x0, bitmapInfoHeader.biHeight - i + y0 - 1); // top up
			}
		}
		glEnd();
	} else if (b_bits == 16) {
		glBegin(GL_POINTS);
		for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
			for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
				color16 = (uint16_t) ((p_bitmap[(i * stride_width) + j + 1]) << 8);
				color16 |= (uint16_t) (p_bitmap[(i * stride_width) + j + 0]);
				glColor3ub(RGB565_R(color16), RGB565_G(color16), RGB565_B(color16));
//				glVertex2i(j / b_color + x0, i + y0); // top down
				glVertex2i(j / b_color + x0, bitmapInfoHeader.biHeight - i + y0 - 1); // top up
			}
		}
		glEnd();
	} else if (b_bits == 24) {
		glBegin(GL_POINTS);
		for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
			for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
				glColor3ub(p_bitmap[(i * stride_width) + j + 2], // R
						p_bitmap[(i * stride_width) + j + 1], // G
						p_bitmap[(i * stride_width) + j + 0]  // B
						);
//				glVertex2i(j / b_color + x0, i + y0); // top down
				glVertex2i(j / b_color + x0, bitmapInfoHeader.biHeight - i + y0 - 1); // top up
			}
		}
		glEnd();
	} else if (b_bits == 32) {
		glBegin(GL_POINTS);
		for (uint32_t i = 0; i < bitmapInfoHeader.biHeight; i++) {
			for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
				glColor3ub(p_bitmap[(i * stride_width) + j + 2], // R
						p_bitmap[(i * stride_width) + j + 1], // G
						p_bitmap[(i * stride_width) + j + 0]  // B
						);
//				glVertex2i(j / b_color + x0, i + y0); // top down
				glVertex2i(j / b_color + x0, bitmapInfoHeader.biHeight - i + y0 - 1); // top up
			}
		}
		glEnd();
	}
}

void draw_bitmap32(uint8_t *bmp_bytes, uint32_t size, uint32_t width, uint32_t height) {
	const uint8_t bits = 32;
	const uint8_t b_color = 4; // bytes designated to color
	uint8_t *stride;
	uint32_t stride_width = (bits * width / 32) * 4; // scanline

	stride = (uint8_t *) malloc(stride_width);
	glBegin(GL_POINTS);
	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
			glColor3ub(bmp_bytes[(i * stride_width) + j + 2], // R
					bmp_bytes[(i * stride_width) + j + 1], // G
					bmp_bytes[(i * stride_width) + j + 0]  // B
					);
			//glVertex2i((j/b_color)+10, (i)+10); // top down
			glVertex2i((j / b_color) + 10, (height - i) + 10); // top up
		}
	}
	glEnd();
	free(stride);
}

void draw_bitmap24(uint8_t *bmp_bytes, uint32_t size, uint32_t width, uint32_t height) {
	const uint8_t bits = 24;
	const uint8_t b_color = 3; // bytes designated to color
	uint8_t *stride;
	uint32_t stride_width = (bits * width / 32) * 4; // scanline

	stride = (uint8_t *) malloc(stride_width);
	glBegin(GL_POINTS);
	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
			glColor3ub(bmp_bytes[(i * stride_width) + j + 2], // R
					bmp_bytes[(i * stride_width) + j + 1], // G
					bmp_bytes[(i * stride_width) + j + 0]  // B
					);
			//glVertex2i((j/b_color)+10, (i)+10); // top down
			glVertex2i((j / b_color) + 10, (height - i) + 10); // top up
		}
	}
	glEnd();
	free(stride);
}

void draw_bitmap16(uint8_t *bmp_bytes, uint32_t size, uint32_t width, uint32_t height) {
	const uint8_t bits = 16;
	const uint8_t b_color = 2; // bytes designated to color
	uint8_t *stride;
	uint32_t stride_width = (bits * width / 32) * 4; // scanline
	uint16_t color16 = 0;

	stride = (uint8_t *) malloc(stride_width);
	glBegin(GL_POINTS);
	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
			color16 = (uint16_t) ((bmp_bytes[(i * stride_width) + j + 1]) << 8);
			color16 |= (uint16_t) (bmp_bytes[(i * stride_width) + j + 0]);
			glColor3ub(RGB565_R(color16), RGB565_G(color16), RGB565_B(color16));
			//glVertex2i((j/b_color)+10, (i)+10); // top down
			glVertex2i((j / b_color) + 10, (height - i) + 10); // top up
		}
	}
	glEnd();
	free(stride);
}

void draw_bitmap8(uint8_t *bmp_bytes, uint32_t size, uint32_t width, uint32_t height) {
	const uint8_t bits = 8;
	const uint8_t b_color = 1; // bytes designated to color
	uint8_t *stride;
	uint32_t stride_width = (bits * width / 32) * 4; // scanline

	stride = (uint8_t *) malloc(stride_width);
	glBegin(GL_POINTS);
	for (uint32_t i = 0; i < height; i++) {
		for (uint32_t j = 0; j < stride_width; j += b_color) { // bitmap has the little endian byte order
			glColor3ub(palette[bmp_bytes[(i * stride_width) + j]] + 2, palette[bmp_bytes[(i * stride_width) + j]] + 1,
					palette[bmp_bytes[(i * stride_width) + j]] + 0);
			//glVertex2i((j/b_color)+10, (i)+10); // top down
			glVertex2i((j / b_color) + 10, (height - i) + 10); // top up
		}
	}
	glEnd();
	free(stride);
}

void draw_text_3d(const char text[], float x, float y, float z, ...) {
	char szBuffer[1024];
	va_list args;

	va_start(args, z);
	memset(szBuffer, 0, sizeof(szBuffer));
	vsnprintf(szBuffer, sizeof(szBuffer), text, args);

	glColor3f(0.75f, 0.75f, 0.75f);
	glListBase(fontListBase);
	glRasterPos3f(x, y, z);
	glCallLists(strlen(szBuffer), GL_UNSIGNED_BYTE, szBuffer);

	va_end(args);
}

void draw_text_2d(const char text[], float x, float y, ...) {
	char szBuffer[1024];
	va_list args;

	va_start(args, y);
	memset(szBuffer, 0, sizeof(szBuffer));
	vsnprintf(szBuffer, sizeof(szBuffer), text, args);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	gluOrtho2D(0.0f, frame_width, 0.0f, frame_height);
	glColor3f(0.75f, 0.75f, 0.75f);
	glListBase(fontListBase);
	glRasterPos2f(x, y);
	glCallLists(strlen(szBuffer), GL_UNSIGNED_BYTE, szBuffer);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	va_end(args);
}

void rectangle_align_demo() {
	rectangle rect1 = { 90, 70, 170, 150 };
	rectangle rect2 = { 10, 10, 60, 60 };
	rectangle rect3 = { 0, 0, 0, 0 };

	draw_plain_rectangle(rect1, ILI9341_RED);
	draw_plain_rectangle(rect2, ILI9341_GREENYELLOW);
	align_rectangle_in(rect2, rect1, ALIGN_CENTER, &rect3);
	draw_plain_rectangle(rect3, ILI9341_BLUE);
}

void rectangle_points_demo() {
	rectangle rect = { 90, 70, 170, 150 };

	point corner_points[4];
	point mid_points[4];
	point *target_point;
	point mouse_point = { mouse_x, mouse_y };
	uint16_t target_distance;

	rectangle_corner_points(rect, corner_points);
	rectangle_mid_points(rect, mid_points);
	draw_plain_rectangle(rect, ILI9341_GREEN);

	target_distance = 0;
	for (uint8_t i = 0; i < sizeof(corner_points) / sizeof(point); i++) {
		draw_circle(corner_points[i].x0, corner_points[i].y0, 2, ILI9341_YELLOW);
		if (points_distance(corner_points[i], mouse_point) > target_distance) {
			target_distance = points_distance(corner_points[i], mouse_point);
			target_point = &corner_points[i]; // farthest point
//			target_point = &corner_points[(i + 2) % sizeof(point)]; // closest point
		}
	}
	if (target_distance > 0)
		draw_line(mouse_point.x0, mouse_point.y0, target_point->x0, target_point->y0, ILI9341_WHITE);

	target_distance = 0;
	for (uint8_t i = 0; i < sizeof(mid_points) / sizeof(point); i++) {
		draw_circle(mid_points[i].x0, mid_points[i].y0, 2, ILI9341_YELLOW);
		if (points_distance(mid_points[i], mouse_point) > target_distance) {
			target_distance = points_distance(mid_points[i], mouse_point);
//			target_point = &mid_points[i]; // farthest point
			target_point = &mid_points[(i + 2) % sizeof(point)]; // closest point
		}
	}
	if (target_distance > 0)
		draw_line(mouse_point.x0, mouse_point.y0, target_point->x0, target_point->y0, ILI9341_WHITE);
}

void glcd_demo(void) {
//	bmp_bytes = load_bitmap_file("C:\\Users\\Sacredlore\\Desktop\\dlance.bmp");
//	draw_bitmap32(bmp_bytes, 66048, 128, 172);
//	draw_bitmap24(bmp_bytes, 66048, 128, 172);
//	draw_bitmap16(bmp_bytes, 66048, 128, 172);
//	draw_bitmap8(bmp_bytes, 66048, 128, 172);
//	draw_bitmap_ca(claymore_bmp, 0, 0);
//	draw_bitmap_fs("C:\\Users\\Sacredlore\\Desktop\\dlance1bit.bmp", 0, 0);

//	draw_plain_rectangle(rect1, ILI9341_RED);
//	printf("%d %d %d %d |%d\n", rect1.x0, rect1.y0, rect1.x1, rect1.y1, rectangle_get_width(rect1));
//	rectangle_set_width(rect1, 0, &rect1);
//	draw_plain_rectangle(rect1, ILI9341_GREEN);
//	printf("%d %d %d %d |%d\n", rect1.x0, rect1.y0, rect1.x1, rect1.y1, rectangle_get_width(rect1));

	draw_text_2d("drag_shift_x, drag_shift_y: %d, %d", 10, 40, drag_shift_x, drag_shift_y);
	draw_text_2d("mouse_dx, mouse_dy: %d, %d", 10, 50, mouse_dx, mouse_dy);
	draw_text_2d("mouse_x, mouse_y: %d, %d", 10, 60, mouse_x, mouse_y);

//	rectangle rect1 = { 0, 0, 319, 30 };
//	rectangle rect2 = { 0, 210, 319, 219 };
//	rectangle rect3 = { 100, 100, 110, 110 };

//	draw_plain_rectangle(rect1, ILI9341_RED);
//	draw_plain_rectangle(rect2, ILI9341_GREENYELLOW);
//	between_rectangle(rect1, rect2, &rect3);
//	draw_plain_rectangle(rect3, ILI9341_BLUE);

//	printf("{ %d, %d, %d, %d }\n", rect3.x0, rect3.y0, rect3.x1, rect3.y1);
//	rectangle_set_width(rect3, 100, &rect3);
//	printf("{ %d, %d, %d, %d }\n", rect3.x0, rect3.y0, rect3.x1, rect3.y1);
//	printf("%d\n", rectangle_get_width(rect3));

	float clocks;
	char s_temp[256];

	clock_t begin = clock();

	if (gui_recompute) {
		gui_compute_components();
		gui_recompute = false;
	}

	if (gui_relayout) {
		gui_layout_components();
		gui_relayout = false;
	}

	if (gui_rerender) { // works in theory
		gui_frame_counter++;
		if (gui_frame_counter) {
			gui_frame_counter--;
			gui_render_components();
			gui_rerender = true;
		}
	}

//	textbox_set_text(&gui.textboxes[0], "label_x");
//	textbox_set_text(&gui.textboxes[3], "aAzzQq");
//
//	textbox_set_text(&gui.textboxes[0], "a");
//	textbox_set_text(&gui.textboxes[3], "q");
//
//	textbox_set_text(&gui.textboxes[0], "");
//	textbox_set_text(&gui.textboxes[3], "");
//
//	textbox_set_text(&gui.textboxes[0], "label_x");
//	textbox_set_text(&gui.textboxes[3], "aAzzQq");

	textbox_set_text(&gui.readings[3], "mouse_x:");
	sprintf(s_temp, "%d", mouse_x);
	textbox_set_value(&gui.readings[3], s_temp);

	textbox_set_text(&gui.readings[4], "mouse_y:");
	sprintf(s_temp, "%d", mouse_y);
	textbox_set_value(&gui.readings[4], s_temp);

//	writer_set_text(&gui.writer, "default LONG TEXT ...");
	writer_set_text(&gui.writer, lord_of_light);

	clock_t end = clock();

	clocks = (float) (end - begin) / CLOCKS_PER_SEC;
	textbox_set_text(&gui.readings[2], "clock():");
	sprintf(s_temp, "%.3f", clocks);
	textbox_set_value(&gui.readings[2], s_temp);

}

void draw_ortho_gridlines(int grid_horizontal, int grid_vertical, int grid_step) {
	float radius_exterior = 200.0f;
	float radius_interior = 150.0f;
	float axis_extension = 30.0f;

	float grid_circle[24][2];
	float grid_circle_sectors = sizeof(grid_circle) / sizeof(float) / 2;

	int circular_index = 0;
	int loop_counter = grid_circle_sectors;

	for (int i = -grid_vertical; i <= grid_vertical; i += grid_step) { // x axis
		glBegin(GL_LINES);
		if (i % 2)
			glColor4f(0.1f, 0.1f, 0.1f, 0.5f);
		else {
			glColor4f(0.2f, 0.2f, 0.2f, 0.5f);
		}
		glVertex2i(-grid_horizontal, i);
		glVertex2i(grid_horizontal, i);
		glEnd();
	}

	glColor4f(0.1f, 0.1f, 0.1f, 0.9f);
	for (int i = -grid_horizontal; i <= grid_horizontal; i += grid_step) { // y axis
		glBegin(GL_LINES);
		if (i % 2)
			glColor4f(0.1f, 0.1f, 0.1f, 0.5f);
		else {
			glColor4f(0.2f, 0.2f, 0.2f, 0.5f);
		}
		glVertex2i(i, -grid_vertical);
		glVertex2i(i, grid_vertical);
		glEnd();
	}

	for (int i = 0; i < grid_circle_sectors; i++) { // compute circle coordinates
		grid_circle[i][0] = cos(M_PI * 2.0f / grid_circle_sectors * i); // x-coordinate
		grid_circle[i][1] = sin(M_PI * 2.0f / grid_circle_sectors * i); // y-coordinate
	}

	glColor4ub(RGB888_R(GOLD), RGB888_G(GOLD), RGB888_B(GOLD), 128); // exterior circle
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < grid_circle_sectors; i++) {
		glVertex2f(grid_circle[i][0] * radius_exterior, grid_circle[i][1] * radius_exterior);
	}
	glEnd();

	glColor4ub(RGB888_R(GOLD), RGB888_G(GOLD), RGB888_B(GOLD), 128); // interior circle
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < grid_circle_sectors; i++) {
		glVertex2f(grid_circle[i][0] * radius_interior, grid_circle[i][1] * radius_interior);
	}
	glEnd();

	glColor4ub(RGB888_R(GOLD), RGB888_G(GOLD), RGB888_B(GOLD), 128); // clock lines
	glBegin(GL_LINES);
	for (int i = 0; i < grid_circle_sectors; i++) {
		glVertex2f(grid_circle[i][0] * radius_exterior, grid_circle[i][1] * radius_exterior);
		glVertex2f(grid_circle[i][0] * radius_interior, grid_circle[i][1] * radius_interior);
	}
	glEnd();

	glColor4ub(RGB888_R(GOLD), RGB888_G(GOLD), RGB888_B(GOLD), 16); // polygon coating
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i < loop_counter + 1; i++) {
		glVertex2f(grid_circle[circular_index][0] * radius_interior, grid_circle[circular_index][1] * radius_interior);
		glVertex2f(grid_circle[circular_index][0] * radius_exterior, grid_circle[circular_index][1] * radius_exterior);
		circular_index = ((circular_index + 1) % loop_counter);
	}
	glEnd();

	glBegin(GL_LINES); // coordinate axis lines
	glColor4ub(RGB888_R(BLUE), RGB888_G(BLUE), RGB888_B(BLUE), 255); // x-axis
	glVertex2f(-radius_exterior - axis_extension, 0.0f);
	glVertex2f(radius_exterior + axis_extension, 0.0f);
	glColor4ub(RGB888_R(GREEN), RGB888_G(GREEN), RGB888_B(GREEN), 255); // y-axis
	glVertex2f(0.0f, -radius_exterior - axis_extension);
	glVertex2f(0.0f, radius_exterior + axis_extension);
	glEnd();

	glBegin(GL_POINTS); // coordinate system origin
	glColor4ub(RGB888_R(WHITE), RGB888_G(WHITE), RGB888_B(WHITE), 255);
	glVertex2f(0.0f, 0.0f);
	glEnd();
}

void render_ortho_frame(HDC hdc) {
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	switch (axis_direction) {
	case (X_AXIS_RIGHT | Y_AXIS_DOWN):
		gluOrtho2D( // y-down, x-right
				0.0f - ((GLdouble) frame_width * frame_sx) / 2 + frame_ox, // left
				((GLdouble) frame_width * frame_sx) / 2 + frame_ox, // right
				((GLdouble) frame_height * frame_sx) / 2 + frame_oy, // bottom
				0.0f - ((GLdouble) frame_height * frame_sx) / 2 + frame_oy); // top
		break;
	case (X_AXIS_RIGHT | Y_AXIS_UP):
		gluOrtho2D( // y-up, x-right
				0.0f - ((GLdouble) frame_width * frame_sx) / 2 + frame_ox, // left
				((GLdouble) frame_width * frame_sx) / 2 + frame_ox, // right
				0.0f - ((GLdouble) frame_height * frame_sx) / 2 + frame_oy, // bottom
				((GLdouble) frame_height * frame_sx) / 2 + frame_oy); // top
		break;
	case (X_AXIS_LEFT | Y_AXIS_DOWN):
		gluOrtho2D( // y-down, x-left
				((GLdouble) frame_width * frame_sx) / 2 + frame_ox, // left
				0.0f - ((GLdouble) frame_width * frame_sx) / 2 + frame_ox, // right
				((GLdouble) frame_height * frame_sx) / 2 + frame_oy, // bottom
				0.0f - ((GLdouble) frame_height * frame_sx) / 2 + frame_oy); // top
		break;
	case (X_AXIS_LEFT | Y_AXIS_UP):
		gluOrtho2D( // y-up, x-left
				((GLdouble) frame_width * frame_sx) / 2 + frame_ox, // left
				0.0f - ((GLdouble) frame_width * frame_sx) / 2 + frame_ox, // right
				0.0f - ((GLdouble) frame_height * frame_sx) / 2 + frame_oy, // bottom
				((GLdouble) frame_height * frame_sx) / 2 + frame_oy); // top
		break;
	}

	draw_ortho_gridlines(320, 240, 1);
	glcd_demo();
	if (ortho_frame_alter == FRAME_ALTER_ROTATION)
		draw_circle(mouse_x, mouse_y, 2, ILI9341_WHITE);

	SwapBuffers(hdc);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void adjust_mouse_coordinates(int *mouse_x, int *mouse_y) {
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

	*mouse_x = *mouse_x * frame_sx + frame_ox;
	*mouse_y = *mouse_y * frame_sy + frame_oy;
}

int compute_percentage(int part, int total) { // returns percent value of "part" for "total"
//	return (float) (part * total / 100.0f);
	return (int) (total * part / 100);
}

void setPixelFormat(HDC hdc) {
	int pixelFormat;

	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), /* size */
	1, /* version */
	PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER, /* support double-buffering */
	PFD_TYPE_RGBA, /* color type */
	16, /* prefered color depth */
	0, 0, 0, 0, 0, 0, /* color bits (ignored) */
	0, /* no alpha buffer */
	0, /* alpha bits (ignored) */
	0, /* no accumulation buffer */
	0, 0, 0, 0, /* accum bits (ignored) */
	16, /* depth buffer */
	0, /* no stencil buffer */
	0, /* no auxiliary buffers */
	PFD_MAIN_PLANE, /* main layer */
	0, /* reserved */
	0, 0, 0, /* no layer, visible, damage masks */
	};

	pixelFormat = ChoosePixelFormat(hdc, &pfd);
	if (pixelFormat == 0) {
		MessageBox(WindowFromDC(hdc), "ChoosePixelFormat failed.", "Error", MB_ICONERROR | MB_OK);
		exit(1);
	}

	if (SetPixelFormat(hdc, pixelFormat, &pfd) != TRUE) {
		MessageBox(WindowFromDC(hdc), "SetPixelFormat failed.", "Error", MB_ICONERROR | MB_OK);
		exit(1);
	}
}

LRESULT CALLBACK WndBitmapProc(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam) {
	RECT wnd_rect;

	switch (message) {
	case WM_CREATE:
		break;
	case WM_SIZE:
		GetClientRect(hBitmapWindow, &wnd_rect);
		SetWindowPos(hWndEdit, NULL,
				0, 0,
				wnd_rect.right - wnd_rect.left,
				wnd_rect.bottom - wnd_rect.top,
				SWP_NOMOVE | SWP_NOACTIVATE);
		break;
	case WM_KEYDOWN:
		switch (LOWORD(wParam)) {
		case VK_SHIFT:
			break;
		case VK_ESCAPE:
			DestroyWindow(hBitmapWindow);
		}
		break;
	case WM_ERASEBKGND:
		break; // no background repaint
	case WM_QUIT:
	case WM_CLOSE:
		DestroyWindow(hBitmapWindow);
		return 0;
	case WM_DESTROY:
		return 0;
	}

	return DefWindowProc(hWindow, message, wParam, lParam);
}

LRESULT CALLBACK WndMainProc(HWND hWindow, UINT message, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;

	switch (message) {
	case WM_COMMAND: // menu items
		if (LOWORD(wParam) == 1001) { // Load Bitmap
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWindow;
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn) == TRUE) {
				//MessageBoxA(hWindow, ofn.lpstrFile, "CAPTION", MB_OK);
				// use ofn.lpstrFile
			}
		}
		break;
	case WM_CONTEXTMENU: // right mouse button menu
//		MessageBoxA(hWindow, "CONTEXT MENU", "CAPTION", MB_OK);
		break;
	case WM_CREATE:
		hFont = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, // FW_NORMAL, FW_MEDIUM, FW_SEMIBOLD, FW_BOLD
				ANSI_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
				ANTIALIASED_QUALITY, // DEFAULT_QUALITY, ANTIALIASED_QUALITY, PROOF_QUALITY, CLEARTYPE_QUALITY, DRAFT_QUALITY
				VARIABLE_PITCH, "Tahoma"); // Tahoma, Fixedsys, Lucida Console, Courier New, Consolas

		hdc = GetDC(hWindow);
		SelectObject(hdc, hFont);
		setPixelFormat(hdc);
		hglrc = wglCreateContext(hdc);
		if (hglrc == NULL) {
			MessageBox(WindowFromDC(hdc), "wglCreateContext failed.", "Error", MB_ICONERROR | MB_OK);
			exit(1);
		}
		wglMakeCurrent(hdc, hglrc);
		wglUseFontBitmaps(hdc, 0, 255, fontListBase);
		ReleaseDC(hWindow, hdc);
		glEnable(GL_BLEND); // enable transparency
		glDisable(GL_CULL_FACE);
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_POINT_SMOOTH);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // best for points and lines
		//glBlendFunc(GL_SRC_ALPHA_SATURATE, GL_ONE); // best for polygons
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glDisable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glDisable(GL_POLYGON_SMOOTH);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		break;
	case WM_SIZE:
//		frame_width = LOWORD(lParam);
//		frame_height = HIWORD(lParam);
		frame_width = compute_percentage(85, LOWORD(lParam));
		frame_height = compute_percentage(85, HIWORD(lParam));
		glViewport( //
				LOWORD(lParam) / 2 - (frame_width / 2), //
				HIWORD(lParam) / 2 - (frame_height / 2), //
				frame_width, //
				frame_height); //
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		SendMessage(hStatusBar, SB_SETPARTS, 3, (LPARAM) iStatusWidths);
		SendMessage(hStatusBar, WM_SIZE, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWindow, &ps);
		if (hglrc) {
			render_ortho_frame(hdc);
		}
		EndPaint(hWindow, &ps);
		return 0;
	case WM_MOUSEMOVE:
		mouse_x = LOWORD(lParam);
		mouse_y = HIWORD(lParam);

		mouse_dx = mouse_px - mouse_x;
		mouse_dy = mouse_py - mouse_y;
		mouse_px = mouse_x;
		mouse_py = mouse_y;

		adjust_mouse_coordinates(&mouse_x, &mouse_y);
		if (ortho_frame_alter == FRAME_ALTER_ROTATION) {

		}

		if (ortho_frame_alter == FRAME_ALTER_DISTANCE) {
			frame_sx += (float) (mouse_dx + mouse_dy) / 350;
			frame_sy += (float) (mouse_dx + mouse_dy) / 350;
		}

		if (ortho_frame_alter == FRAME_ALTER_POSITION) {
			frame_ox += (GLdouble) mouse_dx;
			frame_oy += (GLdouble) mouse_dy;
		}

		if (wParam == MK_LBUTTON) {
			if (gui_drag_component) {
				gui_perform_drag_component(true);
			}
		}
		if (wParam == MK_RBUTTON) {

		}
		if (wParam == MK_MBUTTON) {

		}
		break;
	case WM_LBUTTONDOWN:
		ortho_frame_alter = FRAME_ALTER_ROTATION;
		gui_begin_drag_component();
		gui_process_events();
		break;
	case WM_LBUTTONUP:
		ortho_frame_alter = FRAME_NO_ALTER;
		gui_end_drag_component();
		break;
	case WM_RBUTTONDOWN:
		ortho_frame_alter = FRAME_ALTER_DISTANCE;
		break;
	case WM_RBUTTONUP:
		ortho_frame_alter = FRAME_NO_ALTER;
		break;
	case WM_MBUTTONDOWN:
		ortho_frame_alter = FRAME_ALTER_POSITION;
		break;
	case WM_MBUTTONUP:
		ortho_frame_alter = FRAME_NO_ALTER;
		break;
	case WM_SYSKEYDOWN: // ALT + key
		break;
	case WM_KEYDOWN:
		switch (LOWORD(wParam)) {
		case VK_SHIFT:
			break;
		case VK_ESCAPE:
			DestroyWindow(hWindow);
			break;
		case VK_LEFT: // arrow left
			h_arrows_key--;
			break;
		case VK_RIGHT: // arrow right
			h_arrows_key++;
			break;
		case VK_UP: // arrow up
			v_arrows_key--;
			break;
		case VK_DOWN: // arrow down
			v_arrows_key++;
			break;
		case VK_PRIOR: // page up
			pages_key++;
			break;
		case VK_NEXT: // page down
			pages_key--;
			break;
		case 0x57: // w
			ws_key++;
			break;
		case 0x41: // a
			ad_key--;
			break;
		case 0x53: // s
			ws_key--;
			break;
		case 0x44: // d
			ad_key++;
			break;
		case 0x51: // q
			qe_key--;
			break;
		case 0x45: // e
			qe_key++;
			break;
		case 0x52: // r
			rf_key++;
			break;
		case 0x46: // f
			rf_key--;
			break;
		case 0x30: // 0
			axis_direction = X_AXIS_RIGHT | Y_AXIS_DOWN;
			break;
		case 0x31: // 1
			frame_sx = 1.0f;
			frame_sy = 1.0f;
			break;
		case 0x32: // 2
			frame_ox = 0.0f;
			frame_oy = 0.0f;
			break;
		case 0x33: // 3
			break;
		case 0x34: // 4
			break;
		case 0x35: // 5
			break;
		case 0x36: // 6
			break;
		case 0x37: // 7
			axis_direction = X_AXIS_LEFT | Y_AXIS_UP;
			break;
		case 0x38: // 8
			axis_direction = X_AXIS_LEFT | Y_AXIS_DOWN;
			break;
		case 0x39: // 9
			axis_direction = X_AXIS_RIGHT | Y_AXIS_UP;
			break;
		case VK_OEM_PERIOD: // .
			if (GetKeyState(VK_RSHIFT) < 0) {
				comma_period_key++;
//				gui.pager.page++;
//				if (gui.pager.page >= gui.pager.page_max)
//					gui.pager.page = gui.pager.page_min;
				event_pager_right_button();
			}
			break;
		case VK_OEM_COMMA: // ,
			if (GetKeyState(VK_RSHIFT) < 0) {
				comma_period_key--;
//				if (gui.pager.page <= gui.pager.page_min)
//					gui.pager.page = gui.pager.page_max;
//				gui.pager.page--;
				event_pager_left_button();
			}
			break;
		}
		break;
	case WM_ERASEBKGND:
		return TRUE; // no background repaint
	case WM_QUIT:
	case WM_CLOSE:
		DestroyWindow(hWindow);
		return 0;
	case WM_DESTROY:
		DeleteObject(hFont);
		if (hglrc) {
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hglrc);
		}
		hdc = GetDC(hWindow);
		ReleaseDC(hWindow, hdc);
		DeleteObject(hFont);
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWindow, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG message;
	WNDCLASSEX wndcex;
	BOOL run = TRUE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	RECT winRect = { 0, 0, 960 + 1 - 10, 540 + 1 + 50};
	RECT winBitmapRect = { 0, 0, 480, 640};

	ZeroMemory(&wndcex, sizeof(wndcex));
	wndcex.cbSize = sizeof(WNDCLASSEX);
	wndcex.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wndcex.lpfnWndProc = WndMainProc;
	wndcex.cbClsExtra = 0;
	wndcex.cbWndExtra = 0;
	wndcex.hInstance = hInstance;
	wndcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndcex.hbrBackground = (HBRUSH) GetStockObject(DKGRAY_BRUSH);
	wndcex.lpszMenuName = NULL;
	wndcex.lpszClassName = szWndClassName;
	wndcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wndcex)) {
		MessageBox(NULL, "Failed to register main window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	wndcex.lpfnWndProc = WndBitmapProc;
	wndcex.lpszClassName = szWndBitmapClassName;

	if (!RegisterClassEx(&wndcex)) {
		MessageBox(NULL, "Failed to register bitmap window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hMainMenu = CreateMenu();
	hFileMenu = CreatePopupMenu();
	hViewMenu = CreatePopupMenu();

	AppendMenu(hMainMenu, MF_STRING | MF_POPUP, (UINT_PTR) hFileMenu, "File");
	AppendMenu(hFileMenu, MF_STRING, 1001, "Load Bitmap");
	AppendMenu(hFileMenu, MF_STRING, 1002, "Exit");

	AppendMenu(hMainMenu, MF_STRING | MF_POPUP, (UINT_PTR) hViewMenu, "View");
	AppendMenu(hViewMenu, MF_STRING, 1003, "item 1");
	AppendMenu(hViewMenu, MF_STRING, 1004, "item 2");
	AppendMenu(hViewMenu, MF_STRING, 1005, "item 3");
	AppendMenu(hViewMenu, MF_SEPARATOR, 0, "");
	AppendMenu(hViewMenu, MF_STRING, 1006, "item 4");

	AdjustWindowRectEx(&winRect, dwStyle, TRUE, dwExStyle);
	hMainWindow = CreateWindowEx(dwExStyle, //
			szWndClassName, //
			szApplicationName, //
			dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, //
			winRect.right - winRect.left, winRect.bottom - winRect.top, //
			NULL, //
			hMainMenu, hInstance, NULL);

	if (hMainWindow == NULL) {
		MessageBox(NULL, "Failed to create main window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	AdjustWindowRectEx(&winBitmapRect, dwStyle, FALSE, dwExStyle);
	hBitmapWindow = CreateWindowEx(dwExStyle, //
			szWndBitmapClassName, //
			szApplicationName, //
			dwStyle,
			CW_USEDEFAULT, CW_USEDEFAULT, //
			winBitmapRect.right - winBitmapRect.left,
			winBitmapRect.bottom - winBitmapRect.top, //
			hMainWindow, //
			NULL, hInstance, NULL);

	if (hBitmapWindow == NULL) {
		MessageBox(NULL, "Failed to create bitmap window!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

//	HWND hWndEdit = CreateWindowA("EDIT", //
//			NULL, //
//			WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,//
//			10, 10, winBitmapRect.right - winBitmapRect.left - 50, //
//			winBitmapRect.bottom - winBitmapRect.top - 70,//
//			hBitmapWindow,//
//			NULL,//
//			(HINSTANCE)GetWindowLongPtr(hBitmapWindow, GWLP_HINSTANCE),//
//			NULL);

	GetClientRect(hBitmapWindow, &winBitmapRect);
	hWndEdit = CreateWindowEx(WS_EX_TOPMOST | WS_EX_OVERLAPPEDWINDOW, //
			"EDIT", //
			NULL, //
			WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, //
			CW_USEDEFAULT, CW_USEDEFAULT, //
			winBitmapRect.right - winBitmapRect.left, //
			winBitmapRect.bottom - winBitmapRect.top, //
			hBitmapWindow, //
			NULL, hInstance, NULL);

	SetWindowText( hWndEdit, "TEXT\r\n TEXT\r\n TEXT\r" );

	hStatusBar = CreateWindowEx(0,                              // расширенный стиль окна
			STATUSCLASSNAME,                                    // класс окна для Statusbar
			"",                                                 // заголовок окна отсутствует
			WS_CHILD | WS_BORDER | WS_VISIBLE | SBARS_SIZEGRIP, // стиль окна
			0, 0, 0, 0,                                         // координаты, ширина, высота
			hMainWindow,                                        // идентификатор родительского окна
			(HMENU) IDS_STATUSBAR,                              // идентификатор Statusbar
			hInstance,                                          // идентификатор приложения
			NULL);                                              // доп. данные для окна

	if (hStatusBar == NULL) {
		MessageBox(NULL, "Failed to create status bar", "Error", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	SendMessage(hStatusBar, SB_SETPARTS, 3, (LPARAM) iStatusWidths);
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM) "Status Bar");
	SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM) "Cells");
	SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM) "Statusbar Text");
	ShowWindow(hStatusBar, SW_SHOW); // SW_HIDE

	ShowWindow(hMainWindow, nCmdShow);
	UpdateWindow(hMainWindow);

	while (run) {
		switch (MsgWaitForMultipleObjectsEx(0, NULL, 5, QS_ALLINPUT, 0)) { // time-out interval, in milliseconds
		case WAIT_OBJECT_0:
			while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
				if (message.message == WM_QUIT) {
					run = FALSE;
				} else {
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
			}
			break;
		case WAIT_TIMEOUT:
			InvalidateRect(hMainWindow, 0, TRUE);
		}
	}

	return EXIT_SUCCESS;
}
