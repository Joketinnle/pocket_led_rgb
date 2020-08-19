#include "includes.h"


void hsv2rgb(hsv_t* hsv, led_bright_t* rgb)
{
	float r, g, b;
	
	float h = (float)(hsv->h / 360.0f);
	float s = (float)(hsv->s / 100.0f);
	float v = (float)(hsv->v / 100.0f);
	
	int i = floor(h * 6);
	float f = h * 6 - i;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);
	
	switch (i % 6) {
		case 0: r = v, g = t, b = p; break;
		case 1: r = q, g = v, b = p; break;
		case 2: r = p, g = v, b = t; break;
		case 3: r = p, g = q, b = v; break;
		case 4: r = t, g = p, b = v; break;
		case 5: r = v, g = p, b = q; break;
	}
	
	rgb->r = r * 255;
	rgb->g = g * 255;
	rgb->b = b * 255;
}


