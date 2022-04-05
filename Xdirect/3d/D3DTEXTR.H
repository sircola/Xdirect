//-----------------------------------------------------------------------------
// File: D3DTextr.h
//
// Desc: Functions to manage textures, including creating (loading from a
//	 file), restoring lost surfaces, invalidating, and destroying.
//
//	 Note: the implementation of these fucntions maintain an internal list
//	 of loaded textures. After creation, individual textures are referenced
//	 via their ASCII names.
//
//
// Copyright (C) 1997 Microsoft Corporation. All rights reserved
//-----------------------------------------------------------------------------

#ifndef D3DTEXTR_H
#define D3DTEXTR_H

#include <ddraw.h>
#include <d3d.h>




//-----------------------------------------------------------------------------
// Access functions for loaded textures. Note: these functions search
// an internal list of the textures, and use the texture associated with the
// ASCII name.
//-----------------------------------------------------------------------------
LPDIRECTDRAWSURFACE4 D3DTextr_GetSurface( int id  );
LPDIRECT3DTEXTURE2   D3DTextr_GetTexture( int id  );




//-----------------------------------------------------------------------------
// Texture invalidation and restoration functions
//-----------------------------------------------------------------------------
HRESULT D3DTextr_Invalidate( int id  );
HRESULT D3DTextr_Restore( int id, LPDIRECT3DDEVICE3 pd3dDevice );
HRESULT D3DTextr_InvalidateAllTextures();
HRESULT D3DTextr_RestoreAllTextures( LPDIRECT3DDEVICE3 pd3dDevice );




//-----------------------------------------------------------------------------
// Texture creation and deletion functions
//-----------------------------------------------------------------------------
#define D3DTEXTR_TRANSPARENTWHITE 0x00000001
#define D3DTEXTR_TRANSPARENTBLACK 0x00000002
#define D3DTEXTR_32BITSPERPIXEL   0x00000004
#define D3DTEXTR_CHROMAKEY	  0x00000008

HRESULT D3DTextr_CreateTexture( int id, UCHAR *spr,
					DWORD dwStage=0L, DWORD dwFlags=0L );
HRESULT D3DTextr_DestroyTexture( int id );
VOID	D3DTextr_SetTexturePath( TCHAR* strTexturePath );




#endif // D3DTEXTR_H

