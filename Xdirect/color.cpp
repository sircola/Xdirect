/* Copyright (C,  255}, 1997 Kirschner, Bern t. All Rights Reserved Worldwide. */

#include <stdio.h>
#include <string.h>

#include <xlib.h>

RCSID( "$Id: color.cpp,v 1.0 2012/07/12 14:20:31 bernie Exp $" )


typedef struct color_s {
	char name[100];
	rgb_t rgb;
} color_t, *color_ptr;


color_t color[] = {
	{"aliceblue",240, 248, 255,  255},
	{"antiquewhite",250, 235, 215,  255},
	{"aqua", 0, 255, 255,  255},
	{"aquamarine",127, 255, 212,  255},
	{"azure",240, 255, 255,  255},
	{"beige",245, 245, 220,  255},
	{"bisque",255, 228, 196,  255},
	{"black", 0, 0, 0,  255},
	{"blanchedalmond",255, 235, 205,  255},
	{"blue", 0, 0, 255,  255},
	{"blueviolet",138, 43, 226,  255},
	{"brown",165, 42, 42,  255},
	{"burlywood",222, 184, 135,  255},
	{"cadetblue", 95, 158, 160,  255},
	{"chartreuse",127, 255, 0,  255},
	{"chocolate",210, 105, 30,  255},
	{"coral",255, 127, 80,  255},
	{"cornflowerblue",100, 149, 237,  255},
	{"cornsilk",255, 248, 220,  255},
	{"crimson",220, 20, 60,  255},
	{"cyan", 0, 255, 255,  255},
	{"darkblue", 0, 0, 139,  255},
	{"darkcyan", 0, 139, 139,  255},
	{"darkgoldenrod",184, 134, 11,  255},
	{"darkgray",169, 169, 169,  255},
	{"darkgreen", 0, 100, 0,  255},
	{"darkgrey",169, 169, 169,  255},
	{"darkkhaki",189, 183, 107,  255},
	{"darkmagenta",139, 0, 139,  255},
	{"darkolivegreen", 85, 107, 47,  255},
	{"darkorange",255, 140, 0,  255},
	{"darkorchid",153, 50, 204,  255},
	{"darkred",139, 0, 0,  255},
	{"darksalmon",233, 150, 122,  255},
	{"darkseagreen",143, 188, 143,  255},
	{"darkslateblue", 72, 61, 139,  255},
	{"darkslategray", 47, 79, 79,  255},
	{"darkslategrey", 47, 79, 79,  255},
	{"darkturquoise", 0, 206, 209,  255},
	{"darkviolet",148, 0, 211,  255},
	{"deeppink",255, 20, 147,  255},
	{"deepskyblue", 0, 191, 255,  255},
	{"dimgray",105, 105, 105,  255},
	{"dimgrey",105, 105, 105,  255},
	{"dodgerblue", 30, 144, 255,  255},
	{"firebrick",178, 34, 34,  255},
	{"floralwhite",255, 250, 240,  255},
	{"forestgreen", 34, 139, 34,  255},
	{"fuchsia",255, 0, 255,  255},
	{"gainsboro",220, 220, 220,  255},
	{"ghostwhite",248, 248, 255,  255},
	{"gold",255, 215, 0,  255},
	{"goldenrod",218, 165, 32,  255},
	{"gray",128, 128, 128,  255},
	{"grey",128, 128, 128,  255},
	{"green", 0, 128, 0,  255},
	{"greenyellow",173, 255, 47,  255},
	{"honeydew",240, 255, 240,  255},
	{"hotpink",255, 105, 180,  255},
	{"indianred",205, 92, 92,  255},
	{"indigo", 75, 0, 130,  255},
	{"ivory",255, 255, 240,  255},
	{"khaki",240, 230, 140,  255},
	{"lavender",230, 230, 250,  255},
	{"lavenderblush",255, 240, 245,  255},
	{"lawngreen",124, 252, 0,  255},
	{"lemonchiffon",255, 250, 205,  255},
	{"lightblue",173, 216, 230,  255},
	{"lightcoral",240, 128, 128,  255},
	{"lightcyan",224, 255, 255,  255},
	{"lightgoldenrodyellow",250, 250, 210,  255},
	{"lightgray",211, 211, 211,  255},
	{"lightgreen",144, 238, 144,  255},
	{"lightgrey",211, 211, 211,  255},
	{"lightpink",255, 182, 193,  255},
	{"lightsalmon",255, 160, 122,  255},
	{"lightseagreen", 32, 178, 170,  255},
	{"lightskyblue",135, 206, 250,  255},
	{"lightslategray",119, 136, 153,  255},
	{"lightslategrey",119, 136, 153,  255},
	{"lightsteelblue",176, 196, 222,  255},
	{"lightyellow",255, 255, 224,  255},
	{"lime", 0, 255, 0,  255},
	{"limegreen", 50, 205, 50,  255},
	{"linen",250, 240, 230,  255},
	{"magenta",255, 0, 255,  255},
	{"maroon",128, 0, 0,  255},
	{"mediumaquamarine",102, 205, 170,  255},
	{"mediumblue", 0, 0, 205,  255},
	{"mediumorchid",186, 85, 211,  255},
	{"mediumpurple",147, 112, 219,  255},
	{"mediumseagreen", 60, 179, 113,  255},
	{"mediumslateblue",123, 104, 238,  255},
	{"mediumspringgreen", 0, 250, 154,  255},
	{"mediumturquoise", 72, 209, 204,  255},
	{"mediumvioletred",199, 21, 133,  255},
	{"midnightblue", 25, 25, 112,  255},
	{"mintcream",245, 255, 250,  255},
	{"mistyrose",255, 228, 225,  255},
	{"moccasin",255, 228, 181,  255},
	{"navajowhite",255, 222, 173,  255},
	{"navy", 0, 0, 128,  255},
	{"oldlace",253, 245, 230,  255},
	{"olive",128, 128, 0,  255},
	{"olivedrab",107, 142, 35,  255},
	{"orange",255, 165, 0,  255},
	{"orangered",255, 69, 0,  255},
	{"orchid",218, 112, 214,  255},
	{"palegoldenrod",238, 232, 170,  255},
	{"palegreen",152, 251, 152,  255},
	{"paleturquoise",175, 238, 238,  255},
	{"palevioletred",219, 112, 147,  255},
	{"papayawhip",255, 239, 213,  255},
	{"peachpuff",255, 218, 185,  255},
	{"peru",205, 133, 63,  255},
	{"pink",255, 192, 203,  255},
	{"plum",221, 160, 221,  255},
	{"powderblue",176, 224, 230,  255},
	{"purple",128, 0, 128,  255},
	{"red",255, 0, 0,  255},
	{"rosybrown",188, 143, 143,  255},
	{"royalblue", 65, 105, 225,  255},
	{"saddlebrown",139, 69, 19,  255},
	{"salmon",250, 128, 114,  255},
	{"sandybrown",244, 164, 96,  255},
	{"seagreen", 46, 139, 87,  255},
	{"seashell",255, 245, 238,  255},
	{"sienna",160, 82, 45,  255},
	{"silver",192, 192, 192,  255},
	{"skyblue",135, 206, 235,  255},
	{"slateblue",106, 90, 205,  255},
	{"slategray",112, 128, 144,  255},
	{"slategrey",112, 128, 144,  255},
	{"snow",255, 250, 250,  255},
	{"springgreen", 0, 255, 127,  255},
	{"steelblue", 70, 130, 180,  255},
	{"tan",210, 180, 140,  255},
	{"teal", 0, 128, 128,  255},
	{"thistle",216, 191, 216,  255},
	{"tomato",255, 99, 71,  255},
	{"turquoise", 64, 224, 208,  255},
	{"violet",238, 130, 238,  255},
	{"wheat",245, 222, 179,  255},
	{"white",255, 255, 255,  255},
	{"whitesmoke",245, 245, 245,  255},
	{"yellow",255, 255, 0,  255},
	{"yellowgreen",154, 205, 50,  255},
};


//
// 147 darab szín
//
rgb_t *GetColorRGB( char *name, int pos ) {

	for( int i=0; i<dimof(color) && name; i++ )
		if( !stricmp(color[i].name, name) )
			return &(color[i].rgb);

	if( pos < 0 || pos >= dimof(color) )
		return NULL;

	return &(color[pos].rgb);
}


//
//
//
char *GetColorName( int pos ) {

	if( pos < 0 || pos >= dimof(color) )
		return NULL;

	return color[pos].name;
}



enum blendmethod_e {
	BLEND_COMPOSITE=0,		//!< Color A is composited onto B (Taking A's alpha into account)
	BLEND_STRAIGHT,			//!< Straight linear interpolation from A->B (Alpha ignored)
	BLEND_ONTO,			//!< Similar to BLEND_COMPOSITE, except that B's alpha is maintained
	BLEND_STRAIGHT_ONTO,		//!< \deprecated \writeme
	BLEND_BEHIND,			//!< Similar to BLEND_COMPOSITE, except that B is composited onto A.
	BLEND_SCREEN,			//!< \writeme
	BLEND_OVERLAY,			//!< \writeme
	BLEND_HARD_LIGHT,		//!< \writeme
	BLEND_MULTIPLY,			//!< Simple A*B.
	BLEND_DIVIDE,			//!< Simple B/A
	BLEND_ADD,			//!< Simple A+B.
	BLEND_SUBTRACT,			//!< Simple A-B.
	BLEND_DIFFERENCE,		//!< Simple |A-B|.
	BLEND_BRIGHTEN,			//!< If composite is brighter than B, use composite. B otherwise.
	BLEND_DARKEN,			//!< If composite is darker than B, use composite. B otherwise.
	BLEND_COLOR,			//!< Preserves the U and V channels of color A
	BLEND_HUE,			//!< Preserves the angle of the UV vector of color A
	BLEND_SATURATION,		//!< Preserves the magnitude of the UV Vector of color A
	BLEND_LUMINANCE,		//!< Preserves the Y channel of color A
	BLEND_ALPHA_BRIGHTEN,		//!< \deprecated If A is less opaque than B, use A
	BLEND_ALPHA_DARKEN,		//!< \deprecated If A is more opaque than B, use B
	BLEND_ALPHA_OVER,		//!< \deprecated multiply alphas and then straight blends using the amount
};



#define CLR_NONE 0
#define CLR_DEFAULT 0

//
// These are the main HTML color names.  Returns CLR_NONE if a matching HTML color name can't be found.
// Returns CLR_DEFAULT only if aColorName is the word Default.
// AHK util.cpp
//
ULONG ColorNameToBGR( char *aColorName ) {

	if( !aColorName || !*aColorName)    return CLR_NONE;
	if( !stricmp(aColorName, "Black"))  return 0x000000;  // These colors are all in BGR format, not RGB.
	if( !stricmp(aColorName, "Silver")) return 0xC0C0C0;
	if( !stricmp(aColorName, "Gray"))   return 0x808080;
	if( !stricmp(aColorName, "White"))  return 0xFFFFFF;
	if( !stricmp(aColorName, "Maroon")) return 0x000080;
	if( !stricmp(aColorName, "Red"))    return 0x0000FF;
	if( !stricmp(aColorName, "Purple")) return 0x800080;
	if( !stricmp(aColorName, "Fuchsia"))return 0xFF00FF;
	if( !stricmp(aColorName, "Green"))  return 0x008000;
	if( !stricmp(aColorName, "Lime"))   return 0x00FF00;
	if( !stricmp(aColorName, "Olive"))  return 0x008080;
	if( !stricmp(aColorName, "Yellow")) return 0x00FFFF;
	if( !stricmp(aColorName, "Navy"))   return 0x800000;
	if( !stricmp(aColorName, "Blue"))   return 0xFF0000;
	if( !stricmp(aColorName, "Teal"))   return 0x808000;
	if( !stricmp(aColorName, "Aqua"))   return 0xFFFF00;
	if( !stricmp(aColorName, "Default"))return CLR_DEFAULT;

	return CLR_NONE;
}


/***
// Tango Color Palette
// http://en.wikipedia.org/wiki/Tango_Desktop_Project#Palette
IsoBlock.colors = {
        yellow: {light:"#fce94f", medium:"#edd400", dark:"#c4a000"},
        orange: {light:"#fcaf3e", medium:"#f57900", dark:"#ce5c00"},
        brown: {light:"#e9b96e", medium:"#c17d11", dark:"#8f5902"},
        green: {light:"#8ae234", medium:"#73d216", dark:"#4e9a06"},
        blue: {light:"#729fcf", medium:"#3465a4", dark:"#204a87"},
        purple: {light:"#ad7fa8", medium:"#75507b", dark:"#5c3566"},
        red: {light:"#ef2929", medium:"#cc0000", dark:"#a40000"},
        white: {light:"#eeeeec", medium:"#d3d7cf", dark:"#babdb6"},
        black: {light:"#888a85", medium:"#555753", dark:"#2e3436"},
};

// from David at http://stackoverflow.com/a/11508164/142317
function hexToRgb(hex) {

        // strip out "#" if present.
        if (hex[0] == "#") {
                hex = hex.substring(1);
        }

    var bigint = parseInt(hex, 16);
    var r = (bigint >> 16) & 255;
    var g = (bigint >> 8) & 255;
    var b = bigint & 255;

    return r + "," + g + "," + b;
}
***/