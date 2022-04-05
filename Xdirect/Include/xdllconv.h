
#ifndef _XDLLCONV_H_INCLUDED
#define _XDLLCONV_H_INCLUDED



#define GFXDLLCALLCONV __cdecl
// WINAPI
// __stdcall
// __cdecl
// #define GFXDLLCALLCONV __attribute__((stdcall))

#ifndef PACKED
#ifdef __GNUC__
// #define PACKED __attribute__((packed))
#define PACKED
#else
#define PACKED
#endif
#endif


#endif




