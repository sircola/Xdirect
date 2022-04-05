
#ifndef _XLUA_H_INCLUDED
#define _XLUA_H_INCLUDED

/***
extern "C" {
#include <c:\\My Projects\\Lua5.1\\include\\lua.h>
#include <c:\\My Projects\\Lua5.1\\include\\lualib.h>
#include <c:\\My Projects\\Lua5.1\\include\\lauxlib.h>
}
***/

#define LUA_COMPAT_ALL

#include <lua_dyn-5.1.h>
// #include <lua_dyn-5.2.1.h>


#define LUA_PREFIX LuaFunctions.
XLIBDEF lua_All_functions LuaFunctions;

XLIBDEF lua_State *LUA_GetVM( void );

XLIBDEF BOOL LUA_Init( void );
XLIBDEF void LUA_Deinit( void );

XLIBDEF BOOL LUA_LoadFile( char *file );
XLIBDEF BOOL LUA_RunFile( char *file );
XLIBDEF BOOL LUA_RunString( char *str );
XLIBDEF void LUA_CloseFile( void );

#define LUA(file,func) { LUA_RunFile(file); LUA_RunFunc(func); }
#define LUA1(file,func,i) { LUA_RunFile(file); LUA_RunFunc(func,i); }
#define LUA4(file,func,i,j,k,l) { LUA_RunFile(file); LUA_RunFunc(func,i,j,k,l); }

XLIBDEF BOOL LUA_CheckGlobal( char *name );
XLIBDEF FLOAT LUA_GetNumber( char *name );
XLIBDEF BOOL LUA_GetString( char *name, char *s );
XLIBDEF char *LUA_GetResultString( void );
XLIBDEF FLOAT LUA_GetResultNumber( void );

// XLIBDEF BOOL LUA_Render( void );
// XLIBDEF BOOL LUA_HasRender( void );

#ifdef __cplusplus
XLIBDEF BOOL LUA_RunFunc( char *funcname, int param1=0, int param2=0, int param3=0, int param4=0 );
#endif
XLIBDEF BOOL LUA_HasFunc( char *funcname );

#define LUA_HasRender() LUA_HasFunc("Render")
#define LUA_Render() LUA_RunFunc("Render")

#define LUA_HasTimer() LUA_HasFunc("Timer")
#define LUA_Timer() LUA_RunFunc("Timer")

XLIBDEF void LUA_WriteProfileNumber( char *name, FLOAT value );
XLIBDEF FLOAT LUA_ReadProfileNumber( char *name, FLOAT default_value );
XLIBDEF BOOL LUA_DumpFile( lua_State *L, char *filename );

#ifdef __cplusplus
XLIBDEF void LUA_GetState( int *rc=NULL, BOOL *act=NULL );
XLIBDEF BOOL LUA_AddFunc( char *name, void *func, lua_State *vm=NULL );
#endif

// ez mire jó?
#ifdef _DEBUG
#include <stdlib.h> // for abort()
#define LUA_DEBUG_START(luaptr) const int __luaStartStackDepth = lua_gettop(luaptr)
#define LUA_DEBUG_END(luaptr, expectedStackDiff) 						\
	do { 											\
		const int __luaEndStackDepth = lua_gettop(luaptr); 				\
		if ( __luaEndStackDepth-expectedStackDiff != __luaStartStackDepth) { 		\
			xprintf("%s:%d: lua stack difference is %d, expected %d\n", 	\
				__FILE__, __LINE__, __luaEndStackDepth-__luaStartStackDepth, expectedStackDiff); \
			abort(); 								\
		} 										\
	} while (0)
#define LUA_DEBUG_CHECK(luaptr, expectedStackDiff) LUA_DEBUG_END(luaptr, expectedStackDiff)
#else
#define LUA_DEBUG_START(luaptr)
#define LUA_DEBUG_END(luaptr, expectedStackDiff)
#define LUA_DEBUG_CHECK(luaptr, expectedStackDiff)
#endif

#endif
