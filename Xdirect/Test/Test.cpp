/* Copyright (C) 1997-99 Kirschner, Bernát. All Rights Reserved Worldwide. */
/* mailto: bernie@freemail.c3.hu?subject=test.cpp						   */



#include <windows.h>
#include <winsock.h>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <locale.h>
#include <direct.h>

// #include <crtdbg.h>

#include <stdexcept>
#include <cassert>

#include <xlib.h>

#include "build.h"

// #include "mmgr.h"

RCSID( "$Id: Test.cpp,v 1.2 2003/09/22 14:00:01 bernie Exp $" )


#define TEST_VERSION "1.0"

#pragma comment( exestr , "The Test 3D v" TEST_VERSION " (Build: " BUILD_NUM_STR ") (" __DATE__ " " __TIME__ ")")
#pragma message( "Compiling " __FILE__ )
#pragma message( "Last modified on " __TIMESTAMP__ )

// test22 3d test
// test53 box2d
// test33 PutSprite
// test47 Lua SeeR
// test37 TTF font
// test38 Particle
// test43 AVI
// test44 Csillagok + Betû effekt
// test49 Bezier Spline
// test54 fractal height map
// test55 PutPixel
// test56 particle
// test56 libFLAC
// test58 Lua only

#define TESTFUNC test53

extern "C" void TESTFUNC( void );



int usermain( int argc, char *argv[], char *envp[] ) {

 	xprintf( "%s\n", getProgramDesc() );

	winHideSplash();

#define PADFILE "http://gamesfrom21.webs.com/pad_matchme.xml"
// #define PADFILE "file://c:/xdirect/test/local_pad_matchme.xml"
// #define PADFILE "file://guardia/d$/Xdirect/Test/pad_memory.xml"
	CheckVersion( PADFILE, TEST_VERSION );

	if( winShareware() != TRUE )
		return 0;

	TESTFUNC();

	return 0;
}



//
//
//
char *getProgramDesc( void ) {

	static char str[256];

	sprintf( str, "The Test 3D v%s (Build: %d) (%s, %s)", TEST_VERSION, BUILD_NUM, __DATE__, __TIME__ );

	return str;
}


// Try to figure out what endian this machine is using. Note that the test
// below might fail for cross compilation; additionally, multi-byte
// characters are implementation-defined in C preprocessors.

#if (('1234' >> 24) == '1')
#elif (('4321' >> 24) == '1')
  #define BIG_ENDIAN
#else
  #error "Couldn't determine the endianness!"
#endif


/*

' ------------------------------------------------------------------------------
' FILE DESCRIPTION: Event to increase version counter in rc script after succesfull builds.
' ------------------------------------------------------------------------------
'
sub Application_BuildFinish( nNumErrors, nNumWarnings )

	if nNumErrors <= 0 then

		' OPEN THE PROJECTS RESOURCE SCRIPT

		Documents.Open( ActiveProject.Name + ".rc" ), "Text"
		Windows(ActiveProject.Name + ".rc").Active = True

		' SELECT BUILD NUMBER FROM PRODUCTVERSION
		' Find line with PRODUCTVERSION Information

		ActiveDocument.Selection.FindText "PRODUCTVERSION", dsMatchForward + dsMatchFromStart + dsMatchCase

		' Move to eol

		ActiveDocument.Selection.EndOfLine

		' Mark Build Number

		ActiveDocument.Selection.WordLeft 1

		' Store and increment build number in strVersion

		Dim strVersion
		strVersion = ActiveDocument.Selection.Text
		strVersion = strVersion + 1

		' Replace old build number with new

		ActiveDocument.Selection.Delete
		ActiveDocument.Selection.Text = strVersion

		' NOW DO THE SAME FOR FILEVERSION, ProductVersion AND FileVersion
		' START WITH FILEVERSION
		' Find line with FILEVERSION Information

		ActiveDocument.Selection.FindText "FILEVERSION", dsMatchForward + dsMatchFromStart + dsMatchCase

		' Move to eol

		ActiveDocument.Selection.EndOfLine

		' Mark Build Number

		ActiveDocument.Selection.WordLeft 1

		' Replace old build number with new

		ActiveDocument.Selection.Delete
		ActiveDocument.Selection.Text = strVersion

		' THEN UPDATE ProductVersion
		' Find line with ProductVersion Information

		ActiveDocument.Selection.FindText "VALUE ""ProductVersion"",", dsMatchForward + dsMatchFromStart + dsMatchCase

		' Move to eol and then to end of build number

		ActiveDocument.Selection.EndOfLine
		ActiveDocument.Selection.CharLeft dsMove, 3
		ActiveDocument.Selection.WordLeft 1

		' Replace old build number with new

		ActiveDocument.Selection.Delete
		ActiveDocument.Selection.Text = strVersion

		' FINALLY UPDATE FileVersion
		' Find line with FileVersion Information

		ActiveDocument.Selection.FindText "VALUE ""FileVersion"",", dsMatchForward + dsMatchFromStart + dsMatchCase

		' Move to eol and then to end of build number

		ActiveDocument.Selection.EndOfLine
		ActiveDocument.Selection.CharLeft dsMove, 3
		ActiveDocument.Selection.WordLeft 1

		' Replace old build number with new

		ActiveDocument.Selection.Delete
		ActiveDocument.Selection.Text = strVersion

		' CLOSE FILE, BUILD APP AND SAVE ALL DOCUMENTS
		' Close RC file

		ActiveDocument.Close()

		' Save documents

		Documents.SaveAll true

		PrintToOutputWindow "Successfully built " & ActiveProject.Name & "version " & strVersion

	end if

end sub

*/

/*
//
//
//
void test1(void) {

	UCHAR *p;

	SetX();

	LoadFont(DEFAULTFONT);
	SetFontColor(11);

	//WriteString(10,10,"w: %d, h: %d\n",SPRITEW(mouse_bitmap),SPRITEH(mouse_bitmap));

	SetRgbFile("proba.pal");

	SetRgb(255,63,63,63);
	SetRgb(11,63,63,63);

	if(!InitMouse()) Quit("no mouse");
	else printf("mouse ok!");
	LoadSprite( "proba.spr", &p );
	ChangeMouseCursor(p,0,0);
	FREEMEM(p);
	//ShowMouse();

	while(!kbhit()) {
		DrawMouse();
		WriteString(10,10,"\rx: %3d, y: %3d",mousex,mousey);

		if(mousebl) { HideMouse(); Line(CLIPMINX,CLIPMINY,mousex,mousey,11); ShowMouse(); }
		if(mousebr) { HideMouse(); Line(mousex,mousey,CLIPMAXX,CLIPMAXY,0); ShowMouse(); }
	}
	getch();

	ResetX();
	return;
}
*/

/*
//
//
//
void test2(void) {

	SetX();

	InitKeyboard();

	while(!keys[sc_Escape]) printf("\rnum: %d, ascii: %c, name: %s  time: %ld ",scan_code,ascii_code,GetScanName(scan_code),GetTic());

	DeinitKeyboard();

	ResetX();
	return;
}


//
//
//
void test3(void) {

	UCHAR *b,*pal,*s;
	int i=0;

	SetX();

	while(!kbhit()) {

		LoadPcx("proba.pcx",&b);
		SetRgbPcx();

		PutSpriteBlack(rand()%SCREENW,rand()%SCREENH,b);
		printf("\r%d",i++);

		FREEMEM(b);

	}

	ALLOCMEM(pal,256*3);
	GetRgbBuf(pal);
	GetSprite(&s,0,0,SCREENW,SCREENH);
	SaveRaw("screen.raw",s,pal);
	SavePcx("screen.pcx",s,pal);
	FREEMEM(s);
	FREEMEM(pal);

	getch();
	ResetX();

	return;
}


//
//
//
void test4(void) {

	SetX();

	LoadFont(DEFAULTFONT);
	SetFontColor(11);

	WriteString(10,10,"Hello, world!");
	getch();


	ResetX();

	return;
}


//
//
//
void test5(void) {

	static int flag=0,done=0;
	point_t p[3];
	polygon_t poly;

	SetX();

	poly.point = &p[0];
	poly.npoints = 3;

	while(!done) {

		p[0][0] = xrand()%CLIPMAXX;
		p[0][1] = xrand()%CLIPMAXY;
		p[1][0] = xrand()%CLIPMAXX;
		p[1][1] = xrand()%CLIPMAXY;
		p[2][0] = xrand()%CLIPMAXX;
		p[2][1] = xrand()%CLIPMAXY;

		if( (1-flag) )
			FillConvex(&poly,xrand()%255);
		else
			RasterFill(&poly,xrand()%255);

		flag = 1 - flag;

		if( kbhit() ) {
			switch( getch() ) {
				case 'c':
				case 'C':
					Clear(NOCOLOR);
					break;
				default:
					done = TRUE;
					break;
			}
		}
	}

	ResetX();

	return;
}

//
//
//
void test6(void) {

	int i;

	SetX();

	for(i=0;i<CLIPMAXY;i++) {

		HLine(10,CLIPMAXX-20,i,i);

	}

	getch();

	ResetX();

	return;
}



//
//
//
void test10(void) {

	SetX();

	while(!kbhit()) {
		Clear(rand()%256);
		FlipPage();
	}
	getch();

	ResetX();

	return;
}

//
//
//
void test7(void) {

	FIXED i,j,k,l;

	printf("double: %ld\nfloat: %ld\nint: %ld\nshort: %ld\nFIXED: %ld\n",sizeof(double),sizeof(float),sizeof(int),sizeof(short),sizeof(FIXED));

	i = FLOAT_TO_FIXED(2.5);
	j = FLOAT_TO_FIXED(10.5);

	k = fmul(i,j);
	l = fdiv(j,i);

	printf("i: %ld, j: %ld, k: %ld = %.2f\ni/j = %.2f",i,j,k,FIXED_TO_FLOAT(k),FIXED_TO_FLOAT(l));

	printf("sin(45) = %f\n",GetSin(45));


	return;
}

//
//
//
void test11() {

	SetX();

	while(!kbhit()) {

		Circle(rand()%SCREENW,rand()%SCREENH,rand()%100,rand()%255);
		FilledCircle(rand()%SCREENW,rand()%SCREENH,rand()%100,rand()%255);

	}

	getch();

	ResetX();

	return;
}

//
//
//
void test12() {

	char s[80];
	strcpy(s,"haho string...");

	SetX();
	InitKeyboard();
	LoadFont(DEFAULTFONT);
	SetFontColor(11);

	Clear(2);

	//LineInput(10,10,s,"yo bionic",30,TRUE);
	LineInput(10,10,s,s,30,TRUE);

	DeinitKeyboard();

	ResetX();

	printf("s: \"%s\"\n",s);

	return;
}

//
//
//
void test13() {

	SetX();

	while(!kbhit())
		Clear(rand()%255);

	getch();

	ResetX();

	return;
}


//
//
//
void test14(void) {

	SetX();


	while(!kbhit())
		FilledBox(100,100,300,300,rand()%255);

	getch();

	ResetX();

	return;
}


//
//
//
void test15(void) {

	UCHAR *s,*p;

	LoadFile("proba.pal",&p);
	LoadSprite("proba.spr",&s);

	SaveRaw("proba.raw",s,p);

	return;
}


//
//
//
void test16(void) {

#define PROBAFGV(x) (FIXED_TO_FLOAT(GetFixedCos(FLOAT_TO_FIXED((x)))))
//#define PROBAFGV(x) GetCos((x))

	int x,oldx,oldy;
	FLOAT dx,dy,period=3.0,dp=0.02;

	SetX();
	InitKeyboard();

	LoadFont("default.fnt");
	SetFontColor(11);

	SetRgb(11,63,63,63);

	while( !keys[sc_Escape] ) {

		dx = (period * 360.0) / SCREENW;
		dy = (SCREENH / 2) / 1.5;
		WriteString(10,10,"dx: %.2f, dy: %.2f, period: %.2f",dx,dy,period);

		oldx = 0;
		oldy = SCREENH/2 - dy * PROBAFGV(0);

		for( x=0; x<SCREENW; x++ ) {

			Line( oldx,oldy,
				x, SCREENH/2 - dy * PROBAFGV(x*dx) , 11);

			oldx = x;
			oldy = SCREENH/2 - dy * PROBAFGV(x*dx);
		}

		if( keys[sc_Left] ) { period += dp; Clear(0); } // WaitKeyUp(sc_Left);
		if( keys[sc_Right] && period > 0.1) { period -= dp; Clear(0); } // WaitKeyUp(sc_Right);

	}

	DeinitKeyboard();
	ResetX();

	return;
}






//
//
//
void test17(void) {

	point3_t U,V,A;
	matrix_t a,b;

	MakeMatrix( &a,
		-1.0,  1.0,   3.6,  10.0,
		-7.6,  9.0,  16.8,  10.0,
		-8.4,  5.0,  46.8,  10.0 );
//		   0.0,  0.0,	0.0,   1.0 );


	MAKEVECTOR( U, 1.0,0.0,0.0 );
	MAKEVECTOR( V, 0.0,1.0,0.0 );

	DumpVector(U,"U");
	DumpVector(V,"V");

	AddVector(U,V,&A);
	DumpVector(A,"U+V");

	SubVector(U,V,&A);
	DumpVector(A,"U-V");

	//printf("90' = %.2f, cos = %.2f\n",FIXED_TO_FLOAT(GetFixedAcos(GetFixedCos(FLOAT_TO_FIXED(90.0)))),FIXED_TO_FLOAT(GetFixedCos(FLOAT_TO_FIXED(90.0))));
	printf("UV szog: %.2f\n",RAD2FOK(FIXED_TO_FLOAT(AngleVector(U,V))));

	printf("UV tav: %.2f\n",FIXED_TO_FLOAT(DistanceVector(U,V)));



	CrossProduct(V,U,&A);
	DumpVector(A,"UxV");

	NormalVector(V,U,&A);
	DumpVector(A,"norm");

	MAKEVECTOR( U, 1.0,1.0,0.0 );
	printf("³U³: %.2f\n",FIXED_TO_FLOAT(AbsoluteVector(U)));
	NormalizeVector( &U );
	DumpVector(U,"U normalized");

	//CopyMatrix(&a,&b);
	//TransponeMatrix(&b);
	//MulMatrix(&a,&b,&c);

	printf("a:\n");
	//TransponeMatrix(&a);DumpMatrix(a);TransponeMatrix(&a);
	DumpMatrix(a);
	printf("det a = %.2f\n",FIXED_TO_FLOAT(DetMatrix(a)));

	CopyMatrix( a, &b );
	OrthoNormalizeMatrix( &b );
	printf("b: (ortho&norm)\n");
	DumpMatrix(b);



	return;

}




//
//
//
void test18(void) {

	polygon_t poly;
	point_t pp[4],tp[4];
	UCHAR *sprite;

	SetX(640,480,16);

	LoadPcx("proba.pcx",PTR(sprite));

	SetRgbPcx();


	poly.npoints = 4;
	poly.point = pp;

	while( !kbhit() ) {

		pp[0][0] = (CLIPMAXX-CLIPMINX)/2 - xrand()%((CLIPMAXX-CLIPMINX)/2);
		pp[0][1] = (CLIPMAXY-CLIPMINY)/2 - xrand()%((CLIPMAXY-CLIPMINY)/2);
		pp[1][0] = (CLIPMAXX-CLIPMINX)/2 - xrand()%((CLIPMAXX-CLIPMINX)/2);
		pp[1][1] = (CLIPMAXY-CLIPMINY)/2 + xrand()%((CLIPMAXY-CLIPMINY)/2);
		pp[2][0] = (CLIPMAXX-CLIPMINX)/2 + xrand()%((CLIPMAXX-CLIPMINX)/2);
		pp[2][1] = (CLIPMAXY-CLIPMINY)/2 + xrand()%((CLIPMAXY-CLIPMINY)/2);
		pp[3][0] = (CLIPMAXX-CLIPMINX)/2 + xrand()%((CLIPMAXX-CLIPMINX)/2);
		pp[3][1] = (CLIPMAXY-CLIPMINY)/2 - xrand()%((CLIPMAXY-CLIPMINY)/2);

		tp[0][0] = 0;
		tp[0][1] = 0;
		tp[1][0] = 0;
		tp[1][1] = SPRITEH(sprite)-1;
		tp[2][0] = SPRITEW(sprite)-1;
		tp[2][1] = SPRITEH(sprite)-1;
		tp[3][0] = SPRITEW(sprite)-1;
		tp[3][1] = 0;

		WaitRetrace(1);
		//SetBorder( ~NOCOLOR );
		//DrawTexturedPoly( &poly, tp, sprite );
		//SetBorder( NOCOLOR );

		//getch();

	}

	getch();

	ResetX();

	return;
}




//
//
//
void test19(void) {

	SetX();

	LoadFont("default.fnt");
	SetFontColor(11);

	SetRgb(0,0,0,0);
	SetRgb(11,63,63,63);

	WriteString(10,10,"%s: Hello, World!\n",myargv[0]);

	getch();

	ResetX();

	return;
}




//
//
//
void test21(void) {

	int i,x,y;
	BOOL done = FALSE;

	x = winReadProfileInt( "width", 640 );
	y = winReadProfileInt( "height", 480 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	i = 0;
	SETFLAG( i, MF_3DCURSOR );
	if(!InitMouse(i)) xprintf("no mouse!");
	//HideMouse();

	SetRgb(NOCOLOR,0,0,0);
	SetRgb(~NOCOLOR,63,63,63);

	if( myargc < 2 )
		LoadFont(DEFAULTFONT);
	else
		LoadFont(myargv[1]);

	SetFontColor(~NOCOLOR);

	if(myargc<3)
		WriteString(10,10,"Hello, World!\n");
	else
		WriteString(10,10,myargv[2]);

	WriteChar(10+FontWidth()*14,10,'A');
	WriteChar(10+FontWidth()*16,10,'a');

	while( !done ) {

		if( scan_code == sc_Escape ) done = TRUE;

		for( i=0,y=10+(FontHeight()+1)*2; i<256; i++,y+=(FontHeight()+1))

			for( x=0; x<SCREENW-FontWidth()&&i<256; i++,x+=FontWidth() )

				WriteChar(x,y,i);


		DrawMouse();

		FlipPage();
	}

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}



//
//
//
void test23( void ) {

	UCHAR *p,*q,pal[768];
	int w,h,done=0,scale=0,key;
	char *txt;

	if( !winOpenFile( "PSD Files (.psd)\0*.psd\0PCX Files (.pcx)\0.pcx\0BMP Files (.bmp)\0*.bmp\0RAW Files (.raw)\0*.raw\0SPR Files (.spr)\0*.spr\0JPEG Files (.jpg;.jpeg)\0*.jpg;*.jpeg\0TIFF Files (.tif;.tiff)\0*.tif;*.tiff\0ICO Files (.ico)\0*.ico\0PNG Files (.png)\0*.png\0TGA Files (*.tga)\0*.tga\0All Files (*.*)\0*.*\0") )
		Quit( NULL );

	if( (txt = winGetNextFile()) == NULL )
		Quit( "no file" );

	if( LoadPicture(txt, PTR(p) ) == FALSE )
		Quit("cannot load.");

	GetRgbPicture(pal);

//	if( SavePng( "temp.png", p, pal ) == FALSE )
//		Quit("cannot SAVE picture.");

	InitKeyboard();

	SetX(640,480,16,SETX_NORMAL);

	if(!InitMouse(0)) xprintf("no mouse!");
	HideMouse();

	LoadFont(DEFAULTFONT);
	SetFontColor(~NOCOLOR);

	HighSprite( PTR(p), pal );

	w = SPRITEW(p);
	h = SPRITEH(p);

	DupSprite( p, PTR(q) );

	while( !done ) {

		if( (key = GetKey()) == key_Escape ) done = 1;

		if( keys[sc_GreyMinus] ) {
			scale = 1;
			if( --w <= 5 ) w = 5;
			if( --h <= 5 ) h = 5;
		}

		if( keys[sc_GreyPlus] ) {
			scale = 1;
			++w;
			++h;
		}

		if( scale ) {
			FREEMEM( q );
			//ALLOCMEM( q, w*h+SPRITEHEADER );
			ScaleSprite( w,h, p, PTR(q) );
		}

		// PutSprite( (SCREENW-SPRITEW(q))/2, (SCREENH-SPRITEH(q))/2, q );
		PutSprite( mousex, mousey, q );

		WriteString(10,10,"w: %d\nh: %d",w,h);

		if( key == EXT(sc_F2) ) ScreenSave();

		ReadMouse();

		FlipPage();
		Clear(NOCOLOR);
	}

	FREEMEM( p );
	FREEMEM( q );

	ResetX();

	return;
}







//
//
//
void test24( void ) {

	int x,y,color;


	SetX();
	Clear(NOCOLOR);
	SetRgb(NOCOLOR,30,30,30);

	Dither( 16,16, 63,63,63, 0,0,0 );
	Phong( 32,16, 63,63,63, 0,0,0 );
	Phong( 48,16, 0,0,0, 63,63,63 );

	color = 16;
	y = 10;
	for(x=10; x<160; x+=10,color++) {
		Line(x,y+0,x+10,y+0,color);
		Line(x,y+1,x+10,y+1,color);
		Line(x,y+2,x+10,y+2,color);
		Line(x,y+3,x+10,y+3,color);
	}

	color = 32;
	y = 20;
	for(x=10; x<160; x+=10,color++) {
		Line(x,y+0,x+10,y+0,color);
		Line(x,y+1,x+10,y+1,color);
		Line(x,y+2,x+10,y+2,color);
		Line(x,y+3,x+10,y+3,color);
	}

	color = 48;
	y = 30;
	for(x=10; x<160; x+=10,color++) {
		Line(x,y+0,x+10,y+0,color);
		Line(x,y+1,x+10,y+1,color);
		Line(x,y+2,x+10,y+2,color);
		Line(x,y+3,x+10,y+3,color);
	}


	getch();

	ResetX();

	return;
}




//
//
//
void test25( void ) {

	BOOL done = FALSE;
	int key;

	InitKeyboard();

	while( !done ) {

		if( keys[sc_Escape] ) done = TRUE;

		if( KeyHit() ) {

			key = GetKey();

			if( key&EXTKEY )
				printf("\"%s\"",GetScanName(key^EXTKEY));
			else
				printf("%c",key);
		}
	}


	DeinitKeyboard();

	return;
}




//
//
//
void test27(void) {

	int x1,y1,x2,y2,c;

	SetX();

	SetRgb( 0, 0,0,0 );
	SetRgb( 0xff, 63,63,63 );

	while( !kbhit() ) {

		x1 = rand() % SCREENW;
		y1 = rand() % SCREENH;
		x2 = rand() % SCREENW;
		y2 = rand() % SCREENH;
		c = rand() % 255;

		XorLine( x1, y1, x2, y2 );
	}
	getch();

	ResetX();
	return;
}



//
//
//
void test28( void ) {

	int	x,y;				// pixel coordinates
	double	a;				// length of the semi-axis
	double	rho,theta;		// polar coordinates

	double	pi = 3.14159265358979;

	SetX();

	// draws an n-leaved rose of the form  rho = a * cos(n*theta)

#define Leaves		(double)11	// n must be an odd number

#define Xmax		640
#define Ymax		350
#define PixelValue	14
#define ScaleFactor	(double) 1.37

	a = (SCREENH / 2) - 1;		// a reasonable choice for a

	for( theta=0.0; theta < pi; theta+=0.001 ) {

		rho = a * cos( Leaves*theta );	// apply the formula

		x = rho * cos( theta );		// convert to rectangular coords
		y = rho * sin( theta ) / ScaleFactor;

		PutPixel( x+SCREENW/2, y+SCREENH/2, PixelValue );	// plot the point
	}

	getch();

	ResetX();

	return;
}

*/

/*
//
//
//
void test29( void ) {

	UCHAR *spr,*screen;
	BOOL done;
	FLOAT angle = 0.0;
	int render = 0;

	LoadPcx( "death.pcx", PTR(spr) );
	GetRgbPcx(system_rgb);

	MakeAliasColor(FALSE);

	SetX();
	SetRgbPcx();

	ALLOCMEM( screen, SPRITEHEADER+(SCREENW*SCREENH) );
	MKSPR8(screen);
	MKSPRW(screen,SCREENW);
	MKSPRH(screen,SCREENH);

	done = FALSE;

	while( !done ) {

		RotateScaleSprite( spr, 1.0, angle, screen );
		angle += FOK2RAD(2.0);
		if( angle>2*M_PI ) angle -= 2*M_PI;
		if( angle<(-2)*M_PI ) angle += 2*M_PI;

		if( kbhit() )
		switch( getch() ) {

			case '`':
				render = 0;
				break;

			case '1':
				render = 1;
				break;

			case '2':
				render = 2;
				break;

			case '3':
				render = 3;
				break;

			case '4':
				render = 4;
				break;

			case '5':
				render = 5;
				break;

			case 27:
				done = TRUE;
				break;
		}

		switch( render ) {
			case 0:
				break;

			case 1:
				BilinearAliasing( screen );
				break;

			case 2:
				TrilinearAliasing( screen );
				break;

			case 3:
				HyperlinearAliasing( screen );
				break;

			case 4:
				ColorBilinearAliasing( screen );
				break;

			case 5:
				AccurateColorBilinearAliasing( screen );
				break;
		}

		PutSprite((SCREENW-SPRITEW(screen))/2,(SCREENH-SPRITEH(screen))/2,screen);

	}

	FREEMEM( screen );

	ResetX();

	return;
}


//
//
//
void test18(void) {

	polygon_t poly;
	point_t pp[4],tp[4];
	UCHAR *sprite,pal[768];
	int done;

	InitKeyboard();
	SetX(640,480,16,SETX_NORMAL);
	Clear(RGBINT(0,0,0));

	LoadPcx("proba.pcx",PTR(sprite));
	GetRgbPcx( pal );
	SetRgbPcx();

	AddTexMapHigh(sprite,pal,"proba","proba.pcx",0,NOCOLOR);

	poly.npoints = 4;
	poly.point = pp;

	done = 0;
	while( !done ) {

		done = keys[sc_Q];
		done = keys[sc_Escape];

		pp[0][0] = (CLIPMAXX-CLIPMINX)/2 - xrand()%((CLIPMAXX-CLIPMINX)/2);
		pp[0][1] = (CLIPMAXY-CLIPMINY)/2 - xrand()%((CLIPMAXY-CLIPMINY)/2);
		pp[1][0] = (CLIPMAXX-CLIPMINX)/2 - xrand()%((CLIPMAXX-CLIPMINX)/2);
		pp[1][1] = (CLIPMAXY-CLIPMINY)/2 + xrand()%((CLIPMAXY-CLIPMINY)/2);
		pp[2][0] = (CLIPMAXX-CLIPMINX)/2 + xrand()%((CLIPMAXX-CLIPMINX)/2);
		pp[2][1] = (CLIPMAXY-CLIPMINY)/2 + xrand()%((CLIPMAXY-CLIPMINY)/2);
		pp[3][0] = (CLIPMAXX-CLIPMINX)/2 + xrand()%((CLIPMAXX-CLIPMINX)/2);
		pp[3][1] = (CLIPMAXY-CLIPMINY)/2 - xrand()%((CLIPMAXY-CLIPMINY)/2);

		// pp[0][0] = xrand()%(CLIPMAXX-CLIPMINX);
		// pp[0][1] = xrand()%(CLIPMAXY-CLIPMINY);
		// pp[1][0] = xrand()%(CLIPMAXX-CLIPMINX);
		// pp[1][1] = xrand()%(CLIPMAXY-CLIPMINY);
		// pp[2][0] = xrand()%(CLIPMAXX-CLIPMINX);
		// pp[2][1] = xrand()%(CLIPMAXY-CLIPMINY);
		// pp[3][0] = xrand()%(CLIPMAXX-CLIPMINX);
		// pp[3][1] = xrand()%(CLIPMAXY-CLIPMINY);

  		// pp[0][0] = 10;
		// pp[0][1] = 10;
		// pp[1][0] = 10;
		// pp[1][1] = 100;
		// pp[2][0] = 100;
		// pp[2][1] = 100;
		// pp[3][0] = 100;
		// pp[3][1] = 10;

		tp[0][0] = 0;
		tp[0][1] = 0;
		tp[1][0] = 0;
		tp[1][1] = SPRITEH(sprite)-1;
		tp[2][0] = SPRITEW(sprite)-1;
		tp[2][1] = SPRITEH(sprite)-1;
		tp[3][0] = SPRITEW(sprite)-1;
		tp[3][1] = 0;

		//DrawTexturedFaceHigh( &poly, tp, 0, NULL );
		//DrawTexturedPoly( &poly, tp, sprite );

		PutSprite( rand()%SCREENW, rand()%SCREENH, sprite );
		//FillConvex( &poly, RGBINT8( rand()%256 ) );

		FlipPage();
	}

	ResetX();

	return;
}



//
//
//
void test30( void ) {

	int i,key=sc_Right;

	InitCdrom();

	InitKeyboard();
	SetX(640,480,8,SETX_NORMAL);
	InitMouse(0);

	LoadFont( DEFAULTFONT );
	SetFontColor( RGBINT(0xff,0xff,0xff) );

	CD_GetInfo();
	i = StartTrack;

	while( key != key_Escape ) {

		HideMouse();

		Clear( NOCOLOR );

		WriteString( 10,10, "tracks: %d - %d\nplaying %d track...\n",StartTrack,EndTrack,i);

		ShowMouse();

		DrawMouse();

		if( key == sc_Right )
			if( ++i <= EndTrack )
				CD_PlayRange( i, EndTrack );

		if( key == sc_Left )
			if( --i < StartTrack )
				CD_PlayRange( i, EndTrack );


		FlipPage();

		key = GetKey();

		if( key != key_None ) printf("key = 0x%x\n",key);
	}


	ResetX();

	DeinitCdrom();

	return;
}



//
//
//
void test31( void ) {

	// int x,y;
	static char dir[1024],file[1024];

	InitKeyboard();
	SetX(640,480,16,SETX_NORMAL);
	InitMouse(0);
	// HideMouse();


	winGetCurrentDirectory(dir,256);
	sprintf(file,"valami");


	SetRgb(10,20,20,20);
	Clear(RGBINT8(10));
	//for(y=0;y<SCREENH;y++)
	//for(x=0;x<SCREENW;x++) PutPixel(x,y,rand()%256);

//	Selector(dir,file,"*");

	ResetX();

	xprintf("name: \"%s\"\n",file);

	return;
}



void test32( void ) {

	quat_t First = {2.0f,3.0f,4.0f,1.0f},
			Second = {1.0f,4.0f,0.0f,2.0f},
			c,q={0.0f,0.0f,0.0f,20.0f},w={0.0f,0.0f,0.0f,30.0f},
			b = { 666.333f, 12.1f, 22.12f,15.0f},
			a = { 10.0f, 13.0f, 2.0f, 0.5f},
			result;

	DumpQuat( First, "My first Quaternion: ");
	DumpQuat( Second, "My second Quaternion: ");
	MulQuat( First, Second, c );
	DumpQuat( c, "The product of both Quaternions: ");

	MulQuat( w, q, c );
	DumpQuat(c,  "Product of ");


	// demonstrate slerp
	// calculate position in the middle of both quaternions ( t = 0.5 )
	SlerpQuat( a, b, 0.5f, result );
	DumpQuat( result, "Interpolated Quaternion is: ");

	xprintf("A Quaternion describing a rotation of a 90° Angle around the (1, 0, 0) axis:\n");
	AngleAxis2Quat( deg2rad(90.0f), 1.0f, 0.0f, 0.0f, a );
	DumpQuat( a, "a" );
	ExpQuat( a );
	DumpQuat( a, "Exponent of this Quaternion: ");

	ConjugateQuat( First );
	InvertQuat( Second );

	DumpQuat( First, "Conjugate of Quaternion " );
	DumpQuat( Second, "Inverse of Quaternion " );

	return;
}


*/

//
//
//
void test33( void ) {

	BOOL done = FALSE;
	point3_t cam_pos,cam_view;
	FLOAT zclipnear=0.0f,zclipfar=250.0f;
	int frame=0,key,i,texmapid,x,y;
	char *txt;

	polygon_t poly;
	rgb_t rgb[4];
	point_t point[4];
	point3_t st[4];

#define STR1 "BMP Files (*.bmp)\0*.bmp\0JPEG Files (*.jpg)\0*.jpg\0\
		PCX Files (*.pcx)\0*.pcx\0PSD Files (*.psd)\0*.psd\0\
		RAW Files (*.raw)\0*.raw\0SPR Files (*.spr)\0*.spr\0\
		GIF Files (*.gif)\0*.gif\0LBM Files (*.lbm)\0*.lbm\0\
		PNG Files (*.png)\0*.png\0ICO Files (*.ico)\0*.ico\0\
		TGA Files (*.tga)\0*.tga\0TIF Files (*.tif)\0*.tif\0\
		All Files (*.*)\0*.*\0"

	if( !winOpenFile( STR1 ) )
		Quit( NULL );

	if( (txt = winGetNextFile()) == NULL )
		Quit( "no file" );

	x = winReadProfileInt( "xlib_width", 800 );
	y = winReadProfileInt( "xlib_height", 600 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );

	if(!InitMouse(x)) xprintf("no mouse!");
	// HideMouse();

	/*
	UCHAR *spr,pal[768];
	LoadPicture( txt,PTR(spr));
	GetRgbPicture( pal );
	HighSprite( PTR(spr), pal );
	texmapid = AddTexMapHigh( spr, pal, "TEX", txt, 0, NOCOLOR );
	FREEMEM( spr );
	*/

	int flag = 0L;
	int color = 0L;

	// SETFLAG( flag, TF_CHROMA );
	// SETFLAG( flag, TF_ALPHA );

	if( (texmapid = LoadTexmap( txt, flag, color )) == (-1) )
		Quit("Can't load \"%s\" file.",txt);

	/***
	texmap_t *texmap = NULL;
	texmap = TexForNum( texmapid );
	SavePcx( "valami_freeimage.pcx", texmap->sprite1, pic_pal );
	***/

	poly.npoints = 4;
	poly.point = point;

	point[0][0] = 30;	  point[0][1] = 30;
	point[1][0] = 30;	  point[1][1] = SCREENH-30;
	point[2][0] = SCREENW-30; point[2][1] = SCREENH-30;
	point[3][0] = SCREENW-30; point[3][1] = 30;

	st[0][0] = 0.0f;	st[0][1] = 0.0f;
	st[1][0] = 0.0f;	st[1][1] = 255.0f;
	st[2][0] = 255.0f;	st[2][1] = 255.0f;
	st[3][0] = 255.0f;	st[3][1] = 0.0f;

	for( i=0; i<4; i++ ) {
		rgb[i].r = 255;
		rgb[i].g = 255;
		rgb[i].b = 255;
		rgb[i].a = 255;
	}

	MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
	MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );

	done=FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;


		Clear(NOCOLOR);

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		PutSpritePoly( poly, st, texmapid, rgb );

		point[2][0] = mousex;
		point[2][1] = mousey;

		point[3][0] = mousex;
		point[1][1] = mousey;

		PutSpritePoly( poly, st, texmapid, rgb );

		SetFont(1);
		SetFontAlpha( FA_ALL, 255,255,255, 255 );

		WriteString(10,10,"%s\nframe: %d", GetDateStr(),frame++);

		consoleAnimate();

		DrawMouse();

		EndScene();

		if( keys[sc_F2] == TRUE )
			ScreenShot();

		FlipPage();
	}

	DiscardAllTexmap();

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}



//
// fractal height map
//
void test54( void ) {

	BOOL done = FALSE;
	point3_t cam_pos,cam_view;
	FLOAT zclipnear=0.0f,zclipfar=250.0f;
	int frame=0,key,i,texmapid=-1,x,y;

	polygon_t poly;
	rgb_t rgb[4];
	point_t point[4];
	point3_t st[4];

	x = winReadProfileInt( "xlib_width", 800 );
	y = winReadProfileInt( "xlib_height", 600 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );

	if(!InitMouse(x)) xprintf("no mouse!");
	// HideMouse();

	/*
	UCHAR *spr,pal[768];
	LoadPicture( txt,PTR(spr));
	GetRgbPicture( pal );
	HighSprite( PTR(spr), pal );
	texmapid = AddTexMapHigh( spr, pal, "TEX", txt, 0, NOCOLOR );
	FREEMEM( spr );
	*/

	/***
	texmap_t *texmap = NULL;
	texmap = TexForNum( texmapid );
	SavePcx( "valami_freeimage.pcx", texmap->sprite1, pic_pal );
	***/

	poly.npoints = 4;
	poly.point = point;

	point[0][0] = 30;	  point[0][1] = 30;
	point[1][0] = 30;	  point[1][1] = SCREENH-30;
	point[2][0] = SCREENW-30; point[2][1] = SCREENH-30;
	point[3][0] = SCREENW-30; point[3][1] = 30;

	st[0][0] = 0.0f;	st[0][1] = 0.0f;
	st[1][0] = 0.0f;	st[1][1] = 255.0f;
	st[2][0] = 255.0f;	st[2][1] = 255.0f;
	st[3][0] = 255.0f;	st[3][1] = 0.0f;

	for( i=0; i<4; i++ ) {
		rgb[i].r = 255;
		rgb[i].g = 255;
		rgb[i].b = 255;
		rgb[i].a = 255;
	}

	MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
	MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );


	done=FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;


		if( key == ' ' || mousebld|| mousebmd|| mousebrd || (texmapid == -1) ) {

			DiscardTexmap( texmapid );

			int flag,color;
			UCHAR *spr=NULL,*clr=NULL;

			CreateFractalMap( PTR(spr), 1024, PTR(clr) );

			SavePcx( "height.pcx", spr, GetFractalPalette() );
			SavePcx( "colormap.pcx", clr, GetFractalPalette() );

			flag = 0L;
			color = 0L;

#define FRAC_NAME "fractal_texture1"

			texmapid = AddTexMapHigh( spr, GetFractalPalette(), FRAC_NAME, FRAC_NAME, flag, color );

			if( spr ) FREEMEM( spr );
			if( clr ) FREEMEM( clr );
		}

		Clear(NOCOLOR);

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		PutSpritePoly( poly, st, texmapid, rgb );

		point[2][0] = mousex;
		point[2][1] = mousey;

		point[3][0] = mousex;
		point[1][1] = mousey;

		PutSpritePoly( poly, st, texmapid, rgb );

		SetFont(1);
		SetFontAlpha( FA_ALL, 255,255,255, 255 );

		WriteString(10,10,"%s\nframe: %d", GetDateStr(),frame++);

		consoleAnimate();

		DrawMouse();

		EndScene();

		if( keys[sc_F2] == TRUE )
			ScreenShot();

		FlipPage();
	}

	DiscardAllTexmap();

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}






//
// fractal height map
//
void test55( void ) {

	BOOL done = FALSE;
	point3_t cam_pos,cam_view;
	FLOAT zclipnear=0.0f,zclipfar=250.0f;
	int frame=0,key,x,y;
	UCHAR *spr=NULL,*pal=NULL;

	x = winReadProfileInt( "xlib_width", 800 );
	y = winReadProfileInt( "xlib_height", 600 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );

	if(!InitMouse(x)) xprintf("no mouse!");
	// HideMouse();

	MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
	MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );

	done=FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;


		if( key == ' ' || mousebld|| mousebmd|| mousebrd || (spr == NULL) ) {

			FREEMEM( spr );

			CreateFractalMap( PTR(spr), 256 );
			pal = GetFractalPalette();
		}

		Clear(NOCOLOR);

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		for( y=0; y<SPRITEH(spr); y++ )
		for( x=0; x<SPRITEW(spr); x++ ) {
			rgb_t c;
			int b = SPR(spr,x,y);
			c.r = pal[b*3+0] << 2;
			c.g = pal[b*3+1] << 2;
			c.b = pal[b*3+2] << 2;
			c.a = 255;
			PutPixel( mousex+x-(SPRITEW(spr)/2),mousey+y-(SPRITEH(spr)/2), c );
		}

		SetFont(1);
		SetFontAlpha( FA_ALL, 255,255,255, 255 );

		WriteString(10,10,"%s\nframe: %d", GetDateStr(),frame++);

		consoleAnimate();

		DrawMouse();

		EndScene();

		if( keys[sc_F2] == TRUE )
			ScreenShot();

		FlipPage();
	}

	DiscardAllTexmap();

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}








#include <xlua.h>


//
//
//
void test56( void ) {

	BOOL done = FALSE;
	point3_t cam_pos,cam_view;
	ULONG frame=0L,framecnt=0,tics=0L;
	int key,x,y,status_cnt=0;

	tooltip_t mytooltip[] = { { 0,0,190,190, "Enter The Menu" } };
	tooltip_t masiktooltip[] = { { 200,200,490,490, "Masik Tooltip csak nekem" } };

	/***
	x = LUA_ReadProfileNumber( "xlib_width", 800 );
	xprintf("test56: x = %d\n", x );
	x = 128 - (rand() % 256);
	LUA_WriteProfileNumber( "xlib_width", x );
	***/

	x = winReadProfileInt( "xlib_width", 800 );
	y = winReadProfileInt( "xlib_height", 600 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );

	if(!InitMouse(x)) xprintf("no mouse!");
	// HideMouse();

	MAKEVECTOR( cam_view, 0.0, 0.0, 0.0 );
	MAKEVECTOR( cam_pos, 10.0, 10.0, 10.0 );

	DoParticle( TRUE, cam_pos, cam_view );

	done=FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		if( (key == '-') || (mousedz<0) ) {
			SetParticle( -1, -100 );
			StatusText( ST_YELLOW, "%s: %d,   %d\n", GetEmitterName(), GetNumParticle(), status_cnt++ );
		}

		if( (key == '+') || (mousedz>0) ) {
			SetParticle( -1, 100 );
			StatusText( ST_RED, "%s: %d,   %d\n", GetEmitterName(), GetNumParticle(), status_cnt++ );
		}

		     if( key == '0' ) SetParticle( 0, 0 );
		else if( key == '1' ) SetParticle( 1, 0 );
		else if( key == '2' ) SetParticle( 2, 0 );
		else if( key == '3' ) SetParticle( 3, 0 );
		else if( key == '4' ) SetParticle( 4, 0 );
		else if( key == '5' ) SetParticle( 5, 0 );
		else if( key == '6' ) SetParticle( 6, 0 );
		else if( key == '7' ) SetParticle( 7, 0 );
		else if( key == '8' ) SetParticle( 8, 0 );
		else if( key == '9' ) SetParticle( 9, 0 );
		else if( key == 'a' ) SetParticle( 10, 0 );

		if( key == ' ' || mousebld|| mousebmd|| mousebrd ) {

		}

		Clear(NOCOLOR);

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f );

		DoParticle( FALSE, cam_pos, cam_view );

		DrawTooltip( mytooltip, dimof(mytooltip), TRUE );
		DrawTooltip( masiktooltip, dimof(masiktooltip), TRUE );

		SetFont(1);
		SetFontAlpha( FA_ALL, 255,255,255, 255 );

		WriteString(10,10,"%s\nframe/sec: %d\nparticle: \"%s\" emitter, %d\nmouse: %d, %d",
				GetDateStr(),
				frame,
				GetEmitterName(),
				GetNumParticle(),
				mousex,mousey );

		consoleAnimate();

		DrawMouse();

		EndScene();

		if( keys[sc_F2] == TRUE )
			ScreenShot();

		FlipPage();

		MemCheck();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	DiscardAllTexmap();

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}






//
//
//
void test57( void ) {


	UCHAR *read_file;
	int read_file_size;

	if( (read_file_size = LoadFile("rain.flac", PTR(read_file))) ) {

		UCHAR *buf;
		int size;

		xprintf("flac? %s.\n", IsFlac( (char *)read_file, read_file_size )? "yes" : "no" );

		BOOL ok = DecodeFlac(read_file, read_file_size, PTR(buf), &size );

		if( ok ) {
			PushTomFlag();
			TomFlag( NOFLAG );
			XL_WriteFile( "out.wav", buf, size );
			PopTomFlag();

			FREEMEM(buf);
			buf = NULL;
		}

		FREEMEM(read_file);
		read_file = NULL;

		if( (read_file_size = LoadFile("rain-16bit-st.wav", PTR(read_file))) ) {

			ok = EncodeFlac( read_file, read_file_size, PTR(buf), &size );

			if( ok ) {
				PushTomFlag();
				TomFlag( NOFLAG );
				XL_WriteFile( "out.flac", buf, size );
				PopTomFlag();

				FREEMEM(buf);
				buf = NULL;
			}

			FREEMEM(read_file);
			read_file = NULL;
		}
	}

	return;
}




//
//
//
void test58( void ) {

	BOOL done = FALSE;
	int x,y,key,cnt=0,tic;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;
	light_t *sun;

	// Message("Armadillo: %d\n", isArmadillo() );

	x = winReadProfileInt( "xlib_width", -1 );
	y = winReadProfileInt( "xlib_height", -1 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);

	MAKEVECTOR( cam_view, 0.0, 1.0, 0.0 );
	MAKEVECTOR( cam_pos, 0.0, 0.0, 0.0 );

	CV_SetValue( "zclipfar", 150.0f );
	CV_SetValue( "zclipnear", 0.0f );

	sun = AddLight( LT_AMBIENT, 150,150,150,255, 100.0f, NULL, NULL );

	tic = GetTic();

	InitSpeak();

	// char dir[XMAX_PATH];
	// getcwd( dir, XMAX_PATH);
	// xprintf("cwd: %s\n", dir );

	// Lua
	LUA_Init();

	done = FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		// elso
		if( (key == 'a') || mousebl ) {

			xprintf("*** ELSO **********\n");

			// Lua
			xprintf("LUA_RunFile(test.lua):\n");
			LUA_RunFile( "test.lua" );

			lua_State *Lua_VM = LUA_GetVM();

			LUA_CheckGlobal( "level" );

			// 1: get the function name
			// 2: push the value(s) (from left to right)
			// 3: call the function
			// 4: pop the return value(s)
			lua_getglobal(Lua_VM, "xprintf");
			lua_pushstring(Lua_VM, __FILE__);
			lua_pushstring(Lua_VM, ": helloka!\n");
			lua_call(Lua_VM, 2, 1);
			lua_pop(Lua_VM, 1);


			// 1: get the function name
			// 2: push the value(s) (from left to right)
			// 3: call the function
			// 4: pop the return value(s)
			lua_getglobal(Lua_VM, "double");
			lua_pushnumber(Lua_VM, 3);
			lua_call(Lua_VM, 1, 1);
			xprintf("double returend: %d\n", lua_tointeger(Lua_VM,-1) ); // Should be 6 :-)
			lua_pop(Lua_VM,1);


			// 2 arg
			lua_getglobal(Lua_VM, "f");
			lua_pushnumber(Lua_VM, 21);   /* push 1st argument */
			lua_pushnumber(Lua_VM, 31);   /* push 2nd argument */
			/* do the call (2 arguments, 1 result) */
			lua_pcall(Lua_VM, 2, 1, 0);
			xprintf("Result: %f\n",lua_tonumber(Lua_VM, -1));
			lua_pop(Lua_VM, 1);


			// table
			lua_getglobal(Lua_VM, "level");
			lua_pushnil(Lua_VM);

			xprintf("level = {");

			while(lua_next(Lua_VM, -2)) {
				if(lua_isnumber(Lua_VM, -1)) {
					int i = (int)lua_tonumber(Lua_VM, -1);
					//use number
					xprintf("%d, ", i );
				}
				lua_pop(Lua_VM, 1);
			}
			lua_pop(Lua_VM, 1);

			xprintf("}\n");

			SpeakText( "hello, press start button please" );
		}

		// masodik
		if( (key == 'b') || mousebr ) {

			xprintf("*** MASODIK **********\n");

			// Lua
			xprintf("LUA_RunString(...):\n");
			LUA_RunString( "a = 1 + 1;\nxprintf (\"1+1=\" .. a);\n" );
		}

		// render

		Clear( NOCOLOR ); // 0x00209f  NOCOLOR

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );
		SetupFog( zclipfar.value/2.0f, zclipfar.value, 0x000000 );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		FlushScene();

		consoleAnimate();

		DrawMouse();

		EndScene();

		FlipPage();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	// scripty END

	LUA_Deinit();

	DeinitSpeak();

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}




/*

//
//
//
void valami( char *string ) {

	int x,y,font_color;
	int box_width;

	x = mousex + SPRITEW(mouse_bitmap);
	y = mousey + SPRITEH(mouse_bitmap);

	font_color = FontColor();
	SetFontColor( 0x000000 );

	box_width = MeasureString( string );

	FilledBox( x,y, x + FontWidth()*3 + box_width, y + FontHeight()*2, RGB8INT(0xff,0xff,0xb8) );
	Box( x,y, x + FontWidth()*3 + box_width, y + FontHeight()*2, 0xffffff );

	WriteString( x+FontWidth(), y+FontHeight()/2, string );

	SetFontColor( font_color );

	return;
}





//
//
//
void test34( void ) {

	BOOL done = FALSE;
	int key;
	ULONG frame = 0;
	UCHAR *mouse_spr, *mouse_rgb;

	SetX(320,200,16,SETX_NORMAL);

	InitKeyboard();

	if(!InitMouse(0)) xprintf("no mouse!");


	if( LoadPicture("mokus1.pcx",PTR(mouse_spr)) == FALSE ) Quit("no mokus1.pcx");
	ALLOCMEM( mouse_rgb, 768 );
	GetRgbPicture( mouse_rgb );

	ChangeMouseCursor( mouse_spr, mouse_rgb, 0,0 );

	FREEMEM( mouse_spr );
	FREEMEM( mouse_rgb );

	InitNetwork();
	SetupServer();

	done=FALSE;
	while( !done ) {

		DrawMouse();

		key = GetKey();

		if( key == key_Escape ) done = TRUE;

		HideMouse();
		Clear(NOCOLOR);
		ShowMouse();

		WriteString(10,10,"frame: %d",frame++);

		valami( "proba string");

		FlipPage();
	}

	ShutdownServer();
	DeinitNetwork();

	ResetX();

	return;
}



//
//
//
void test36( void ) {

	int done=0,key,handle,pan,i;
	char *txt;

	if( !winOpenFile( "Wave Files (*.wav)\0*.wav\0MP3 Files (*.mp3)\0*.mp3\0All Files (*.*)\0*.*\0") )
		Quit( NULL );

	if( (txt = winGetNextFile()) == NULL )
		Quit( "no file" );

	InitKeyboard();

	SetX(640,480,16,SETX_NORMAL);

	i = 0;
	SETFLAG( i, MF_3DCURSOR );
	if(!InitMouse(0)) xprintf("no mouse!");
	// HideMouse();

	DS_Init();

	handle = 1;
	if( DS_LoadWave( txt, handle ) == -1 )
		xprintf("cannot load.");

	pan = MAXPAN/2;

	DS_SetWaveParam( handle, -1, MAXVOL, pan, 0 );

	while( !done ) {

		HideMouse();
		Clear( 0x00209f );
		ShowMouse();

		key = GetKey();

		if( key == key_Escape )
			done = 1;

		pan = mousex * MAXPAN / SCREENW;

		if( key == ' ' || mousebl )
			if( !DS_IsPlaying( handle ) ) {
				DS_SetWaveParam( handle, -1, MAXVOL, pan, 0 );
				DS_PlayWave( handle );
			}

		if( key == key_Enter ) {
			MIDI_PlayFile( "enig.mid" );
			xprintf("playing...\n");
		}

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		consoleAnimate();

		DrawMouse();

		WriteString(10,10,"memory used: %d\nvideo: %dx%dx%d\nfile: %s\npan: %d / %d",(int)mem_used,SCREENW,SCREENH,bpp,txt,pan,MAXPAN);

		FlipPage();
	}

	DS_Deinit();

	ResetX();

	return;
}


#include <xnet.h>

//
//
//
void test35( void ) {

	BOOL done;
	int key,x,cnt=0,i;
	ULONG frame=0L,framecnt=0,tics=0L;
	double newtime,oldtime,time;
	char server_name[MAX_PATH] = "local";
	point3_t cam_pos,cam_view;
	int width,height,depth;

	width = winReadProfileInt( "xlib_width", -1 );
	height = winReadProfileInt( "xlib_height", -1 );
	depth = winReadProfileInt( "xlib_depth", 16 );

	SetX(width,height,depth,SETX_NORMAL);

	// SetFont( LoadFont( "triton.fnt" ) - 1 );

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);

	MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
	MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );

	if( (x=CheckParm("server")) )
		strcpy( server_name, myargv[x+1] );
	else
		strcpy( server_name, winGetMachineName() );

	Host_Init();
	oldtime = GetTic();

	done = FALSE;
	while( !done ) {

		newtime = Sys_FloatTime();
		time = newtime - oldtime;

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		if( key == key_Escape ) done = TRUE;
		if( scan_code == sc_Escape ) done = TRUE;

		// client

		if( key == EXT(sc_F1) ) {

			if( hostCacheCount > 0 )
				strcpy( server_name, hostcache[0].name );

			xprintf("trying to connect to %s...\n",server_name);
			Host_Connect( server_name );
		}

		if( key == EXT(sc_F2) )
			Host_Disconnect();

		// server

		if( key == EXT(sc_F11) ) {
			Host_ShutdownServer( FALSE );
			NET_Listen( TRUE );
			SV_SpawnServer( "MAPNAME" );
		}

		if( key == EXT(sc_F12) )
			Host_ShutdownServer( FALSE );

		if( key == 'p' ) {
			cmd_source = src_command;
			Host_Ping();
		}

		if( key == 'l' )
			NET_Slist();
		NET_Poll();

		if( key == 't' )
			for( i=0; i<hostCacheCount; i++ )
				Host_Test( hostcache[i].name );


		Clear( NOCOLOR ); // 0x00209f );

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
						 CLIPMAXX - CLIPMINX,
						 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		FlushScene();

		consoleAnimate();

		DrawMouse();

		WriteString(10,10,"memory used: %d\nframe/sec: %d\nframe cnt: %d\nvideo: %dx%dx%d",(int)mem_used,(int)frame,cnt++,SCREENW,SCREENH,bpp);

		EndScene();

		FlipPage();

		Host_Frame( time );
		oldtime = newtime;

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	Host_Disconnect();
	Host_ShutdownServer( FALSE );

	Host_Shutdown();

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}

*/

fontloader_t gold0_bmp[] = {
	// font\tower\gold0.bmp
	{ 'A', 1,1, 36,50 },
	{ 'B', 38,1, 69,50 },
	{ 'C', 71,1, 109,50 },
	{ 'D', 111,1, 149,50 },
	{ 'E', 151,1, 181,50 },
	{ 'F', 183,1, 213,50 },
	{ 'G', 215,1, 255,50 },
	{ 'H', 1,52, 42,101 },
	{ 'I', 44,52, 62,101 },
	{ 'J', 64,52, 84,101 },
	{ 'K', 86,52, 124,101 },
	{ 'L', 126,52, 157,101 },
	{ 'M', 159,52, 205,101 },
	{ 'N', 207,52, 245,101 },
	{ 'O', 1,103, 43,152 },
	{ 'P', 45,103, 76,152 },
	{ 'Q', 78,103, 121,152 },
	{ 'R', 123,103, 153,152 },
	{ 'S', 155,103, 186,152 },
	{ 'T', 188,103, 222,152 },
	{ 'U', 1,154, 43,203 },
	{ 'V', 45,154, 86,203 },
	{ 'W', 88,154, 137,203 },
	{ 'X', 139,154, 177,203 },
	{ 'Y', 179,154, 216,203 },
	{ 'Z', 218,154, 253,203 },
};


static fontloader_t nfkfont_bmp[] = {
	// font\nfk\nfkfont.bmp
	{ 'A', 0,1, 16,18 },
	{ 'B', 18,1, 34,18 },
	{ 'C', 36,1, 51,18 },
	{ 'D', 53,1, 68,18 },
	{ 'E', 71,1, 84,18 },
	{ 'F', 87,1, 101,18 },
	{ ':', 104,1, 111,18 },
	{ '-', 113,1, 122,18 },
	{ 'G', 0,21, 15,38 },
	{ 'H', 17,21, 32,38 },
	{ 'I', 34,21, 42,38 },
	{ 'J', 44,21, 56,38 },
	{ 'K', 58,21, 74,38 },
	{ 'L', 77,21, 89,38 },
	{ '?', 92,21, 103,38 },
	{ '\'', 106,21, 111,38 },
	{ 'Ä', 114,21, 125,38 },
	{ 'M', 0,41, 16,58 },
	{ 'N', 18,41, 34,58 },
	{ 'O', 36,41, 51,58 },
	{ 'P', 53,41, 68,58 },
	{ 'Q', 71,41, 86,58 },
	{ 'R', 89,41, 104,58 },
	{ '@', 107,41, 121,58 },
	{ 'S', 0,61, 13,78 },
	{ 'T', 15,61, 27,78 },
	{ 'U', 29,61, 44,78 },
	{ 'V', 46,61, 61,78 },
	{ 'W', 63,61, 81,78 },
	{ 'X', 84,61, 98,78 },
	{ ',', 101,61, 103,78 },
	{ '!', 107,61, 111,78 },
	{ 'Ü', 114,61, 125,78 },
	{ 'Y', 0,82, 13,98 },
	{ 'Z', 15,82, 28,98 },
	{ ' ', 30,81, 36,98 },
	{  1,  39,81, 78,98 },
	{ '0', 81,82, 92,98 },
	{ '1', 94,82, 102,98 },
	{ '2', 105,82, 116,98 },
	{ '3', 0,102, 11,118 },
	{ '4', 13,102, 24,118 },
	{ '5', 26,102, 38,118 },
	{ '6', 40,102, 51,118 },
	{ '7', 53,102, 64,118 },
	{ '8', 67,102, 78,118 },
	{ '9', 81,102, 90,118 },
	{ '.', 93,102, 98,118 },
	{ '/', 101,102, 109,118 },
	{ 'Ö', 112,102, 123,118 },
};




fontloader_t asciifont_bmp[] = {
	// font\ascii.bmp
	{ /* " " */ 32, 0,16, 8,24 },
	{ /* "!" */ 33, 8,16, 16,24 },
	{ /* """ */ 34, 16,16, 24,24 },
	{ /* "#" */ 35, 24,16, 32,24 },
	{ /* "$" */ 36, 32,16, 40,24 },
	{ /* "%" */ 37, 40,16, 48,24 },
	{ /* "&" */ 38, 48,16, 56,24 },
	{ /* "'" */ 39, 56,16, 64,24 },
	{ /* "(" */ 40, 64,16, 72,24 },
	{ /* ")" */ 41, 72,16, 80,24 },
	{ /* "*" */ 42, 80,16, 88,24 },
	{ /* "+" */ 43, 88,16, 96,24 },
	{ /* "," */ 44, 96,16, 104,24 },
	{ /* "-" */ 45, 104,16, 112,24 },
	{ /* "." */ 46, 112,16, 120,24 },
	{ /* "/" */ 47, 120,16, 128,24 },
	{ /* "0" */ 48, 0,24, 8,32 },
	{ /* "1" */ 49, 8,24, 16,32 },
	{ /* "2" */ 50, 16,24, 24,32 },
	{ /* "3" */ 51, 24,24, 32,32 },
	{ /* "4" */ 52, 32,24, 40,32 },
	{ /* "5" */ 53, 40,24, 48,32 },
	{ /* "6" */ 54, 48,24, 56,32 },
	{ /* "7" */ 55, 56,24, 64,32 },
	{ /* "8" */ 56, 64,24, 72,32 },
	{ /* "9" */ 57, 72,24, 80,32 },
	{ /* ":" */ 58, 80,24, 88,32 },
	{ /* ";" */ 59, 88,24, 96,32 },
	{ /* "<" */ 60, 96,24, 104,32 },
	{ /* "=" */ 61, 104,24, 112,32 },
	{ /* ">" */ 62, 112,24, 120,32 },
	{ /* "?" */ 63, 120,24, 128,32 },
	{ /* "@" */ 64, 0,32, 8,40 },
	{ /* "A" */ 65, 8,32, 16,40 },
	{ /* "B" */ 66, 16,32, 24,40 },
	{ /* "C" */ 67, 24,32, 32,40 },
	{ /* "D" */ 68, 32,32, 40,40 },
	{ /* "E" */ 69, 40,32, 48,40 },
	{ /* "F" */ 70, 48,32, 56,40 },
	{ /* "G" */ 71, 56,32, 64,40 },
	{ /* "H" */ 72, 64,32, 72,40 },
	{ /* "I" */ 73, 72,32, 80,40 },
	{ /* "J" */ 74, 80,32, 88,40 },
	{ /* "K" */ 75, 88,32, 96,40 },
	{ /* "L" */ 76, 96,32, 104,40 },
	{ /* "M" */ 77, 104,32, 112,40 },
	{ /* "N" */ 78, 112,32, 120,40 },
	{ /* "O" */ 79, 120,32, 128,40 },
	{ /* "P" */ 80, 0,40, 8,48 },
	{ /* "Q" */ 81, 8,40, 16,48 },
	{ /* "R" */ 82, 16,40, 24,48 },
	{ /* "S" */ 83, 24,40, 32,48 },
	{ /* "T" */ 84, 32,40, 40,48 },
	{ /* "U" */ 85, 40,40, 48,48 },
	{ /* "V" */ 86, 48,40, 56,48 },
	{ /* "W" */ 87, 56,40, 64,48 },
	{ /* "X" */ 88, 64,40, 72,48 },
	{ /* "Y" */ 89, 72,40, 80,48 },
	{ /* "Z" */ 90, 80,40, 88,48 },
	{ /* "[" */ 91, 88,40, 96,48 },
	{ /* "\" */ 92, 96,40, 104,48 },
	{ /* "]" */ 93, 104,40, 112,48 },
	{ /* "^" */ 94, 112,40, 120,48 },
	{ /* "_" */ 95, 120,40, 128,48 },
	{ /* "`" */ 96, 0,48, 8,56 },
	{ /* "a" */ 97, 8,48, 16,56 },
	{ /* "b" */ 98, 16,48, 24,56 },
	{ /* "c" */ 99, 24,48, 32,56 },
	{ /* "d" */ 100, 32,48, 40,56 },
	{ /* "e" */ 101, 40,48, 48,56 },
	{ /* "f" */ 102, 48,48, 56,56 },
	{ /* "g" */ 103, 56,48, 64,56 },
	{ /* "h" */ 104, 64,48, 72,56 },
	{ /* "i" */ 105, 72,48, 80,56 },
	{ /* "j" */ 106, 80,48, 88,56 },
	{ /* "k" */ 107, 88,48, 96,56 },
	{ /* "l" */ 108, 96,48, 104,56 },
	{ /* "m" */ 109, 104,48, 112,56 },
	{ /* "n" */ 110, 112,48, 120,56 },
	{ /* "o" */ 111, 120,48, 128,56 },
	{ /* "p" */ 112, 0,56, 8,64 },
	{ /* "q" */ 113, 8,56, 16,64 },
	{ /* "r" */ 114, 16,56, 24,64 },
	{ /* "s" */ 115, 24,56, 32,64 },
	{ /* "t" */ 116, 32,56, 40,64 },
	{ /* "u" */ 117, 40,56, 48,64 },
	{ /* "v" */ 118, 48,56, 56,64 },
	{ /* "w" */ 119, 56,56, 64,64 },
	{ /* "x" */ 120, 64,56, 72,64 },
	{ /* "y" */ 121, 72,56, 80,64 },
	{ /* "z" */ 122, 80,56, 88,64 },
	{ /* "{" */ 123, 88,56, 96,64 },
	{ /* "|" */ 124, 96,56, 104,64 },
	{ /* "}" */ 125, 104,56, 112,64 },
	{ /* "~" */ 126, 112,56, 120,64 },
	{ /* "" */ 127, 120,56, 128,64 },
};

fontloader_t nehefont1_bmp[] = {
	// font\nehe\nehe.bmp
	{ /* " " */ 32, 2,0, 14,16 },
	{ /* "!" */ 33, 18,0, 30,16 },
	{ /* """ */ 34, 34,0, 46,16 },
	{ /* "#" */ 35, 50,0, 62,16 },
	{ /* "$" */ 36, 66,0, 78,16 },
	{ /* "%" */ 37, 82,0, 94,16 },
	{ /* "&" */ 38, 98,0, 110,16 },
	{ /* "'" */ 39, 114,0, 126,16 },
	{ /* "(" */ 40, 130,0, 142,16 },
	{ /* ")" */ 41, 146,0, 158,16 },
	{ /* "*" */ 42, 162,0, 174,16 },
	{ /* "+" */ 43, 178,0, 190,16 },
	{ /* "," */ 44, 194,0, 206,16 },
	{ /* "-" */ 45, 210,0, 222,16 },
	{ /* "." */ 46, 226,0, 238,16 },
	{ /* "/" */ 47, 242,0, 254,16 },
	{ /* "0" */ 48, 2,16, 14,32 },
	{ /* "1" */ 49, 18,16, 30,32 },
	{ /* "2" */ 50, 34,16, 46,32 },
	{ /* "3" */ 51, 50,16, 62,32 },
	{ /* "4" */ 52, 66,16, 78,32 },
	{ /* "5" */ 53, 82,16, 94,32 },
	{ /* "6" */ 54, 98,16, 110,32 },
	{ /* "7" */ 55, 114,16, 126,32 },
	{ /* "8" */ 56, 130,16, 142,32 },
	{ /* "9" */ 57, 146,16, 158,32 },
	{ /* ":" */ 58, 162,16, 174,32 },
	{ /* ";" */ 59, 178,16, 190,32 },
	{ /* "<" */ 60, 194,16, 206,32 },
	{ /* "=" */ 61, 210,16, 222,32 },
	{ /* ">" */ 62, 226,16, 238,32 },
	{ /* "?" */ 63, 242,16, 254,32 },
	{ /* "@" */ 64, 2,32, 14,48 },
	{ /* "A" */ 65, 18,32, 30,48 },
	{ /* "B" */ 66, 34,32, 46,48 },
	{ /* "C" */ 67, 50,32, 62,48 },
	{ /* "D" */ 68, 66,32, 78,48 },
	{ /* "E" */ 69, 82,32, 94,48 },
	{ /* "F" */ 70, 98,32, 110,48 },
	{ /* "G" */ 71, 114,32, 126,48 },
	{ /* "H" */ 72, 130,32, 142,48 },
	{ /* "I" */ 73, 146,32, 158,48 },
	{ /* "J" */ 74, 162,32, 174,48 },
	{ /* "K" */ 75, 178,32, 190,48 },
	{ /* "L" */ 76, 194,32, 206,48 },
	{ /* "M" */ 77, 210,32, 222,48 },
	{ /* "N" */ 78, 226,32, 238,48 },
	{ /* "O" */ 79, 242,32, 254,48 },
	{ /* "P" */ 80, 2,48, 14,64 },
	{ /* "Q" */ 81, 18,48, 30,64 },
	{ /* "R" */ 82, 34,48, 46,64 },
	{ /* "S" */ 83, 50,48, 62,64 },
	{ /* "T" */ 84, 66,48, 78,64 },
	{ /* "U" */ 85, 82,48, 94,64 },
	{ /* "V" */ 86, 98,48, 110,64 },
	{ /* "W" */ 87, 114,48, 126,64 },
	{ /* "X" */ 88, 130,48, 142,64 },
	{ /* "Y" */ 89, 146,48, 158,64 },
	{ /* "Z" */ 90, 162,48, 174,64 },
	{ /* "[" */ 91, 178,48, 190,64 },
	{ /* "\" */ 92, 194,48, 206,64 },
	{ /* "]" */ 93, 210,48, 222,64 },
	{ /* "^" */ 94, 226,48, 238,64 },
	{ /* "_" */ 95, 242,48, 254,64 },
	{ /* "`" */ 96, 2,64, 14,80 },
	{ /* "a" */ 97, 18,64, 30,80 },
	{ /* "b" */ 98, 34,64, 46,80 },
	{ /* "c" */ 99, 50,64, 62,80 },
	{ /* "d" */ 100, 66,64, 78,80 },
	{ /* "e" */ 101, 82,64, 94,80 },
	{ /* "f" */ 102, 98,64, 110,80 },
	{ /* "g" */ 103, 114,64, 126,80 },
	{ /* "h" */ 104, 130,64, 142,80 },
	{ /* "i" */ 105, 146,64, 158,80 },
	{ /* "j" */ 106, 162,64, 174,80 },
	{ /* "k" */ 107, 178,64, 190,80 },
	{ /* "l" */ 108, 194,64, 206,80 },
	{ /* "m" */ 109, 210,64, 222,80 },
	{ /* "n" */ 110, 226,64, 238,80 },
	{ /* "o" */ 111, 242,64, 254,80 },
	{ /* "p" */ 112, 2,80, 14,96 },
	{ /* "q" */ 113, 18,80, 30,96 },
	{ /* "r" */ 114, 34,80, 46,96 },
	{ /* "s" */ 115, 50,80, 62,96 },
	{ /* "t" */ 116, 66,80, 78,96 },
	{ /* "u" */ 117, 82,80, 94,96 },
	{ /* "v" */ 118, 98,80, 110,96 },
	{ /* "w" */ 119, 114,80, 126,96 },
	{ /* "x" */ 120, 130,80, 142,96 },
	{ /* "y" */ 121, 146,80, 158,96 },
	{ /* "z" */ 122, 162,80, 174,96 },
	{ /* "{" */ 123, 178,80, 190,96 },
	{ /* "|" */ 124, 194,80, 206,96 },
	{ /* "}" */ 125, 210,80, 222,96 },
	{ /* "~" */ 126, 226,80, 238,96 },
	{ /* "" */ 127, 242,80, 254,96 },
};

fontloader_t nehefont2_bmp[] = {
	// font\nehe\nehe.bmp
	{ /* " " */ 32, 2,128, 14,144 },
	{ /* "!" */ 33, 18,128, 30,144 },
	{ /* """ */ 34, 34,128, 46,144 },
	{ /* "#" */ 35, 50,128, 62,144 },
	{ /* "$" */ 36, 66,128, 78,144 },
	{ /* "%" */ 37, 82,128, 94,144 },
	{ /* "&" */ 38, 98,128, 110,144 },
	{ /* "'" */ 39, 114,128, 126,144 },
	{ /* "(" */ 40, 130,128, 142,144 },
	{ /* ")" */ 41, 146,128, 158,144 },
	{ /* "*" */ 42, 162,128, 174,144 },
	{ /* "+" */ 43, 178,128, 190,144 },
	{ /* "," */ 44, 194,128, 206,144 },
	{ /* "-" */ 45, 210,128, 222,144 },
	{ /* "." */ 46, 226,128, 238,144 },
	{ /* "/" */ 47, 242,128, 254,144 },
	{ /* "0" */ 48, 2,144, 14,160 },
	{ /* "1" */ 49, 18,144, 30,160 },
	{ /* "2" */ 50, 34,144, 46,160 },
	{ /* "3" */ 51, 50,144, 62,160 },
	{ /* "4" */ 52, 66,144, 78,160 },
	{ /* "5" */ 53, 82,144, 94,160 },
	{ /* "6" */ 54, 98,144, 110,160 },
	{ /* "7" */ 55, 114,144, 126,160 },
	{ /* "8" */ 56, 130,144, 142,160 },
	{ /* "9" */ 57, 146,144, 158,160 },
	{ /* ":" */ 58, 162,144, 174,160 },
	{ /* ";" */ 59, 178,144, 190,160 },
	{ /* "<" */ 60, 194,144, 206,160 },
	{ /* "=" */ 61, 210,144, 222,160 },
	{ /* ">" */ 62, 226,144, 238,160 },
	{ /* "?" */ 63, 242,144, 254,160 },
	{ /* "@" */ 64, 2,160, 14,176 },
	{ /* "A" */ 65, 18,160, 30,176 },
	{ /* "B" */ 66, 34,160, 46,176 },
	{ /* "C" */ 67, 50,160, 62,176 },
	{ /* "D" */ 68, 66,160, 78,176 },
	{ /* "E" */ 69, 82,160, 94,176 },
	{ /* "F" */ 70, 98,160, 110,176 },
	{ /* "G" */ 71, 114,160, 126,176 },
	{ /* "H" */ 72, 130,160, 142,176 },
	{ /* "I" */ 73, 146,160, 158,176 },
	{ /* "J" */ 74, 162,160, 174,176 },
	{ /* "K" */ 75, 178,160, 190,176 },
	{ /* "L" */ 76, 194,160, 206,176 },
	{ /* "M" */ 77, 210,160, 222,176 },
	{ /* "N" */ 78, 226,160, 238,176 },
	{ /* "O" */ 79, 242,160, 254,176 },
	{ /* "P" */ 80, 2,176, 14,192 },
	{ /* "Q" */ 81, 18,176, 30,192 },
	{ /* "R" */ 82, 34,176, 46,192 },
	{ /* "S" */ 83, 50,176, 62,192 },
	{ /* "T" */ 84, 66,176, 78,192 },
	{ /* "U" */ 85, 82,176, 94,192 },
	{ /* "V" */ 86, 98,176, 110,192 },
	{ /* "W" */ 87, 114,176, 126,192 },
	{ /* "X" */ 88, 130,176, 142,192 },
	{ /* "Y" */ 89, 146,176, 158,192 },
	{ /* "Z" */ 90, 162,176, 174,192 },
	{ /* "[" */ 91, 178,176, 190,192 },
	{ /* "\" */ 92, 194,176, 206,192 },
	{ /* "]" */ 93, 210,176, 222,192 },
	{ /* "^" */ 94, 226,176, 238,192 },
	{ /* "_" */ 95, 242,176, 254,192 },
	{ /* "`" */ 96, 2,192, 14,208 },
	{ /* "a" */ 97, 18,192, 30,208 },
	{ /* "b" */ 98, 34,192, 46,208 },
	{ /* "c" */ 99, 50,192, 62,208 },
	{ /* "d" */ 100, 66,192, 78,208 },
	{ /* "e" */ 101, 82,192, 94,208 },
	{ /* "f" */ 102, 98,192, 110,208 },
	{ /* "g" */ 103, 114,192, 126,208 },
	{ /* "h" */ 104, 130,192, 142,208 },
	{ /* "i" */ 105, 146,192, 158,208 },
	{ /* "j" */ 106, 162,192, 174,208 },
	{ /* "k" */ 107, 178,192, 190,208 },
	{ /* "l" */ 108, 194,192, 206,208 },
	{ /* "m" */ 109, 210,192, 222,208 },
	{ /* "n" */ 110, 226,192, 238,208 },
	{ /* "o" */ 111, 242,192, 254,208 },
	{ /* "p" */ 112, 2,208, 14,224 },
	{ /* "q" */ 113, 18,208, 30,224 },
	{ /* "r" */ 114, 34,208, 46,224 },
	{ /* "s" */ 115, 50,208, 62,224 },
	{ /* "t" */ 116, 66,208, 78,224 },
	{ /* "u" */ 117, 82,208, 94,224 },
	{ /* "v" */ 118, 98,208, 110,224 },
	{ /* "w" */ 119, 114,208, 126,224 },
	{ /* "x" */ 120, 130,208, 142,224 },
	{ /* "y" */ 121, 146,208, 158,224 },
	{ /* "z" */ 122, 162,208, 174,224 },
	{ /* "{" */ 123, 178,208, 190,224 },
	{ /* "|" */ 124, 194,208, 206,224 },
	{ /* "}" */ 125, 210,208, 222,224 },
	{ /* "~" */ 126, 226,208, 238,224 },
	{ /* "" */ 127, 242,208, 254,224 },
};

fontloader_t exocetfont_bmp[] = {
	// font\exocet.bmp
	{ 'A', 1,2, 17,19 },
	{ 'B', 19,2, 32,19 },
	{ 'C', 34,2, 48,19 },
	{ 'D', 50,2, 66,19 },
	{ 'E', 68,2, 82,19 },
	{ 'F', 84,2, 97,19 },
	{ 'G', 99,2, 112,19 },
	{ 'H', 114,2, 126,19 },
	{ 'I', 1,21, 8,39 },
	{ 'J', 10,21, 19,42 },
	{ 'K', 21,21, 35,39 },
	{ 'L', 37,21, 48,39 },
	{ 'M', 50,21, 69,39 },
	{ 'N', 71,21, 88,39 },
	{ 'O', 90,21, 107,39 },
	{ 'P', 109,21, 121,39 },
	{ 'Q', 1,44, 16,62 },
	{ 'R', 18,44, 32,62 },
	{ 'S', 34,44, 45,62 },
	{ 'T', 47,44, 63,62 },
	{ 'U', 65,44, 84,62 },
	{ 'V', 86,44, 103,62 },
	{ 'W', 105,44, 126,62 },
	{ 'X', 1,64, 16,81 },
	{ 'Y', 18,64, 36,81 },
	{ 'Z', 38,64, 52,81 },
	{ '!', 54,64, 60,81 },
	{ '"', 62,64, 74,81 },
	{ '$', 76,64, 87,81 },
	{ '%', 89,64, 108,81 },
	{ '@', 110,64, 126,81 },
	{ '\'', 1,83, 6,104 },
	{ '(', 8,83, 16,104 },
	{ ')', 18,83, 26,104 },
	{ '*', 28,83, 39,104 },
	{ ',', 41,83, 46,104 },
	{ '-', 48,83, 55,104 },
	{ '.', 57,83, 61,104 },
	{ '/', 63,83, 75,104 },
	{ '?', 77,85, 87,104 },
	{ ':', 89,87, 98,104 },
	{ '0', 100,86, 116,104 },
	{ '+', 118,84, 126,104 },
	{ '1',  1,109, 9,126 },
	{ '2',  11,109, 24,126 },
	{ '3',  26,109, 38,126 },
	{ '4',  40,109, 53,126 },
	{ '5',  55,109, 67,126 },
	{ '6',  69,109, 82,126 },
	{ '7',  84,109, 97,126 },
	{ '8',  99,109, 111,126 },
	{ '9',  113,109, 126,126 },
};


fontloader_t matrixfont_bmp[] = {
	// font\matrix\matrix1.bmp
	{ 33,  0,0, 16,16 },		// 1. sor
	{ 34, 17,0, 32,16 },
	{ 35, 33,0, 48,16 },
	{ 36, 49,0, 64,16 },
	{ 37, 65,0, 80,16 },
	{ 38, 81,0, 96,16 },
	{ 39, 97,0, 112,16 },
	{ 40, 113,0, 126,16 },
	{ 41,  0,17, 16,32 },		// 2. sor
	{ 42, 17,17, 32,32 },
	{ 43, 33,17, 48,32 },
	{ 44, 49,17, 64,32 },
	{ 45, 65,17, 80,32 },
	{ 46, 81,17, 96,32 },
	{ 47, 97,17, 112,32 },
	{ 48, 113,17, 126,32 },
	{ 49,  0,33, 16,48 },		// 3. sor
	{ 50, 17,33, 32,48 },
	{ 51, 33,33, 48,48 },
	{ 52, 49,33, 64,48 },
	{ 53, 65,33, 80,48 },
	{ 54, 81,33, 96,48 },
	{ 55, 97,33, 112,48 },
	{ 56, 113,33, 126,48 },
	{ 57,  0,49, 16,64 },		// 4. sor
	{ 58, 17,49, 32,64 },
	{ 59, 33,49, 48,64 },
	{ 60, 49,49, 64,64 },
	{ 61, 65,49, 80,64 },
	{ 62, 81,49, 96,64 },
	{ 63, 97,49, 112,64 },
	{ 64, 113,49, 126,64 },
	{ 65,  0,65, 16,80 },		// 5. sor
	{ 66, 17,65, 32,80 },
	{ 67, 33,65, 48,80 },
	{ 68, 49,65, 64,80 },
	{ 69, 65,65, 80,80 },
	{ 70, 81,65, 96,80 },
	{ 71, 97,65, 112,80 },
	{ 72, 113,65, 126,80 },
	{ 73,  0,81, 16,96 },		// 6. sor
	{ 74, 17,81, 32,96 },
	{ 75, 33,81, 48,96 },
	{ 76, 49,81, 64,96 },
	{ 77, 65,81, 80,96 },
	{ 78, 81,81, 96,96 },
	{ 79, 97,81, 112,96 },
	{ 80, 113,81, 126,96 },
	{ 81,  0,97, 16,112 },		// 7. sor
	{ 82, 17,97, 32,112 },
	{ 83, 33,97, 48,112 },
	{ 84, 49,97, 64,112 },
	{ 85, 65,97, 80,112 },
	{ 86, 81,97, 96,112 },
	{ 87, 97,97, 112,112 },
	{ 88, 113,97, 126,112 }
};



fontloader_t matrix2font_bmp[] = {
	// font\matrix\matrix2.bmp
	{ 'A',  0,0, 16,16 },		// 1. sor
	{ 'B', 17,0, 32,16 },
	{ 'C', 33,0, 48,16 },
	{ 'D', 49,0, 64,16 },
	{ 'E', 65,0, 80,16 },
	{ 'F', 81,0, 96,16 },
	{ 'G', 97,0, 112,16 },
	{ 'H', 113,0, 126,16 },
	{ 'I',  0,17, 16,32 },		// 2. sor
	{ 'J', 17,17, 32,32 },
	{ 'K', 33,17, 48,32 },
	{ 'L', 49,17, 64,32 },
	{ 'M', 65,17, 80,32 },
	{ 'N', 81,17, 96,32 },
	{ 'O', 97,17, 112,32 },
	{ 'P', 113,17, 126,32 },
	{ 'Q',  0,33, 16,48 },		// 3. sor
	{ 'R', 17,33, 32,48 },
	{ 'S', 33,33, 48,48 },
	{ 'T', 49,33, 64,48 },
	{ 'U', 65,33, 80,48 },
	{ 'V', 81,33, 96,48 },
	{ 'W', 97,33, 112,48 },
	{ 'X', 113,33, 126,48 },
	{ 'Y',  0,49, 16,64 },		// 4. sor
	{ 'Z', 17,49, 32,64 },
	{ '?', 33,49, 48,64 },
	{ '!', 49,49, 64,64 },
	{ '@', 65,49, 80,64 },
	{ '$', 81,49, 96,64 },
	{ '&', 97,49, 112,64 }
};

// 16x20  color 0:255:0
fontloader_t ledfont_bmp[] = {
	// font\led.bmp
	{ /* " " */ 32, 0,0, 16,20 },		// 1. sor
	{ /* """ */ 33, 16,0, 32,20 },
	{ /* "$" */ 34, 32,0, 48,20 },
	{ /* "&" */ 35, 48,0, 64,20 },
	{ /* "(" */ 36, 64,0, 80,20 },
	{ /* "*" */ 37, 80,0, 96,20 },
	{ /* "," */ 38, 96,0, 112,20 },
	{ /* "." */ 39, 112,0, 128,20 },
	{ /* "/" */ 40, 128,0, 144,20 },
	{ /* "/" */ 41, 144,0, 160,20 },
	{ /* "/" */ 42, 160,0, 176,20 },
	{ /* "/" */ 43, 176,0, 192,20 },
	{ /* "/" */ 44, 192,0, 208,20 },
	{ /* "/" */ 45, 208,0, 224,20 },
	{ /* "/" */ 46, 224,0, 240,20 },
	{ /* "/" */ 47, 240,0, 256,20 },
	{ /* " " */ 48, 0,22, 16,42 },		// 2. sor
	{ /* """ */ 49, 16,22, 32,42 },
	{ /* "$" */ 50, 32,22, 48,42 },
	{ /* "&" */ 51, 48,22, 64,42 },
	{ /* "(" */ 52, 64,22, 80,42 },
	{ /* "*" */ 53, 80,22, 96,42 },
	{ /* "," */ 54, 96,22, 112,42 },
	{ /* "." */ 55, 112,22, 128,42 },
	{ /* "/" */ 56, 128,22, 144,42 },
	{ /* "/" */ 57, 144,22, 160,42 },
	{ /* "/" */ 58, 160,22, 176,42 },
	{ /* "/" */ 59, 176,22, 192,42 },
	{ /* "/" */ 60, 192,22, 208,42 },
	{ /* "/" */ 61, 208,22, 224,42 },
	{ /* "/" */ 62, 224,22, 240,42 },
	{ /* "/" */ 63, 240,22, 256,42 },
	{ /* " " */ 64, 0,44, 16,64 },		// 3. sor
	{ /* """ */ 65, 16,44, 32,64 },
	{ /* "$" */ 66, 32,44, 48,64 },
	{ /* "&" */ 67, 48,44, 64,64 },
	{ /* "(" */ 68, 64,44, 80,64 },
	{ /* "*" */ 69, 80,44, 96,64 },
	{ /* "," */ 70, 96,44, 112,64 },
	{ /* "." */ 71, 112,44, 128,64 },
	{ /* "/" */ 72, 128,44, 144,64 },
	{ /* "/" */ 73, 144,44, 160,64 },
	{ /* "/" */ 74, 160,44, 176,64 },
	{ /* "/" */ 75, 176,44, 192,64 },
	{ /* "/" */ 76, 192,44, 208,64 },
	{ /* "/" */ 77, 208,44, 224,64 },
	{ /* "/" */ 78, 224,44, 240,64 },
	{ /* "/" */ 79, 240,44, 256,64 },
	{ /* " " */ 80, 0,66, 16,86 },		// 4. sor
	{ /* """ */ 81, 16,66, 32,86 },
	{ /* "$" */ 82, 32,66, 48,86 },
	{ /* "&" */ 83, 48,66, 64,86 },
	{ /* "(" */ 84, 64,66, 80,86 },
	{ /* "*" */ 85, 80,66, 96,86 },
	{ /* "," */ 86, 96,66, 112,86 },
	{ /* "." */ 87, 112,66, 128,86 },
	{ /* "/" */ 88, 128,66, 144,86 },
	{ /* "/" */ 89, 144,66, 160,86 },
	{ /* "/" */ 90, 160,66, 176,86 },
	{ /* "/" */ 91, 176,66, 192,86 },
	{ /* "/" */ 92, 192,66, 208,86 },
	{ /* "/" */ 93, 208,66, 224,86 },
	{ /* "/" */ 94, 224,66, 240,86 },
	{ /* "/" */ 95, 240,66, 256,86 }
};


fontloader_t kanafont_bmp[] = {
	// font\kana.bmp
	{ 'A',  0,0, 48,63 },			// 1. sor
	{ 'B',  49,0, 87,63 },
	{ 'C',  88,0, 127,63 },
	{ 'D',  0,64, 48,127 },			// 2. sor
	{ 'E',  49,64, 87,127 },
	{ 'F',  88,64, 127,127 },
	{ 'G',  128,64, 170,127 },
	{ 'H',  171,64, 208,127 },
	{ 'I',  209,64, 252,127 },
	{ 'J',  0,128, 48,192 },		// 3. sor
	{ 'K',  49,128, 87,192 },
	{ 'L',  88,128, 127,192 },
	{ 'M',  128,128, 170,192 },
	{ 'N',  0,193, 48,255 },		// 4. sor
	{ 'O',  49,193, 87,255 },
	{ 'P',  88,193, 127,255 },
	{ 'Q',  128,193, 170,255 },
	{ 'R',  171,193, 208,255 },
	{ 'S',  209,193, 252,255 }
};


// 320x192
// 32x32 fontok
// 10 darab egy sorban
// 6 magas
fontloader_t biggoldfont_bmp[] = {
	// font\biggoldfont.bmp
	{  0,  0,0, 31,31 },			// 1. sor
	{ '!', 32,0, 63,31 },
	{ '"', 64,0, 95,31 },
	{ '\'', 224,0, 255,31 },
	{ '(', 256,0, 287,31 },
	{ ')', 288,0, 319,31 },

	{ ',', 64,32, 95,63 },
	{ '.', 128,32, 159,63 },

	{ 'B',  49,0, 87,63 },
	{ 'C',  88,0, 127,63 },
	{ 'D',  0,64, 48,127 },			// 2. sor
	{ 'E',  49,64, 87,127 },
	{ 'F',  88,64, 127,127 },
	{ 'G',  128,64, 170,127 },
	{ 'H',  171,64, 208,127 },
	{ 'I',  209,64, 252,127 },
	{ 'J',  0,128, 48,192 },		// 3. sor
	{ 'K',  49,128, 87,192 },
	{ 'L',  88,128, 127,192 },
	{ 'M',  128,128, 170,192 },
	{ 'N',  0,193, 48,255 },		// 4. sor
	{ 'O',  49,193, 87,255 },
	{ 'P',  88,193, 127,255 },
	{ 'Q',  128,193, 170,255 },
	{ 'R',  171,193, 208,255 },
	{ 'S',  209,193, 252,255 }
};




//
// A: 1,1 36,50
//
void test37( void ) {

	BOOL done = FALSE;
	int x,y,key,cnt=0,f,d;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;
	FLOAT zclipnear=0.0f,zclipfar=250.0f;
	char *txt;

	if( !winOpenFile( "Trutype Font (.ttf)\0*.ttf\0All Files (*.*)\0*.*\0") )
		Quit( NULL );

	if( (txt=winGetNextFile()) == NULL )
		return;

	/*
	{
		UCHAR *buf = NULL;
		int size;

		size = LoadFile( txt, PTR(buf) );
		PushTomFlag();
		TomFlag( ZIPFLAG );
		WriteFile( "1.1", buf, size );
		PopTomFlag();

		FREEMEM( buf );
	}
	*/

	static char c[]=" AbCdEfG \n HiJkLmNoPqR \n StUvWxYz \n 0123456789:!? \n -=()+[] .,><//\\*#";

	x = winReadProfileInt( "xlib_width", 800 );
	y = winReadProfileInt( "xlib_height", 600 );
	d = winReadProfileInt( "xlib_depth", 16 );

	SetX(x,y,d,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);

	SetFont( LoadFont3D( "font\\tower\\gold0.bmp", TRUE, FALSE, gold0_bmp, dimof(gold0_bmp) ) - 1 );
	SetFont( LoadFont3D( "font\\nfk\\nfkfont.bmp", TRUE, FALSE, nfkfont_bmp, dimof(nfkfont_bmp) ) - 1 );
	SetFont( LoadFont3D( "font\\ascii.bmp", TRUE, FALSE, asciifont_bmp, dimof(asciifont_bmp) ) - 1 );
	SetFont( LoadFont3D( "font\\nehe\\nehe.bmp", TRUE, FALSE, nehefont1_bmp, dimof(nehefont1_bmp) ) - 1 );
	SetFont( LoadFont3D( "font\\nehe\\nehe.bmp", TRUE, FALSE, nehefont2_bmp, dimof(nehefont2_bmp) ) - 1 );
	SetFont( LoadFont3D( "font\\exocet.bmp", TRUE, FALSE, exocetfont_bmp, dimof(exocetfont_bmp) ) - 1 );
	SetFont( LoadFont3D( "font\\led.bmp", FALSE, FALSE, ledfont_bmp, dimof(ledfont_bmp) ) - 1 );
	SetFont( LoadFont3D( "font\\kana.bmp", TRUE, FALSE, kanafont_bmp, dimof(kanafont_bmp) ) - 1 );

	SetFont( LoadFont3D( "font\\biggoldfont.bmp", TRUE, FALSE, biggoldfont_bmp, dimof(biggoldfont_bmp) ) - 1 );

	f = LoadFont3D( txt, TRUE, FALSE, NULL, 0 );

	MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
	MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );

	char myname[PATH_MAX];
	GetTtfName( "Arial", myname );
	xprintf( "ttf: %s\n", myname );

	done = FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		Clear( 0x00209f ); // 0x00209f NOCOLOR);

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		SetFont( f );
		SetFontAlpha( FA_ALL, 255,255,255, mousey*255/SCREENH );
		// SetFontRgb( FA_ALL, bquad );
		// SetFontAlpha( FA_EFFECT, FA_ALL,FR_MEGANTA,0, mousey*255/SCREENH );

		WriteString(10,10,"Memory used: %d\nFrame/sec: %d\nFrame cnt: %d\nVideo: %dx%dx%d",(int)mem_used,(int)frame,cnt++,SCREENW,SCREENH,bpp);

		WriteString( 10, SCREENH/2, c );

		// for( x=32; x<128; x++ )
		//	WriteChar( (x-32)*FontWidth(), SCREENH/2, x );

		/*
		if( mousebl ) {
			int volume = (1000 * mousey +1) / SCREENH;
			MIDI_SetVolume( volume );
		}
		*/

		if( mousebrd ) {
			FLOAT f = 0.0f + (2.0f - 0.0f)*(FLOAT)mousey/(FLOAT)SCREENH;
//			winSetGammaRamp( f );
			xprintf( "gamma: %.2f\n", f );
		}

		FlushScene();

		consoleAnimate();

		DrawMouse();

		EndScene();

		FlipPage();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}





//
//
//
typedef struct {				// Create A Structure For Particle

	BOOL	active;				// Active (Yes/No)
	FLOAT	life;				// Particle Life
	FLOAT	fade;				// Fade Speed
	rgb_t	rgb;
	point3_t pos;
	FLOAT	xi;					// X Direction
	FLOAT	yi;					// Y Direction
	FLOAT	zi;					// Z Direction
	FLOAT	xg;					// X Gravity
	FLOAT	yg;					// Y Gravity
	FLOAT	zg;					// Z Gravity
	FLOAT	xc,yc;				// center

} particle_t;					// Particles Structure




//
//
//
void DrawParticle( particle_t *particle, int texmapid ) {

	int i;
	FLOAT x,y,w,h;

	polygon_t poly;
	point_t point[4];	// a négyzet mind a 4 csucsának külön
	rgb_t rgb[4];
	point3_t st[4] = {
		{0.0f,0.0f,0.0f},
		{0.0f,255.0f,0.0f},
		{255.0f,255.0f,0.0f},
		{255.0f,0.0f,0.0f},
	};

	poly.npoints = 4;
	poly.point = point;

	x = particle->xc + particle->pos[0]/40.0f * SCREENW;
	y = particle->yc + particle->pos[1]/40.0f * SCREENH;

	w = 10.0f; // - particle->pos[2] * 10.0f / 40.f;
	h = 10.0f; // - particle->pos[2] * 10.0f / 40.f;

	CLAMPMINMAX( w, 5.0f, 20.0f );
	CLAMPMINMAX( h, 5.0f, 20.0f );

	point[0][0] = x - w;	point[0][1] = y - h;
	point[1][0] = x - w;	point[1][1] = y + h;
	point[2][0] = x + w;	point[2][1] = y + h;
	point[3][0] = x + w;	point[3][1] = y -h;

	for( i=0; i<4; i++ )
		memcpy( &rgb[i], &particle->rgb, sizeof(rgb_t) );

	PutSpritePoly( poly, st, texmapid, rgb );

	return;
}



//
// A: 1,1 36,50
//
void test38( void ) {

	BOOL done = FALSE;
	int x,y,key,cnt=0,i;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;
	FLOAT f,zclipnear=0.0f,zclipfar=250.0f;

	#define	MAX_PARTICLES	300		// Number Of Particles To Create
	#define BUTTON_PARTICLES (MAX_PARTICLES - (MAX_PARTICLES/3))

	BOOL	rainbow = TRUE;				// Rainbow Mode?

	FLOAT	slowdown = 2.0f;				// Slow Down Particles
	FLOAT	xspeed = 0.0f;						// Base X Speed (To Allow Keyboard Direction Of Tail)
	FLOAT	yspeed = 100.0f;						// Base Y Speed (To Allow Keyboard Direction Of Tail)
	FLOAT	zoom = -40.0f;				// Used To Zoom Out

	int	col = 0;						// Current Color Selection
	int	delay = 0;						// Rainbow Effect Delay
	int idlecnt = 0;
	int particle_oldmousex = mousex;
	int particle_oldmousey = mousey;
	ULONG rainbow_tic = 0;
	int deltax, deltay;

	particle_t particle[MAX_PARTICLES];	// Particle Array (Room For Particle Info)
	int texmapid;

	rgb_t color[12] = {		// Rainbow Of Colors
		{255,127,127,255},
		{255,191,127,255},
		{255,255,127,255},
		{191,255,127,255},

		{127,255,127,255},
		{127,255,191,255},
		{127,255,255,255},
		{127,191,255,255},

		{127,127,255,255},
		{191,127,255,255},
		{255,127,255,255},
		{255,127,191,255},
	};

	x = winReadProfileInt( "width", 640 );
	y = winReadProfileInt( "height", 480 );

	SetX(x,y,16,SETX_NORMAL);

	for( i=0; i<MAX_PARTICLES; i++ ) {				// Initials All The Textures
		// particle[i].active = TRUE;						// Make All The Particles Active
		particle[i].life = -1.0f;						// Give All The Particles Full Life
		particle[i].fade = 1.0f;	// Random Fade Speed
		// particle[i].fade = (FLOAT)(rand()%100)/1000.0f+0.003f;	// Random Fade Speed
		// particle[i].rgb.r = color[(i+1)/(MAX_PARTICLES/12)].r;	// Select Red Rainbow Color
		// particle[i].rgb.g = color[(i+1)/(MAX_PARTICLES/12)].g;	// Select Green Rainbow Color
		// particle[i].rgb.b = color[(i+1)/(MAX_PARTICLES/12)].b;	// Select Blue Rainbow Color
		// particle[i].rgb.a = color[(i+1)/(MAX_PARTICLES/12)].a;	// Select Blue Rainbow Color
		// ZEROVECTOR( particle[i].pos );
		// particle[i].xi = (FLOAT)((rand()%50)-26.0f)*10.0f;	// Random Speed On X Axis
		// particle[i].yi = (FLOAT)((rand()%50)-25.0f)*10.0f;	// Random Speed On Y Axis
		// particle[i].zi = (FLOAT)((rand()%50)-25.0f)*10.0f;	// Random Speed On Z Axis
		// particle[i].xg = 0.0f;							// Set Horizontal Pull To Zero
		// particle[i].yg = 0.8f;						// Set Vertical Pull Downward
		// particle[i].zg = 0.0f;							// Set Pull On Z Axis To Zero
		// particle[i].xc = SCREENW / 2;
		// particle[i].yc = SCREENH / 2;
	}

	i = 0;
	SETFLAG( i, TF_CHROMA );
	SETFLAG( i, TF_ALPHA );
	if( (texmapid = LoadTexmap( "particle.bmp", i, 0L )) == (-1) )
			Quit("LoadFont3D: no win95.bmp named texture for object.");

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(2);
	// HideMouse();

	MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
	MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );

	done = FALSE;
	while( !done ) {

		key = GetKey();

		if( scan_code == sc_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

//		particle_oldmousex = mousex;
//		particle_oldmousey = mousey;

		// ReadMouse();

		Clear( NOCOLOR ); // 0x00209f );

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
						 CLIPMAXX - CLIPMINX,
						 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);


		// control

		if( (GetTic() - rainbow_tic) > (TICKBASE / 3) ) {
			if( ++col > 11 ) col=0;				// If Color Is To High Reset It
			rainbow_tic = GetTic();
		}

		deltax = mousex - particle_oldmousex;
		deltay = mousey - particle_oldmousey;

		for( i=0; i<MAX_PARTICLES; i++ ) {					// Loop Through All The Particles

//			if( particle[i].active == TRUE ) {							// If The Particle Is Active

				// glTranslatef(particle[loop].x,particle[loop].y,particle[loop].z+zoom);
				// Draw The Particle Using Our RGB Values, Fade The Particle Based On It's Life
				// glColor4f(particle[loop].r,particle[loop].g,particle[loop].b,particle[loop].life);

				f = particle[i].life * 255.0f;
				particle[i].rgb.a = MAX(f,0.0f);

				if( f > 0.0f )
					DrawParticle( &particle[i], texmapid );

				particle[i].pos[0] += particle[i].xi / (slowdown*1000);	// Move On The X Axis By X Speed
				particle[i].pos[1] += particle[i].yi / (slowdown*1000);	// Move On The Y Axis By Y Speed
				particle[i].pos[2] += particle[i].zi / (slowdown*1000);	// Move On The Z Axis By Z Speed

				particle[i].xi += particle[i].xg;			// Take Pull On X Axis Into Account
				particle[i].yi += particle[i].yg;			// Take Pull On Y Axis Into Account
				particle[i].zi += particle[i].zg;			// Take Pull On Z Axis Into Account
				particle[i].life -= particle[i].fade;		// Reduce Particles Life By 'Fade'

				if( (particle[i].xi < 0.0f) && (particle[i].xg < 0.0f) ) { particle[i].xi = 0.0f; particle[i].xg = 0.0f; }
				if( (particle[i].xi > 0.0f) && (particle[i].xg > 0.0f) ) { particle[i].xi = 0.0f; particle[i].xg = 0.0f; }

				if( (mousex-particle_oldmousex==0) || (mousey-particle_oldmousey==0) )
					++idlecnt;

				if( (i<=BUTTON_PARTICLES) && (particle[i].life < 0.0f) && ((ABS(mousex-particle_oldmousex)>3) || (ABS(mousey-particle_oldmousey)>3) || ((idlecnt%20) == 0) ) ) {
					particle[i].life = 1.0f;					// Give It New Life
					particle[i].fade = (FLOAT)(rand()%100)/1000.0f+0.003f;	// Random Fade Value
					ZEROVECTOR( particle[i].pos );
					particle[i].xi = xspeed + 8*(FLOAT)((rand()%60)-32.0f);	// X Axis Speed And Direction
					particle[i].yi = yspeed + 8*(FLOAT)((rand()%60)-30.0f);	// Y Axis Speed And Direction
					particle[i].zi = (FLOAT)((rand()%60)-30.0f);	// Z Axis Speed And Direction
					if( particle[i].xi > 0.0f ) particle[i].xg = -1.0f; else particle[i].xg = 1.0f;
					particle[i].xg = 0.0f;							// Set Horizontal Pull To Zero
					particle[i].yg = 6.0f;						// Set Vertical Pull Downward
					particle[i].zg = 0.0f;							// Set Pull On Z Axis To Zero
					if( deltax > 0 ) { deltax -= 1; particle_oldmousex += 1; } if( deltax < 0 ) { deltax += 1; particle_oldmousex -= 1; }
					if( deltay > 0 ) { deltay -= 1; particle_oldmousey += 1; } if( deltay < 0 ) { deltay += 1; particle_oldmousey -= 1; }
					particle[i].xc = (particle_oldmousex + deltax);
					particle[i].yc = (particle_oldmousey + deltay);
					particle[i].rgb.r = color[col].r;			// Select Red From Color Table
					particle[i].rgb.g = color[col].g;			// Select Green From Color Table
					particle[i].rgb.b = color[col].b;			// Select Blue From Color Table
					particle[i].rgb.a = color[col].a;			// Select Blue From Color Table
				}

				if( (mousebr || mousebl) && (i>BUTTON_PARTICLES) && (particle[i].life < 0.2f) ) {
					particle[i].life = 1.0f;
					particle[i].fade = (FLOAT)(rand()%100)/500.0f+0.003f;	// Random Fade Value
					ZEROVECTOR( particle[i].pos );
					particle[i].xi = 6*(FLOAT)((rand()%50)-26.0f)*10.0f;	// Random Speed On X Axis
					particle[i].yi = 6*(FLOAT)((rand()%50)-25.0f)*10.0f;	// Random Speed On Y Axis
					particle[i].zi = (FLOAT)((rand()%50)-25.0f)*10.0f;	// Random Speed On Z Axis
					particle[i].xg = 0.0f;							// Set Horizontal Pull To Zero
					particle[i].yg = 16.0f;						// Set Vertical Pull Downward
					particle[i].zg = 0.0f;							// Set Pull On Z Axis To Zero
					particle[i].xc = mousex;
					particle[i].yc = mousey;
					particle[i].rgb.r = color[col].r;			// Select Red From Color Table
					particle[i].rgb.g = color[col].g;			// Select Green From Color Table
					particle[i].rgb.b = color[col].b;			// Select Blue From Color Table
					particle[i].rgb.a = color[col].a;			// Select Blue From Color Table
				}
//			}
		}

		particle_oldmousex += deltax;
		particle_oldmousey += deltay;

		// end particles

		DrawMouse();

		EndScene();

		WriteString(10,10,"memory used: %d\nframe/sec: %d\nframe cnt: %d\nvideo: %dx%dx%d",(int)mem_used,(int)frame,cnt++,SCREENW,SCREENH,bpp);

		ShowMouse();

		DrawMouse();

		FlipPage();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}


/*

//
//
//
void Mandel( void ) {

	int x,y,i,j;
	UCHAR pal[768];

	static float X = 0;
	static float Y = -.5;

	static float s = 1;

	j = 0;

	for( i=1; i<64; i++, j++ ) {
		pal[j*3+0] = 63;
		pal[j*3+1] = 0;
		pal[j*3+2] = 64-1;
	}

	for( i=0; i<64; i++, j++ ) {
		pal[j*3+0] = 64-1;
		pal[j*3+1] = 0;
		pal[j*3+2] = i;
	}

	for( i=0; i<64; i++, j++ ) {
		pal[j*3+0] = 0;
		pal[j*3+1] = i;
		pal[j*3+2] = 64-i;
	}

	for( i=0; i<64; i++, j++ ) {
		pal[j*3+0] = i;
		pal[j*3+1] = 64-i;
		pal[j*3+2] = i;
	}

	pal[0*3+0] = 0;
	pal[0*3+1] = 0;
	pal[0*3+2] = 0;

	pal[63*3+0] = 63;
	pal[63*3+1] = 0;
	pal[63*3+2] = 0;

	pal[127*3+0] = 0;
	pal[127*3+1] = 0;
	pal[127*3+2] = 63;

	pal[191*3+0] = 0;
	pal[191*3+1] = 63;
	pal[191*3+2] = 0;

	pal[255*3+0] = 63;
	pal[255*3+1] = 0;
	pal[255*3+2] = 63;

	if( 1 ) {

		float x1 = X-s;
		float y1 = Y-s;
		float x2 = X+s;
		float y2 = Y+s;

		float xd = (x2-x1) / 320;
		float yd = (y2-y1) / 200;

		float ci = y1;

		for( y=0; y<200; y++ ) {

			float cr = x1;

			for( x=0; x<320; x++ ) {

				float rq = 0;
				float iq = 0;

				float r = 0;
				float i = 0;

				int count = 0;

				while( (rq+iq < 4) && (count++ < 511) ) {
					i = 2*r*i + cr;
					r = rq - iq + ci;
					rq = r*r;
					iq = i*i;
				}

				CLAMPMINMAX( count, 0, 255 );

				// vga[ (y<<8) + (y<<6) + x ] = count;

				PutPixel( x, y, RGBINT( pal[count*3+0], pal[count*3+1], pal[count*3+2] ) );

				cr += xd;
			}

			ci += yd;
		}

		if( keys[sc_A] )
			s *= 0.7f;

		if( keys[sc_Z] )
			s *= 1.3f;

		if( keys[sc_B] )
			X -= s/3;

		if( keys[sc_M] )
			 X += s/3;

		if( keys[sc_H] )
			 Y -= s/3;

		if( keys[sc_N] )
			 Y += s/3;
	}

	return;
}


#define EXPORT_IN_MIDASDLL_H

#include <midasdll.h>


//
// Midas
//
void test39( void ) {

	BOOL done = FALSE;
	int x,y,key,cnt=0;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;
	FLOAT zclipnear=0.0f,zclipfar=250.0f;
	char *txt;

	if( !winOpenFile( "Protracker (.mod)\0*.mod\0Scream Tracker 3 (.s3m)\0*.s3m\0FastTracker 2 (.xm)\0*.xm\0Impulse Tracker (.it)\0*.it\0All Files (*.*)\0*.*\0") )
		Quit( NULL );

	if( (txt=winGetNextFile()) == NULL )
		return;

	if( !MIDASconfig() )
		Quit("midas");

	x = winReadProfileInt( "width", 640 );
	y = winReadProfileInt( "height", 480 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(2);
	// HideMouse();

	MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
	MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );

    MIDASmodule module=NULL;
    MIDASmodulePlayHandle modulePlayHandle=0,playHandle1=0;
	MIDASsample sample1=0;

    MIDASstartup();

	// MIDASsetOption( MIDAS_OPTION_DSOUND_HWND, (ULONG)GetHwnd() );
    // MIDASsetOption( MIDAS_OPTION_DSOUND_MODE, 2 );

//	if( !MIDASconfig() )
//		Quit("midas");

    if( !MIDASinit() )
		Quit("midas");

    if( !MIDASopenChannels(32) )
        Quit("midas");

    if( !MIDASallocAutoEffectChannels(4) )
        Quit("midas");

    if( (module = MIDASloadModule(txt)) == NULL )
        Quit("midas: MIDASloadModule.");

    if( (sample1 = MIDASloadWaveSample("proba.wav", MIDAS_LOOP_NO)) == 0 )
		xprintf("no wave.");

	if( (modulePlayHandle = MIDASplayModule(module, TRUE)) == 0 )
		Quit("midas: MIDASplayModule.");

	done = FALSE;
	while( !done ) {

		key = GetKey();

		if( scan_code == sc_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		if( key == ' ' || mousebl )
			playHandle1 = MIDASplaySample( sample1, MIDAS_CHANNEL_AUTO, 0, 22050, 64, MIDAS_PAN_MIDDLE);

		if( key == key_Enter || mousebr )
			MIDASstopSample( playHandle1 );

		Clear( NOCOLOR ); // 0x00209f );

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
						 CLIPMAXX - CLIPMINX,
						 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		DrawMouse();

		EndScene();

		WriteString(10,10,"MIDAS memory used: %d\nframe/sec: %d\nframe cnt: %d\nvideo: %dx%dx%d",(int)mem_used,(int)frame,cnt++,SCREENW,SCREENH,bpp);

		ShowMouse();

		DrawMouse();

		FlipPage();

		MIDASpoll();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}


    MIDASstopSample(playHandle1);
    MIDASfreeSample(sample1);

	if( !MIDASfreeAutoEffectChannels() )
		Quit("midas");

    if( modulePlayHandle && !MIDASstopModule(modulePlayHandle) )
		Quit("midas");

	if( module && !MIDASfreeModule(module) )
		Quit("midas");

	if( !MIDASclose() )
		Quit("midas");


	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}


*/




//
//
//
void test43( void ) {

	char *txt;
	int width,height,cnt,x;

	if( !winOpenFile( "AVI Files (*.avi)\0*.avi\0All Files (*.*)\0*.*\0") )
		Quit( NULL );

	width = winReadProfileInt( "xlib_width", 640 );
	height = winReadProfileInt( "xlib_height", 480 );

	SetX(width,height,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);

	if( (txt = winGetNextFile()) != NULL ) {

		if( AVI_Open( txt ) == TRUE ) {

			HideMouse();

			cnt = 0;
			while( cnt < AVI_GetNumFrame() && !GetKey() && !mousebl && !mousebr && !mousebm ) {

				// Clear(NOCOLOR);

				int tic = GetTic();
				AVI_DrawFrame( cnt++, -1,-1,-1,-1 );

				DrawMouse();

				do { DX_GetMessage(); } while( tic+(TICKBASE/AVI_GetFps()) > GetTic() );

				// FlipPage();
			}

			AVI_Close();

			ShowMouse();
		}
	}

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}






#define STARTEXNAME "lens.bmp"


//
//
//
static BOOL drawBackStar( int x, int y, FLOAT scale, int r, int g, int b, int a ) {

	polygon_t poly;
	point_t point[4];
	rgb_t rgb[4] = { {255,255,255,255}, {255,255,255,255}, {255,255,255,255}, {255,255,255,255} };
	// point3_t st[4] = { {0.0f,0.0f,0.0f}, {0.0f,255.0f,0.0f}, {255.0f,255.0f,0.0f}, {255.0f,0.0f,0.0f} };
	// point3_t st[4] = { {0.0f,0.0f,0.0f}, {0.0f,64.0f,0.0f}, {64.0f,64.0f,0.0f}, {64.0f,0.0f,0.0f} };
	point3_t st[4] = { {192.0f,64.0f,0.0f}, {192.0f,128.0f,0.0f}, {255.0f,128.0f,0.0f}, {255.0f,64.0f,0.0f} };
	int alpha,tex,tw,th;
	texmap_t *texmap = NULL;

	if( (tex = TexNumForName( STARTEXNAME )) == (-1) ) {

		int flag,color;
		UCHAR pal[768],*spr;

		LoadPicture( STARTEXNAME, PTR(spr) );
		GetRgbPicture( pal );

		flag = 0L;
		color = NOCOLOR;

		SETFLAG( flag, TF_CHROMA );
		SETFLAG( flag, TF_ALPHA );

		AddTexMapHigh( spr, pal, STARTEXNAME, STARTEXNAME, flag, color );

		if( (tex = TexNumForName( STARTEXNAME )) == (-1) ) {
				xprintf("drawStar: no \"%s\" textures.", STARTEXNAME );
				return FALSE;
		}

		FREEMEM( spr );
	}

	texmap = TexForNum( tex );
	tw = texmap->width1;
	th = texmap->height1;

	tw = 32;
	th = 32;

	poly.npoints = 4;
	poly.point = point;

	alpha = a;
	CLAMPMINMAX( alpha, 0, 255 );

	rgb[0].a = alpha;
	rgb[1].a = alpha;
	rgb[2].a = alpha;
	rgb[3].a = alpha;

	rgb[0].r = r; rgb[1].r = r; rgb[2].r = r; rgb[3].r = r;
	rgb[0].g = g; rgb[1].g = g; rgb[2].g = g; rgb[3].g = g;
	rgb[0].b = b; rgb[1].b = b; rgb[2].b = b; rgb[3].b = b;

	point[0][0] = x;		point[0][1] = y;
	point[1][0] = x;		point[1][1] = y+(FLOAT)th*scale;
	point[2][0] = x+(FLOAT)tw*scale;	point[2][1] = y+(FLOAT)th*scale;
	point[3][0] = x+(FLOAT)tw*scale;	point[3][1] = y;

	PutSpritePoly( poly, st, tex, rgb );

	return TRUE;
}


typedef struct star_s {

	int x,y;
	FLOAT scale;
	int r,g,b,a;

	int dx,dy;

	BOOL inited;

} star_t, *star_ptr;


#define MAXSTAR 80
#define MAXCOLOR 100

static star_t stars[MAXSTAR];

//
//
//
void starBackdrop( void ) {

	int i;
	static BOOL inited = FALSE;

	if( inited == FALSE ) {

		for( i=0; i<MAXSTAR; i++ ) {
			stars[i].inited = FALSE;
			stars[i].x = rand()%SCREENW;
			stars[i].dx = 0;
			stars[i].y = rand()%SCREENH;
			stars[i].dy = 2+rand()%(SCREENH/80);
			stars[i].scale = 1.0f;
			stars[i].inited = TRUE;
			stars[i].r = MAXCOLOR+rand()%(255-MAXCOLOR);
			stars[i].g = stars[i].r;
			stars[i].b = stars[i].r;
			stars[i].a = 255; // stars[i].r;
		}

		inited = TRUE;
	}

	for( i=0; i<MAXSTAR; i++ )
		if( stars[i].inited == FALSE ) {
			stars[i].x = rand()%SCREENW;
			stars[i].dx = 0;
			stars[i].y = 0;
			stars[i].dy = 2+rand()%(SCREENH/80);
			stars[i].scale = 1.0f;
			stars[i].inited = TRUE;
			stars[i].r = MAXCOLOR+rand()%(255-MAXCOLOR);
			stars[i].g = stars[i].r;
			stars[i].b = stars[i].r;
			stars[i].a = 255; // stars[i].r;
		}


	for( i=0; i<MAXSTAR; i++ ) {

		if( stars[i].inited == TRUE )
			drawBackStar( stars[i].x, stars[i].y, stars[i].scale, stars[i].r,stars[i].g,stars[i].b,stars[i].a );

		stars[i].x += stars[i].dx;
		stars[i].y += stars[i].dy;

		if( stars[i].y > SCREENH )
			stars[i].inited = FALSE;
	}

	return;
}




#define FNAME		"FLASHTEX"
#define FLASHTIC	(TICKBASE*0.45)
static int flash_tic = (-FLASHTIC);

//
//
//
void InitFlash( void ) {

	flash_tic = GetTic();

	xprintf("flash %d sec ...\n",flash_tic/TICKBASE);

	return;
}



//
//
//
BOOL DoFlash( void ) {

	polygon_t poly;
	point_t point[4];
	rgb_t rgb[4] = { {255,255,255,255}, {255,255,255,255}, {255,255,255,255}, {255,255,255,255} };
	point3_t st[4] = { {40.0f,10.0f,0.0f}, {40.0f,20.0f,0.0f}, {50.0f,20.0f,0.0f}, {50.0f,10.0f,0.0f} };
	int alpha,tex,tic;

	tic = GetTic();

	if( (flash_tic + FLASHTIC) < tic )
		return FALSE;

	if( (tex = TexNumForName( FNAME )) == (-1) ) {

		int flag,color;
		UCHAR pal[768],*spr;

		CheckerTexture( &spr, 32,32 );

		flag = 0L;
		color = 0L;

		AddTexMapHigh( spr, pal, FNAME, FNAME, flag, color );

		if( (tex = TexNumForName( FNAME )) == (-1) ) {
				xprintf("showSplash: no \"%s\" textures.", FNAME );
				return FALSE;
		}

		FREEMEM( spr );
	}

	poly.npoints = 4;
	poly.point = point;

	alpha = 255.0 * (tic - flash_tic) / (FLASHTIC*0.5);
	CLAMPMINMAX( alpha, 0, 255 );

	rgb[0].a = alpha;
	rgb[1].a = alpha;
	rgb[2].a = alpha;
	rgb[3].a = alpha;

	point[0][0] = 0;		point[0][1] = 0;
	point[1][0] = 0;		point[1][1] = SCREENH;
	point[2][0] = SCREENW;	point[2][1] = SCREENH;
	point[3][0] = SCREENW;	point[3][1] = 0;

	PutSpritePoly( poly, st, tex, rgb );

	return TRUE;
}


/***
#include <libintl.h>

#define _(String) gettext(String)

#pragma comment(lib, "intl.lib" )
***/

//
// A: 1,1 36,50 sima semmi
//
void test44( void ) {

	BOOL done = FALSE;
	int x,y,key,cnt=0,i,tic;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;
	light_t *sun;

	// UCHAR *buf = NULL;
	// int size;
	// char *url = "http://index.hu/tech/mobil/naiv2";
	// extern int winGetUrlDocument( char *url, UCHAR *buf );

	// ALLOCMEM( buf, 100000 );
	// memset( buf, 0L, 100000 );

	// if( size = winGetUrlDocument( url, buf ) > 0 ) {
		// TomFlag( NOFLAG );
		// WriteFile( "1.html", buf, size );
	// }

	// FREEMEM( buf );

	// localize
// #define LOCALEDIR	"."
// #define PACKAGE	NULL

	// setlocale( LC_ALL, "" );
	// bindtextdomain( PACKAGE, LOCALEDIR );
	// textdomain( PACKAGE );

	// xprintf(_("Hello World.\n"));
	// localize

	char *str = "bernie";
	FLOAT beta = 0, gamma;
	int betu_tic;

	x = winReadProfileInt( "width", 800 );
	y = winReadProfileInt( "height", 600 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);

	MAKEVECTOR( cam_view, 0.0, 1.0, 0.0 );
	MAKEVECTOR( cam_pos, 0.0, 0.0, 0.0 );

	CV_SetValue( "zclipfar", 150.0f );
	CV_SetValue( "zclipnear", 0.0f );

	sun = AddLight( LT_AMBIENT, 150,150,150,255, 100.0f, NULL, NULL );

	// TomFlag(NOFLAG);
	// SaveAsc( tunnel[0].obj_ptr );

	tic = GetTic();
	betu_tic = tic;

	done = FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		// if( key == key_Escape ) done = TRUE;
		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		if( mousebl )
			InitFlash();

		// render

		Clear( NOCOLOR ); // 0x00209f  NOCOLOR

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );
		SetupFog( zclipfar.value/2.0f, zclipfar.value, 0x000000 );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		FlushScene();

		// betûk BEGIN

		int tic = GetTic();
		int anim_wait = (int)( (FLOAT)TICKBASE / 1 );

		FLOAT scale = (FLOAT)(tic - betu_tic) / (FLOAT)anim_wait;

		if( (tic - betu_tic) > anim_wait )
			betu_tic = tic;

		beta = (360.0 * scale);

		while( beta >= 360.0 )
			beta -= 360.0;

		SetFont( 1 );
		SetFontAlpha( FA_ALL, 255,255,255, 255 );

		for( i=0; i<strlen(str); i++ ) {

			gamma = i*45 + beta;

			y = 64.0 * GetSin(deg2rad(gamma))/5.0 + SCREENH/4;
			x = 32.0 * GetCos(deg2rad(gamma))/5.0 + SCREENW/4;

			WriteChar( x+((FontWidth()*2.0)*i), y, str[i] );
		}

		// betûk END

		// csillagok

		starBackdrop();

		// csillagok


		DoFlash();

		consoleAnimate();

		DrawMouse();

		EndScene();

		FlipPage();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}



// video game name creator: http://www.norefuge.net/vgng/vgng.html

int progressbar_x1 = 0;
int progressbar_y1 = 0;

//
//
//
int InitProgressBar( int x1, int y1, int x2, int y2 ) {

	return TRUE;
}


//
//
//
void ProgressBar( int percent ) {

	return;
}


#include <xseer.h>
#include <xlua.h>


//
//
//
static int SC_ProgressCallback( int percent ) {

	// int i;
	static int per=-1;

	// percent /= 5;

	// don't repeat yourself
	if( per == percent )
		return 1;

	per = percent;

	// xprintf("\b\b\b%2d%%",per);

	// xprintf("\r|");

	// for( i=0; i<percent; i++ ) xprintf("*");
	// for( ; i<100/5; i++ ) xprintf(".");

	// xprintf("|");

	return 1;
}


//
//
//
static int whatIf( char *ptr_this, char *str ) {

	xprintf("whatIf: \"%s\"\n", str );

	return 7;
}



//
//
//
static int whatIf2( char *ptr_this, char *str ) {

	xprintf("whatIf2: \"%s\"\n", str );

	return 7;
}


extern "C" {
#include "data/proba_script.h"
int  tolua_proba_script_open (lua_State*);
}


int a[10] = {1,2,3,4,5,6,7,8,9,10};
Point p[10] = {{0,1},{1,2},{2,3},{3,4},{4,5},{5,6},{6,7},{7,8},{8,9},{9,10}};
Point* pp[10];

int ma[10];
Point mp[10];
Point* mpp[10];

Array array;
Array* parray = &array;

int b = 2;
int c = 3;
int d = 4;
int e = 1;


Status checkenum (Order o)
{
	if (o == FIRST)
		return myTRUE;
	else
		return myFALSE;
}


//
// http://home.elka.pw.edu.pl/~ppodsiad/seer/
//
void test47( void ) {

	BOOL done = FALSE;
	int x,y,key,cnt=0,tic;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;
	light_t *sun;

	// Message("Armadillo: %d\n", isArmadillo() );

	x = winReadProfileInt( "xlib_width", -1 );
	y = winReadProfileInt( "xlib_height", -1 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);

	MAKEVECTOR( cam_view, 0.0, 1.0, 0.0 );
	MAKEVECTOR( cam_pos, 0.0, 0.0, 0.0 );

	CV_SetValue( "zclipfar", 150.0f );
	CV_SetValue( "zclipnear", 0.0f );

	sun = AddLight( LT_AMBIENT, 150,150,150,255, 100.0f, NULL, NULL );

	tic = GetTic();

	InitSpeak();

	// scripty

	typedef struct { int a,b,c,d; } module_t;
	typedef int (*memberf)( char *ptr_this, char *str );

#pragma pack(push, 1)

	typedef struct proba_s {
		int a;
		FLOAT b;
		double c;
		char d1 ,d2;
		struct { int x,y; } inside;
		char name[32];
		module_t mod;
		memberf mf;
	} proba_t PACKED;

#pragma pack(pop)

	proba_t proba = { 1, 2.0, 3.0, 64,65, {1,2}, "Az elso", {1,2,3,4}, whatIf };
	proba_t proba2 = { 2, 3.5, 5.6, 78,95, {4,7}, "A masodik", {4,3,2,1}, whatIf2 };

	scScript script = NULL;
	scInstance *prog = NULL, *prog2 = NULL;

	SC_InitSeeR();

	SC_Set( scSeeR_ProgressCallback, SC_ProgressCallback );

	if( ((script = SC_CompileFile("hello.sc")) == NULL) || SC_ErrorNo() )
		Quit("Error: %s\nLine: %s\n",SC_ErrorMsg(),SC_ErrorLine());

	// compile 1.

	// SC_AddExtSym( proba );
	// SC_AddMemberSymbol( "structuretype_t.mf", proba.mf );

	if( (prog = SC_CreateInstance( script, "" )) == NULL )
		Quit("Seer: Error creating instance 1.");

	SC_NameVar( prog,"proba", proba_t* ) = &proba;

	// compile 2.

	// SC_AddExternalSymbol( "proba", &proba2 );
	// SC_AddMemberSymbol( "structuretype_t.mf", proba2.mf );

	if( (prog2 = SC_CreateInstance( script, "" )) == NULL )
		Quit("Seer: Error creating instance 2.");

	SC_NameVar( prog2,"proba", proba_t* ) = &proba2;


	// Lua
	LUA_Init();

	done = FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		// elso
		if( (key == 'a') || mousebl ) {

			xprintf("*** ELSO **********\n");

			if( prog != NULL ) {

				// variable

				SC_NameVar( prog, "life", int ) = 24;
				xprintf("life = %d\nlebego = %.2f\n", SC_NameVar( prog, "life", int ), SC_NameVar( prog, "lebego", FLOAT ) );

				// run

				SC_CallInstance( prog, SC_GetSymbol(prog,"main"), "Elso" );

				if( SC_ErrorNo() )
					xprintf("Seer: %s\n",SC_ErrorMsg());

				xprintf("utan: \"%s\"\n", proba.name );
			}

			// Lua
			xprintf("LUA_RunFile(test.lua):\n");
			LUA_RunFile( "test.lua" );

			lua_State *Lua_VM = LUA_GetVM();

			LUA_CheckGlobal( "level" );

			// 1: get the function name
			// 2: push the value(s) (from left to right)
			// 3: call the function
			// 4: pop the return value(s)
			lua_getglobal(Lua_VM, "xprintf");
			lua_pushstring(Lua_VM, __FILE__);
			lua_pushstring(Lua_VM, ": helloka!\n");
			lua_call(Lua_VM, 2, 1);
			lua_pop(Lua_VM, 1);


			// 1: get the function name
			// 2: push the value(s) (from left to right)
			// 3: call the function
			// 4: pop the return value(s)
			lua_getglobal(Lua_VM, "double");
			lua_pushnumber(Lua_VM, 3);
			lua_call(Lua_VM, 1, 1);
			xprintf("double returend: %d\n", lua_tointeger(Lua_VM,-1) ); // Should be 6 :-)
			lua_pop(Lua_VM,1);


			// 2 arg
			lua_getglobal(Lua_VM, "f");
			lua_pushnumber(Lua_VM, 21);   /* push 1st argument */
			lua_pushnumber(Lua_VM, 31);   /* push 2nd argument */
			/* do the call (2 arguments, 1 result) */
			lua_pcall(Lua_VM, 2, 1, 0);
			xprintf("Result: %f\n",lua_tonumber(Lua_VM, -1));
			lua_pop(Lua_VM, 1);


			// table
			lua_getglobal(Lua_VM, "level");
			lua_pushnil(Lua_VM);

			xprintf("level = {");

			while(lua_next(Lua_VM, -2)) {
				if(lua_isnumber(Lua_VM, -1)) {
					int i = (int)lua_tonumber(Lua_VM, -1);
					//use number
					xprintf("%d, ", i );
				}
				lua_pop(Lua_VM, 1);
			}
			lua_pop(Lua_VM, 1);

			xprintf("}\n");

			SpeakText( "hello, press start button please" );
		}

		// masodik
		if( (key == 'b') || mousebr ) {

			xprintf("*** MASODIK **********\n");

			if( prog2 != NULL ) {

				// variable

				SC_NameVar( prog2, "life", int ) = 48;
				xprintf("life = %d\nlebego = %.2f\n", SC_NameVar( prog, "life", int ), SC_NameVar( prog, "lebego", FLOAT ) );

				// run

				SC_CallInstance( prog2, SC_GetSymbol(prog2,"main"), "Masodik" );

				if( SC_ErrorNo() )
					xprintf("Seer: %s\n",SC_ErrorMsg());

				xprintf("utan: \"%s\"\n", proba2.name );

				SpeakText( "hello lua" );
			}

			// Lua
			xprintf("LUA_RunString(...):\n");
			LUA_RunString( "a = 1 + 1;\nxprintf (\"1+1=\" .. a);\n" );


			/// tolue start
			/*** int i;
			lua_State *Lua_VM = LUA_GetVM();


			for (i=0; i<10; ++i)
			{
				pp[i] = &p[i];

				ma[i] = a[i];
				mp[i] = p[i];
				mpp[i] = pp[i];

				array.a[i] = a[i];
				array.p[i] = p[i];
				array.pp[i] = pp[i];
			}

			tolua_proba_script_open(Lua_VM);

			LUA_RunFile("data\\proba_script.lua"); ***/
			/// tolua end
		}

		// render

		Clear( NOCOLOR ); // 0x00209f  NOCOLOR

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );
		SetupFog( zclipfar.value/2.0f, zclipfar.value, 0x000000 );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		FlushScene();

		consoleAnimate();

		DrawMouse();

		EndScene();

		FlipPage();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	// scripty END

	LUA_Deinit();

	if( prog ) SC_FreeInstance(prog);
	if( prog2 ) SC_FreeInstance(prog2);
	if( script ) SC_FreeScript(script);

	SC_DeinitSeeR();

	DeinitSpeak();

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}


tooltip_t mytooltip[] = {
	{ 10,10,90,90, "Ez az elso tooltip" },
	{ 100,100, 90,90, "Ez Egy: \"blabla-blabla\", Teljesen Masik Tooltip!" },
	{ 30,400, 190,90, "egy szinte teljesen elrejtett TOOLTIP kovetkezik" },
	{ 600,40, 190,190, "ez egy kilogos TOOLTIP kovetkezik" },
	{ 0,0, 50,280, "abcdefghijklmnopqrstvwxyz 123456789 \n\tbla-bla-bla    bla-bla-bla \nbla-bla-bla" },
};



//
//
//
static void soundcallback( void *ptr, int len ) {

	UCHAR *buf = (UCHAR *)ptr;

	for( int i=0; i<len/2; i++ )
		buf[i] = i;

	for( int i=len/2; i<len; i++ )
		buf[i] = len/2-(i-len/2);

	return;
}


// #include "c:/MYPROJ~1/BOX2D_~1.2/Box2D/Box2D/Box2D.h"

UCHAR *maze;
#define MAZE(x,y) maze[ SPRITEHEADER + SPRITEW(maze) * (y) + (x) ]

UCHAR *washere;
#define WASHERE(x,y) washere[ SPRITEW(maze) * (y) + (x) ]

UCHAR *correctpath;
#define CORRECTPATH(x,y) correctpath[ SPRITEW(maze) * (y) + (x) ]

point_t start = {2,2};
point_t end = {510,510};

#define WALL 0
#define FLOOR 1
#define PATH 2


//
//
//
BOOL mazeSolve( int x, int y ) {

	// xprintf("pos: [%d, %d] = %d\n",x,y,MAZE(x,y));

	if( x == end[0] && y == end[1] )
		return TRUE; // If you reached the end

	if( MAZE(x,y) == WALL || WASHERE(x,y) )
		return FALSE;

	// If you are on a wall or already were here
	WASHERE(x,y) = TRUE;

	if( x != 0 ) // Checks if not on left edge
		if( mazeSolve(x-1, y) ) { // Recalls method one to the left
			CORRECTPATH(x,y) = TRUE; // Sets that path value to true;
			return TRUE;
		}

	if( x != SPRITEW(maze) - 1 ) // Checks if not on right edge
		if( mazeSolve(x+1, y) ) { // Recalls method one to the right
			CORRECTPATH(x,y) = TRUE;
			return TRUE;
		}

	if( y != 0 )  // Checks if not on top edge
		if( mazeSolve(x, y-1)) { // Recalls method one up
			CORRECTPATH(x,y) = TRUE;
			return TRUE;
		}

	if( y != SPRITEH(maze) - 1) // Checks if not on bottom edge
		if( mazeSolve(x, y+1) ) { // Recalls method one down
			CORRECTPATH(x,y) = TRUE;
			return TRUE;
		}

	return FALSE;
}


//
//
//
void mazeDo( void ) {

	UCHAR *spr;

	if( LoadPicture( "C:/MYDATA~1/Objects/Blizzard/fractal.gif", PTR(spr) ) == FALSE )
		Quit("cannot load.");

	ALLOCMEM( maze, SPRITEHEADER + SPRITEW(spr) * SPRITEH(spr) );
	memset( maze, 0L, SPRITEHEADER + SPRITEW(spr) * SPRITEH(spr) );
	MKSPRW( maze, SPRITEW(spr) );
	MKSPRH( maze, SPRITEH(spr) );
	MKSPR8( maze );

	UCHAR *p = &maze[SPRITEHEADER];
	for( int y=0; y<SPRITEH(spr); y++ )
	for( int x=0; x<SPRITEW(spr); x++ )
		if( SPR(spr,x,y) > 0 )
			*p++ = FLOOR;
		else
			*p++ = WALL;

	FREEMEM( spr );

	xprintf("maze: %d, %d,  %d\n", SPRITEW(maze), SPRITEH(maze), SPRPIXELLEN(maze) );

	ALLOCMEM( washere, SPRITEW(maze)*SPRITEH(maze) );
	memset( washere, 0L, SPRITEW(maze)*SPRITEH(maze) );

	ALLOCMEM( correctpath, SPRITEW(maze)*SPRITEH(maze) );
	memset( correctpath, 0L, SPRITEW(maze)*SPRITEH(maze) );

	xprintf("==================== solving:\n");

	if( mazeSolve(start[0], start[1]) == FALSE )
		Message("cannot solve");

	p = &maze[SPRITEHEADER];
	for( int y=0; y<SPRITEH(maze); y++ )
	for( int x=0; x<SPRITEW(maze); x++ ) {
		if( CORRECTPATH(x,y) )
			*p = PATH;
		++p;
	}

	UCHAR pal[256*3] = {0,0,0,  255,255,255, 200,0,0};

	SaveFree("C:/solved.png",maze, pal);


	FREEMEM( maze );
	FREEMEM( washere );
	FREEMEM( correctpath );

	return;
}


//
// Box2d
//
void test53( void ) {

	BOOL done = FALSE;
	int x,y,key,cnt=0,tic;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;

	x = winReadProfileInt( "xlib_width", -1 );
	y = winReadProfileInt( "xlib_height", -1 );

	SetX(x,y,32,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);

	MAKEVECTOR( cam_view, 0.0, 1.0, 0.0 );
	MAKEVECTOR( cam_pos, 0.0, 0.0, 0.0 );

	CV_SetValue( "zclipfar", 150.0f );
	CV_SetValue( "zclipnear", 0.0f );

	tic = GetTic();

	// Box2s

	xprintf("elso: %s\n", GetString(1) );
	xprintf("harm: %s\n", GetString(100) );
	xprintf("negy: %s\n", GetString(101) );
	xprintf("otod: %s\n", GetString(103) );
	xprintf("msdk: %s\n", GetString(103248) );

	// MM_Open( (void *)soundcallback, 1000 );

	// mazeDo();

	done = FALSE;
	while( !done && !winWantToQuit() ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		CHECKCONSOLE(key);

		// elso
		if( (key == 'a') || mousebl ) {

			// Overlay( "left button: %d", mousebl );

			OpenZipFile( "proba.zip" );

			// PLAYSOUNDPARAMOAL( "data\\laser1.wav", MIDVOL, MAXPAN*mousex/SCREENW, -1 );
			// PLAYSOUNDVOLOAL( "data\\laser1.wav", rand() % MAXVOL );
			// PLAYSOUNDVOLOAL( "c:\\CDiro\\Music\\CRIMSO~1.OGG", rand() % MAXVOL );
			// PLAYSOUNDVOLOAL( "c:\\CDiro\\Music\\SZCSIP~1.OGG", rand() % MAXVOL );

			// PLAYSOUNDPARAMMIKMOD( "data\\laser1.wav", MIDVOL, MAXPAN*mousex/SCREENW, -1 );
			// PLAYSOUNDVOLMIKMOD( "data\\laser1.wav", rand() % MAXVOL );

			// PLAYSOUNDPARAMSDL( "data\\laser1.wav", MIDVOL, MAXPAN*mousex/SCREENW, -1 );
			PLAYSOUNDVOLSDL( "data\\laser1.wav", rand() % MAXVOL );


			xprintf("*** ELSO **********\n");
		}

		// masodik
		if( (key == 'b') || mousebr ) {

			// Overlay(NULL);

			CloseZipFile( "proba.zip" );

			DiscardAllBigSprite();

			// PLAYSOUNDMIKMOD( "button.wav" );
			// PLAYSOUNDOAL( "button.wav" );

			// MPP_PlayMod( "c:\\MYDATA~1\\Music\\Mods\\Games\\AGONY_~1.MOD" );

			// OAL_FreeWave( OAL_LoadWave( "c:\\CDiro\\Music\\SZCSIP~1.OGG" ) );

			xprintf("*** MASODIK **********\n");
		}

		// render

		Clear( NOCOLOR ); // 0x00209f  NOCOLOR

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );
		SetupFog( zclipfar.value/2.0f, zclipfar.value, 0x000000 );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		DrawChecker();

		FlushScene();

		if( 1 && mousebld ) {

			PUTPICTURE( "data\\orion_star.jpg", SCREENW/2, SCREENH/2 );
			PUTPICTUREPARAM( "xraydiscovery2010-ps.gif", mousex + 30, mousey - (120+30) , TRUE, TRUE );
			PUTPICTUREPARAMSCALE( "Anim1.bmp", 10, 10, FALSE, FALSE, mousex - 10, mousey - 30 );
			PUTPICTUREPARAMSCALE( "c:\\pwndlol.jpg", 10, mousey+30, FALSE, TRUE, mousex - 10, SCREENH - (mousey + 50) );
			PUTPICTUREFADE( "reactor.jpg", SCREENW/2 - 320, SCREENH/2 - 240, 255 - 255*mousey/SCREENH );
		}

		PUTPICTURE( "c:\\MYDATA~1\\Textures\\8bit\\zxava.gif", mousex, mousey );

		if( key == 'c' || mouseb4 || mouseb5 ) {
			LUA_RunFile("c:/MYPROJ~1/Doc/Name/NameGen.lua");
			P64(LUA_GetResultString());
		}

		DrawTooltip( mytooltip, dimof(mytooltip),TRUE );

		SetFont(1);
		SetFontAlpha( FA_ALL, 255,255,255, 255 );

		// DrawTexRect( mousex, mousey, SCREENW/2, SCREENH/2, 5 );
		// DrawTexLine( mousex, mousey, SCREENW/2, SCREENH/2,  3, 2 );

		rgb_t color = { 200,200,200, 255 };
		DrawArrow( 5,5, mousex, mousey, color );

		consoleAnimate();

		DrawMouse();

		EndScene();

		FlipPage();
		// DX_GetMessage();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	MM_Close();

	DEINITSOUND;

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}


/*
extern "C" BOOL my_load_mp3( char *filename );
extern "C" BOOL mp3_poll( void );

//
//
//
void test45( void ) {

	int done=0,key,handle,pan,i;
	char *txt;

	if( !winOpenFile( "MP3 Files (*.mp3)\0*.mp3\0All Files (*.*)\0*.*\0") )
		Quit( NULL );

	if( (txt = winGetNextFile()) == NULL )
		Quit( "no file" );

	InitKeyboard();

	SetX(640,480,16,SETX_NORMAL);

	i = 0;
	SETFLAG( i, MF_3DCURSOR );
	if(!InitMouse(0)) xprintf("no mouse!");
	// HideMouse();

	DxS_Init();

	if( my_load_mp3( txt ) == FALSE )
		Quit("no %s file.", txt );

	while( !done ) {

		HideMouse();
		Clear( 0x00209f );
		ShowMouse();

		key = GetKey();

		if( key == key_Escape )
			done = 1;

		pan = mousex * MAXPAN / SCREENW;

		mp3_poll();

		if( key == key_Enter ) {
			MIDI_PlayFile( "enig.mid" );
			xprintf("playing...\n");
		}

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		DrawMouse();

		WriteString(10,10,"memory used: %d\nvideo: %dx%dx%d\nfile: %s\npan: %d / %d",(int)mem_used,SCREENW,SCREENH,bpp,txt,pan,MAXPAN);

		FlipPage();
	}

	DxS_Deinit();

	ResetX();

	return;
}


//
// Direct Music
//
void test48( void ) {

	BOOL done = FALSE;
	int x,y,c,key,cnt=0,i,tic,gametic;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;
	light_t *sun;
	char *txt;

	if( !winOpenFile( "All Music Files\0*.mp3;*.mid\0All Files (*.*)\0*.*\0") )
		Quit( NULL );

	if( (txt=winGetNextFile()) == NULL )
		return;

	DM_Init();

	x = winReadProfileInt( "width", 640 );
	y = winReadProfileInt( "height", 480 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);

	MAKEVECTOR( cam_view, 0.0, 1.0, 0.0 );
	MAKEVECTOR( cam_pos, 0.0, 0.0, 0.0 );

	CV_SetValue( "zclipfar", 150.0f );
	CV_SetValue( "zclipnear", 0.0f );

	sun = AddLight( LT_AMBIENT, 150,150,150,255, 100.0f, NULL, NULL );

	DM_Load( txt );
	DM_Play();

	tic = GetTic();

	done = FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		// if( key == key_Escape ) done = TRUE;
		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		if( mousedy ) {
			int vol = (int)(MAXVOL * mousey / SCREENH);
			xprintf( "setvol: %d\n", vol );
			DM_SetVol( vol );
		}

		// render

		Clear( NOCOLOR ); // 0x00209f  NOCOLOR

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );
		SetupFog( zclipfar.value/2.0f, zclipfar.value, 0x000000 );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		FlushScene();



		consoleAnimate();

		DrawMouse();

		EndScene();

		FlipPage();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	DM_Deinit();

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}

***/


//=================== BSpline calculations =================================
// out_p - BSpline curve point (time t)
// T - BSpline curve tangent, normalized (time t)
// t - current time (paramentr) should be: 0..1
//==========================================================================
void BSpline( point3_t *p, FLOAT t, point3_t *out_p, point3_t *T ) {

	FLOAT t2, t3, tm1, d, tn;
	point3_t der;

	t2 = t*t;
	t3 = t2*t;
	tn = 3*t3-6*t2+4;
	tm1 = (1-t)*(1-t)*(1-t);

	(*out_p)[0] = (tm1/6)*p[0][0] + ((tn)/6)*p[1][0] + ((-3*t3+3*t2+3*t+1)/6)*p[2][0] + (t3/6)*p[3][0];
	(*out_p)[1] = (tm1/6)*p[0][1] + ((tn)/6)*p[1][1] + ((-3*t3+3*t2+3*t+1)/6)*p[2][1] + (t3/6)*p[3][1];
	(*out_p)[2] = (tm1/6)*p[0][2] + ((tn)/6)*p[1][2] + ((-3*t3+3*t2+3*t+1)/6)*p[2][2] + (t3/6)*p[3][2];

	// First derivative
	der[0] = (-0.5*t2+t-0.5)*p[0][0] + (1.5*t2-2*t)*p[1][0] + (-1.5*t2+t+0.5)*p[2][0] + 0.5*t2*p[3][0];
	der[1] = (-0.5*t2+t-0.5)*p[0][1] + (1.5*t2-2*t)*p[1][1] + (-1.5*t2+t+0.5)*p[2][1] + 0.5*t2*p[3][1];
	der[2] = (-0.5*t2+t-0.5)*p[0][2] + (1.5*t2-2*t)*p[1][2] + (-1.5*t2+t+0.5)*p[2][2] + 0.5*t2*p[3][2];

	(*T)[0] = der[0];
	(*T)[1] = der[1];
	(*T)[2] = der[2];

	// Vector length
	d = fsqrt( (*T)[0]*(*T)[0] + (*T)[1]*(*T)[1] + (*T)[2]*(*T)[2]);

	// Normalization
	(*T)[0] /= d;
	(*T)[1] /= d;
	(*T)[2] /= d;

	return;
}



//
// 4 kontrol ponttal
// http://astronomy.swin.edu.au/pbourke/curves/bezier/
// clotoid
//
void PointOnCurve( point3_t p1, point3_t p2, point3_t p3, point3_t p4, FLOAT t, point3_t *out ) {

	FLOAT var1, var2, var3;
    point3_t vPoint = { 0.0f, 0.0f, 0.0f };

	// bezier curve:
	// B(t) = P1 * ( 1 - t )^3 + P2 * 3 * t * ( 1 - t )^2 + P3 * 3 * t^2 * ( 1 - t ) + P4 * t^3

	// Store the (1 - t) in a variable because it is used frequently
    var1 = 1 - t;

	// Store the (1 - t)^3 into a variable to cut down computation and create clean code
    var2 = var1 * var1 * var1;

	// Store the t^3 in a variable to cut down computation and create clean code
    var3 = t * t * t;

    (*out)[0] = var2*p1[0] + 3*t*var1*var1*p2[0] + 3*t*t*var1*p3[0] + var3*p4[0];
    (*out)[1] = var2*p1[1] + 3*t*var1*var1*p2[1] + 3*t*t*var1*p3[1] + var3*p4[1];
    (*out)[2] = var2*p1[2] + 3*t*var1*var1*p2[2] + 3*t*t*var1*p3[2] + var3*p4[2];

    return;
}



//
// Number of control points is n+1
// 0 <= mu < 1 IMPORTANT, the last point is not computed
//
void Bezier( point3_t *p, int n, FLOAT mu, point3_t *b ) {

	int k,kn,nn,nkn;
	FLOAT blend,muk,munk;

	--n;
	muk = 1;
	munk = pow( 1-mu,(double)n );

	for( k=0; k<=n; k++ ) {

		nn = n;
		kn = k;
		nkn = n - k;
		blend = muk * munk;
		muk *= mu;
		munk /= (1-mu);

		while( nn >= 1 ) {

			blend *= (FLOAT)nn;
			nn--;

			if( kn > 1 ) {
				blend /= (FLOAT)kn;
				kn--;
			}

			if( nkn > 1 ) {
				blend /= (FLOAT)nkn;
				nkn--;
			}
		}

		(*b)[0] += (p[k][0] * blend);
		(*b)[1] += (p[k][1] * blend);
		(*b)[2] += (p[k][2] * blend);
	}

	return;
}


//   #define NSTEPS 100
//   int ncontrolpoints=0;
//   XYZ p,*controlpoints;

//   ... create/read the controlpoint array and data....
//
//   for (i=0;i%lt;NSTEPS;i++) {
//      p = Bezier(controlpoints,ncontrolpoints,i/(double)N));
//      .... do something with p, eg: plot it  ...
//   }




//
// A: 1,1 36,50 sima semmi
//
void test49( void ) {

	BOOL done = FALSE;
	int x,y,key,cnt=0,i,j,tic;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;
	light_t *sun;
	rgb_t color;


#define MAX_STEPS   25								// This is the amount of steps we want to draw the curve

	point3_t points[] = {
		{ 10, 150, 0},
		{100,  50, 0},
		{150, 150, 0},
		{200, 350, 0},
		{250, 300, 0},
		{300, 180, 0} };
	point3_t prev,p;

	x = winReadProfileInt( "width", 800 );
	y = winReadProfileInt( "height", 600 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);

	MAKEVECTOR( cam_view, 0.0, 1.0, 0.0 );
	MAKEVECTOR( cam_pos, 0.0, 0.0, 0.0 );

	CV_SetValue( "zclipfar", 150.0f );
	CV_SetValue( "zclipnear", 0.0f );

	sun = AddLight( LT_AMBIENT, 150,150,150,255, 100.0f, NULL, NULL );

	// TomFlag(NOFLAG);
	// SaveAsc( tunnel[0].obj_ptr );

	tic = GetTic();

	done = FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		// if( key == key_Escape ) done = TRUE;
		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		// render

		Clear( NOCOLOR ); // 0x00209f  NOCOLOR

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );
		SetupFog( zclipfar.value/2.0f, zclipfar.value, 0x000000 );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		SetFont(1);
		SetFontAlpha( FA_ALL, 255,255,255, 255 );

		if( mousebld ) {

			FLOAT min = 100.0f, d;
			point3_t m;
			int cur = -1;

			MAKEVECTOR( m, mousex, mousey, 0.0 );

			for( i=0; i<dimof(points); i++ )
				if( (d = DistanceVector( m, points[i] )) < min ) {
					min = d;
					cur = i;
				}

			if( cur != (-1) ) {
				MAKEVECTOR( points[cur], mousex, mousey, 0.0 );
				WriteString(10,10,"koord: %d, %d",mousex,mousey);
				color.r = 0;
				color.g = 250;
				color.b = 0;
				color.a = 255;
				for( j=0; j<2; j++ ) {
					Circle( mousex, mousey, 10-j, color );
				}
			}
		}

		// kontrol pontok

		color.r = 255;
		color.g = 255;
		color.b = 255;
		color.a = 255;

		for( i=0; i<dimof(points)-1; i++ )
			Line( points[i][0], points[i][1], points[i+1][0], points[i+1][1], color );

		CopyVector( prev, points[0] );

		for( i=0; i<=MAX_STEPS; i++ ) {

			MAKEVECTOR( p, 0,0,0 );

			// PointOnCurve( points[0], points[1], points[2], points[3], (FLOAT)i/(FLOAT)MAX_STEPS, &p );
			Bezier( points, dimof(points), (FLOAT)i/(FLOAT)MAX_STEPS, &p );

			if( i == MAX_STEPS )
				MAKEVECTOR( p, points[dimof(points)-1][0],points[dimof(points)-1][1],points[dimof(points)-1][2] );

			color.r = 50+((FLOAT)i*200.0)/(FLOAT)MAX_STEPS;
			color.b = 50+200-((FLOAT)i*200.0)/(FLOAT)MAX_STEPS;

			color.g = 50;
			color.a = 255;

			for( j=0; j<3; j++ )
				Line( prev[0], prev[1]+j, p[0], p[1]+j, color );

			CopyVector( prev, p );
		}

		FlushScene();

		consoleAnimate();

		DrawMouse();

		EndScene();

		FlipPage();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}



/***

extern int colorquant( UCHAR *red, UCHAR *green, UCHAR *blue,
						ULONG pixels,
						UCHAR *colormap[3],
						int colors,
						int bits,
						UCHAR *rgbmap,
						int flags,
						int accum_hist );


//
// Aureal3D
//
void test50( void ) {

	BOOL done = FALSE,stopped;
	int x,y,key,cnt=0,i,handle,pan;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;
	FLOAT f;
	char *txt;

	if( !winOpenFile( "Wave Files (*.wav)\0*.wav\0MP3 Files (*.mp3)\0*.mp3\0All Files (*.*)\0*.*\0") )
		Quit( NULL );

	if( (txt=winGetNextFile()) == NULL )
		return;

	x = winReadProfileInt( "width", 640 );
	y = winReadProfileInt( "height", 480 );

	SetX(x,y,16,SETX_NORMAL);

	if( AU_Init() == FALSE ) Quit("No A3D.");
	handle = AU_LoadWave( txt );

	InitKeyboard();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);
	// HideMouse();

	MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
	MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );

	CV_SetValue( "zclipfar", 250.0f );
	CV_SetValue( "zclipnear", 0.0f );

	done = FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		pan = mousex * MAXPAN / SCREENW;

		if( key == ' ' || mousebl ) {
		}

		if( key == key_Enter || mousebr ) {
		}

		point3_t v0 = {0,0,0};

		v0[0] = mousex * 10 / SCREENW - 5;
		v0[1] = mousey * 10 / SCREENH - 5;

		AU_SetParam( handle, v0, MAXVOL, 1 );

		Clear( NOCOLOR ); // 0x00209f );

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
						 CLIPMAXX - CLIPMINX,
						 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		FlushScene();

		consoleAnimate();

		DrawMouse();

		WriteString(10,10,"memory used: %d\nframe/sec: %d\nframe cnt: %d\nvideo: %dx%dx%d",(int)mem_used,(int)frame,cnt++,SCREENW,SCREENH,bpp);

		EndScene();

		FlipPage();

		AU_Update();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	AU_Deinit();

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}

*/

#define MODTEST

//
// Fmod, Bass, Seal, Mikmod, Audiere
//
void test51( void ) {

	BOOL done = FALSE;
	int x,y,key,cnt=0,handle=-1,pan;
	ULONG frame=0L,framecnt=0,tics=0L;
	point3_t cam_pos,cam_view;
	char txt[MAX_PATH],*mod;

	// Message( winTemp() );
	xprintf("%s\n", winTemp() );

	cvar_t sound_volume = {"sound_volume","1",TRUE};
	cvar_t music_volume = {"music_volume","1",TRUE};

	CV_RegisterVariable( &sound_volume );
	CV_RegisterVariable( &music_volume );

	CV_SetValue( "sound_volume", (FLOAT)winReadProfileInt( "sound_volume", MAXVOL/2 ) );
	CLAMPMINMAX( sound_volume.value, 0.0f, MAXVOL );

	CV_SetValue( "music_volume", (FLOAT)winReadProfileInt( "music_volume", MAXVOL/2 ) );
	CLAMPMINMAX( music_volume.value, 0.0f, MAXVOL );

#ifdef MODTEST
	if( !winOpenFile( "All Module Files\0*.mod;*.s3m;*.xm;*.it;*.mtm\0Protracker (*.mod)\0*.mod\0Scream Tracker 3 (*.s3m)\0*.s3m\0MTM Files (*.mtm)\0*.mtm\0FastTracker 2 (*.xm)\0*.xm\0Impulse Tracker (*.it)\0*.it\0All Files (*.*)\0*.*\0") )
		Quit( NULL );
#else
	if( !winOpenFile( "All Wave Files\0*.wav;*.ogg;*.mp3;\0All Files (*.*)\0*.*\0") )
		Quit( NULL );
#endif

 	if( (mod=winGetNextFile()) == NULL )
		return;

	strcpy( txt, mod );

	x = winReadProfileInt( "width", 800 );
	y = winReadProfileInt( "height", 600 );

	SetX(x,y,16,SETX_NORMAL);

	InitKeyboard();

	SL_Init();

	x = 0;
	SETFLAG( x, MF_3DCURSOR );
	InitMouse(x);
	// HideMouse();

	MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
	MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );

	char *wave_name = "proba.wav";

	if( (handle = SL_LoadWave(wave_name, 1)) == -1 )
		Quit("no %s wave.",wave_name);

	// E:\\XenoHammerGL\\Level2.ogg
	// if( (handle = SL_LoadWave("proba.wav", 1)) == -1 )
	//	xprintf("no wave.");

	SL_PlayMod(txt);
	SL_SetModuleParam( music_volume.value, -1, -1 );

	CV_SetValue( "zclipfar", 250.0f );
	CV_SetValue( "zclipnear", 0.0f );

	done = FALSE;
	while( !done ) {

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		if( key == EXT(sc_F12) )
			display_zprintf ^= 1;

		pan = mousex * MAXPAN / SCREENW;

		if( key == ' ' || mousebld ) {
			// if( !SL_IsPlaying( handle ) )
			static int tic = 0;

			if( (GetTic() - tic) > (TICKBASE/10) ) {
				tic = GetTic();
				// SL_PLAYWAVE( handle, freq, vol, pan, loop )
				SL_PLAYWAVE( handle, -1, sound_volume.value, pan, 0 );
			}
		}

		if( key == key_Enter || mousebr ) {
			SL_StopMod();
			SL_PlayMod(txt);
			SL_SetModuleParam( music_volume.value, -1, -1 );
		}

		const int delta = 5;

		if( (key == '+') || (mousedz>0) ) {
			music_volume.value += delta;
			CLAMPMINMAX( music_volume.value, 0, MAXVOL );
			SL_SetModuleParam( music_volume.value, -1, -1 );
			sound_volume.value = music_volume.value;
		}

		if( (key == '-') || (mousedz<0) ) {
			music_volume.value -= delta;
			CLAMPMINMAX( music_volume.value, 0, MAXVOL );
			SL_SetModuleParam( music_volume.value, -1, -1 );
			sound_volume.value = music_volume.value;
		}

		// F1 - F8

		switch( key ) {
			case EXT(sc_F1):
				music_volume.value = MAXVOL*0/8; CLAMPMINMAX( music_volume.value, 0, MAXVOL ); SL_SetModuleParam( music_volume.value, -1, -1 );
				sound_volume.value = music_volume.value;
				break;
			case EXT(sc_F2):
				music_volume.value = MAXVOL*2/8; CLAMPMINMAX( music_volume.value, 0, MAXVOL ); SL_SetModuleParam( music_volume.value, -1, -1 );
				sound_volume.value = music_volume.value;
				break;
			case EXT(sc_F3):
				music_volume.value = MAXVOL*3/8; CLAMPMINMAX( music_volume.value, 0, MAXVOL ); SL_SetModuleParam( music_volume.value, -1, -1 );
				sound_volume.value = music_volume.value;
				break;
			case EXT(sc_F4):
				music_volume.value = MAXVOL*4/8; CLAMPMINMAX( music_volume.value, 0, MAXVOL ); SL_SetModuleParam( music_volume.value, -1, -1 );
				sound_volume.value = music_volume.value;
				break;
			case EXT(sc_F5):
				music_volume.value = MAXVOL*5/8; CLAMPMINMAX( music_volume.value, 0, MAXVOL ); SL_SetModuleParam( music_volume.value, -1, -1 );
				sound_volume.value = music_volume.value;
				break;
			case EXT(sc_F6):
				music_volume.value = MAXVOL*6/8; CLAMPMINMAX( music_volume.value, 0, MAXVOL ); SL_SetModuleParam( music_volume.value, -1, -1 );
				sound_volume.value = music_volume.value;
				break;
			case EXT(sc_F7):
				music_volume.value = MAXVOL*7/8; CLAMPMINMAX( music_volume.value, 0, MAXVOL ); SL_SetModuleParam( music_volume.value, -1, -1 );
				sound_volume.value = music_volume.value;
				break;
			case EXT(sc_F8):
				music_volume.value = MAXVOL*8/8; CLAMPMINMAX( music_volume.value, 0, MAXVOL ); SL_SetModuleParam( music_volume.value, -1, -1 );
				sound_volume.value = music_volume.value;
				break;
		}

		Clear( NOCOLOR ); // 0x00209f );

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
						 CLIPMAXX - CLIPMINX,
						 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );

		GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f);

		FlushScene();

		consoleAnimate();

		DrawMouse();

		int vol,ord,row;
		SL_GetModuleParam(&vol,&ord,&row);

		SetFont(1);
		WriteString(10,SCREENH/2,"memory used: %d\nframe/sec: %d\nframe cnt: %d\nvideo: %dx%dx%d\nmod: %d vol, %d ord, %d row\nlib: %s",(int)mem_used,(int)frame,cnt++,SCREENW,SCREENH,bpp,vol,ord,row,SL_NAME);

		// xprintf("\rvalaki: %d", framecnt );

		EndScene();

		FlipPage();

		if(tics+TICKBASE<GetTic()) { tics = GetTic(); frame = framecnt; framecnt=0; }
		else { ++framecnt; }
	}

	SL_StopMod();

	SL_FreeAllWave();

	SL_Deinit();

	DeinitMouse();
	DeinitKeyboard();

	ResetX();

	return;
}

/*

//
//
//
void test52( void ) {

	char path[MAX_PATH];
	int tilos[][2] = {
		{ 'á', 'a' },
		{ 'é', 'e' },
		{ 'í', 'i' },
		{ 'ó', 'o' },
		{ 'ö', 'o' },
		{ 'õ', 'o' },
		{ 'ú', 'u' },
		{ 'ü', 'u' },
		{ 'û', 'u' },
		{ '\'', ' ' },	// '
		{ '!', ' ' },
		{ '%', ' ' },
	};

	// winHideBackWindow();
	// winHideLog();

	if( myargc > 1 )
		strcpy( path, myargv[1] );
	else
	if( winBrowseForFolder( path ) == FALSE )
		return;

	// Message( path );

	findfile_t find;
	char str[1024];
	int i,j,k;

	typedef char string2_t[256];

	string2_t *filez = NULL;
	int nfilez = 0;

	_chdir( path );

	if( !FindFirst( "*.*", &find ) )
		;
	else
	do {

		if( !stricmp( find.name, "." ) ||
			!stricmp( find.name, ".." ) ||
			ISFLAG( find.attrib, FIND_SUBDIR ) )
			continue;

		if( (filez = (string2_t *)realloc( filez, (nfilez+1)*sizeof(string2_t))) == NULL ) {
			Message("Not enough memory.\n");
			break;
		}

		strcpy( filez[nfilez], find.name );
		++nfilez;

	} while( FindNext( &find ) );

	MyFindClose();

	if( nfilez < 1 )
		xprintf( "None found.\n");

	for( i=0; i<nfilez; i++ ) {

		BOOL flag = FALSE;

		strcpy( str, filez[i] );

		for( k=0; k<dimof(tilos); k++ )
			for( j=0; j<strlen(str); j++ )
				if( str[j] == tilos[k][0] ) {
					flag = TRUE;
					str[j] = tilos[k][1];
				}

		if( flag == TRUE ) {

			if( rename( filez[i], str ) != 0 )
				xprintf( "Could not rename '%s'\n", filez[i] );
			else
				xprintf( "File '%s' renamed to '%s'\n", filez[i], str );
		}
		else
			xprintf( "kewl: %s\n", filez[i] );
	}

	if( filez ) free( filez );

	Message( "beendet." );

	return;
}

*/



#ifndef NDEBUG

#define REQUIRE( expr )    {if(!(expr))throw std::runtime_error("precondition failed: "  #expr " in " __FILE__);}
#define ENSURE( expr )     {if(!(expr))throw std::runtime_error("postcondition failed: " #expr " in " __FILE__);}
#define INVARIANT( expr )  {if(!(expr))throw std::runtime_error("invariant failed: "     #expr " in " __FILE__);}
#define CHECK( expr )      {if(!(expr))throw std::runtime_error("assertion failed: "     #expr " in " __FILE__);}

#else

#define REQUIRE( expr )    assert(expr)
#define ENSURE( expr )     assert(expr)
#define INVARIANT( expr )  assert(expr)
#define CHECK( expr )      assert(expr)

#endif

#define rangedrand(N) ((int) (((double) N)*rand()/(RAND_MAX+1.0)))



// #include "name.h"



//
//
//
void showNuke( int x, int y ) {

	polygon_t poly;
	point_t point[4];
	rgb_t rgb[4] = { {255,255,255,255}, {255,255,255,255}, {255,255,255,255}, {255,255,255,255} };
	point3_t st[4] = { {0.0f,0.0f,0.0f}, {0.0f,63.0f,0.0f}, {224.0f,63.0f,0.0f}, {224.0f,0.0f,0.0f} };
	int alpha;
	int row,col;
	static int texmapid = (-1);
	static int tick = 0,cnt=0;

#define NUKEMAP "dx.bmp"

	if( (texmapid = TexNumForName( NUKEMAP )) == (-1) ) {

		int flag,color;
		UCHAR pal[768],*spr;

		if( FileExist(NUKEMAP) ) {
			LoadPicture( NUKEMAP, PTR(spr) );
			GetRgbPicture(pal);
		}
		else
			CheckerTexture( &spr, 256,256 );

		flag = 0L;
		color = 0L;

		SETFLAG( flag, TF_CHROMA );
		// SETFLAG( flag, TF_ALPHA );

		AddTexMapHigh( spr, pal, NUKEMAP, NUKEMAP, flag, color );

		if( (texmapid = TexNumForName( NUKEMAP )) == (-1) ) {
				xprintf("showSplash: no \"%s\" textures.", NUKEMAP );
				return;
		}

		FREEMEM( spr );
	}

	poly.npoints = 4;
	poly.point = point;

	alpha = 255;

	rgb[0].a = alpha;
	rgb[1].a = alpha;
	rgb[2].a = alpha;
	rgb[3].a = alpha;

	point[0][0] = x;	point[0][1] = y;
	point[1][0] = x;	point[1][1] = y+30;
	point[2][0] = x+40;	point[2][1] = y+30;
	point[3][0] = x+40;	point[3][1] = y;

	if( (tick + (TICKBASE*0.04)) < GetTic() ) {
		++cnt;
		tick = GetTic();
	}

	cnt %= 30;

	row = ftoi(cnt / 6);
	col = cnt % 6;

	st[0][0] = col*40;		st[0][1] = row*30;
	st[1][0] = col*40;		st[1][1] = (row+1)*30;
	st[2][0] = (col+1)*40;	st[2][1] = (row+1)*30;
	st[3][0] = (col+1)*40;	st[3][1] = row*30;

	PutSpritePoly( poly, st, texmapid, rgb );

	return;
}


/*

//
//
//
void selfModify( void ) {

	DWORD oldprotect;
	int val1,val2;

	typedef LONG (*FunctionType)( LONG, LONG );

	FunctionType ComputeSum;

	ComputeSum = (FunctionType)( new BYTE[11] );

	((LPBYTE)ComputeSum)[0] = 0x55; // push ebp
	((LPBYTE)ComputeSum)[1] = 0x8B; // mov ebp, esp
	((LPBYTE)ComputeSum)[2] = 0xEC;
	((LPBYTE)ComputeSum)[3] = 0x8B; // mov eax,[bp+8]
	((LPBYTE)ComputeSum)[4] = 0x45;
	((LPBYTE)ComputeSum)[5] = 0x08;
	((LPBYTE)ComputeSum)[6] = 0x03; // add eax,[bp+12]
	((LPBYTE)ComputeSum)[7] = 0x45;
	((LPBYTE)ComputeSum)[8] = 0x0C;
	((LPBYTE)ComputeSum)[9] = 0x5D; // pop ebp
	((LPBYTE)ComputeSum)[10] = 0xC3; // ret eax

	VirtualProtect( ComputeSum, 11, PAGE_EXECUTE, &oldprotect );

	FlushInstructionCache( NULL, NULL, NULL );

	xprintf( "sum = %d\n", ComputeSum(1, 2) );

	val1 = 12;
	val2 = 34;

	xprintf( "sum = %d\n", ComputeSum(val1, val2) );

	VirtualProtect( ComputeSum, 11, oldprotect, &oldprotect );

	delete (LPBYTE)ComputeSum;


	return;
}

*/


//
//
//
void spinFlip( void ) {

	static int	trail = 8;
	static int	items = 60;
	static int	ampl = 60;
	static int	aa, bb;
	static int	frame = 0, old_frame = 0;
	static double	z, a, csapb, snapb, cssab, sa, ca, b, csab, snab, s5, x, y;
	static double	tt;
	static int spinx = SCREENW/2;
	static int spiny = SCREENH/2;
	static int tic = GetTic();
	int i;

	polygon_t poly;
	rgb_t rgb[31],tri_rgb[3];
	point_t point[31],tri[3];

	poly.npoints = 3; // trail*2;
	poly.point = tri;

#define FRAMERATE (50.0f)

	// draw the spinner
	frame = ftoi( ( (FLOAT)(GetTic()-tic)/ ( (FLOAT)TICKBASE / FRAMERATE ) ) );

	if( frame > (old_frame + 0) ) {
		spinx += ((mousex - spinx) / 10);
		spiny += ((mousey - spiny) / 10);
		old_frame = frame;
	}

	for( bb=0; bb<items; bb+=2 ) {

		b = 1.5 + bb / 33.3333333;
		for( aa=0; aa<trail; aa++ ) {

			tt = tt + .0005;
			a = (aa + frame) / 33.33333333;
    		csab = cos(a * b);
			csapb = cos(a + b);
			snab = sin(a * b);
			snapb = sin(a + b);
			sa = sin(a * .1);
			ca = cos(a * .1);
			s5 = sin(a * .05);

			point[aa*2][0] = sa * snapb * ampl * csab + ca * (csapb * ampl * b * csab + s5 * (ampl * (csab + 2 * snapb))) + spinx;
			point[aa*2][1] = sa * csapb * ampl * csab + ca * (csapb * ampl * b * snab + s5 * (ampl * (cssab + 2 * csapb))) + spiny;

			rgb[aa*2].r = (long)(((bb<<6)/items)*(sin(tt)+2));
			rgb[aa*2].g = (long)(((bb<<6)/items)*(cos(tt)+2)),
			rgb[aa*2].b = (long)(((bb<<6)/items)*(sin(tt*1.3)+2));
			rgb[aa*2].a = 255;

			// xprintf("p: %d, %d\n", point[aa*2][0], point[aa*2][1] );
		}

		b = 1.5 + (bb+1) / 33.3333333;
		for( aa=0; aa<trail; aa++ ) {

			tt = tt + .0001;
			a = (aa + frame) / 33.33333333;
    		csab = cos(a * b);
			csapb = cos(a + b);
			snab = sin(a * b);
			snapb = sin(a + b);
			sa = sin(a * .1);
			ca = cos(a * .1);
			s5 = sin(a * .05);

			point[aa*2+1][0] = sa * snapb * ampl * csab + ca * (csapb * ampl * b * csab + s5 * (ampl * (csab + 2 * snapb))) + spinx;
			point[aa*2+1][1] = sa * csapb * ampl * csab + ca * (csapb * ampl * b * snab + s5 * (ampl * (cssab + 2 * csapb))) + spiny;

			rgb[aa*2+1].r = (long)(((bb<<6)/items)*(sin(tt)+2));
			rgb[aa*2+1].g = (long)(((bb<<6)/items)*(cos(tt)+2)),
			rgb[aa*2+1].b = (long)(((bb<<6)/items)*(sin(tt*1.1)+2));
			rgb[aa*2+1].a = 255;

		}

		// strip to tris
		for( i=0; i<(trail*2-2)/2; i++ ) {

			tri[0][0] = point[(i*2)+0][0];
			tri[0][1] = point[(i*2)+0][1];

			tri[1][0] = point[(i*2)+1][0];
			tri[1][1] = point[(i*2)+1][1];

			tri[2][0] = point[(i*2)+2][0];
			tri[2][1] = point[(i*2)+2][1];

			tri_rgb[0].r = rgb[(i*2)+0].r;	tri_rgb[0].g = rgb[(i*2)+0].g;	tri_rgb[0].b = rgb[(i*2)+0].b;	tri_rgb[0].a = rgb[(i*2)+0].a;
			tri_rgb[1].r = rgb[(i*2)+1].r;	tri_rgb[1].g = rgb[(i*2)+1].g;	tri_rgb[1].b = rgb[(i*2)+1].b;	tri_rgb[1].a = rgb[(i*2)+1].a;
			tri_rgb[2].r = rgb[(i*2)+2].r;	tri_rgb[2].g = rgb[(i*2)+2].g;	tri_rgb[2].b = rgb[(i*2)+2].b;	tri_rgb[2].a = rgb[(i*2)+2].a;

			PutPoly( poly, tri_rgb );

			tri[0][0] = point[(i*2)+2][0];
			tri[0][1] = point[(i*2)+2][1];

			tri[1][0] = point[(i*2)+1][0];
			tri[1][1] = point[(i*2)+1][1];

			tri[2][0] = point[(i*2)+3][0];
			tri[2][1] = point[(i*2)+3][1];

			tri_rgb[0].r = rgb[(i*2)+2].r;	tri_rgb[0].g = rgb[(i*2)+2].g;	tri_rgb[0].b = rgb[(i*2)+2].b;	tri_rgb[0].a = rgb[(i*2)+2].a;
			tri_rgb[1].r = rgb[(i*2)+1].r;	tri_rgb[1].g = rgb[(i*2)+1].g;	tri_rgb[1].b = rgb[(i*2)+1].b;	tri_rgb[1].a = rgb[(i*2)+1].a;
			tri_rgb[2].r = rgb[(i*2)+3].r;	tri_rgb[2].g = rgb[(i*2)+3].g;	tri_rgb[2].b = rgb[(i*2)+3].b;	tri_rgb[2].a = rgb[(i*2)+3].a;

			PutPoly( poly, tri_rgb );
		}

		// err = dev->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DVT_TLVERTEX, TLVertices, trail * 2, D3DDP_WAIT);
		// if you prefer lines
		// err = dev->DrawPrimitive(D3DPT_LINESTRIP, D3DVT_TLVERTEX, TLVertices, trail, D3DDP_WAIT);
	}

	return;
}


/*
//
// Define our own reporting function.
// We'll hook it into the debug reporting
// process later using _CrtSetReportHook.
// Define a global int to keep track of
// how many assertion failures occur.
//
static int gl_num_asserts=0;
int OurReportingFunction( int reportType, char *userMessage, int *retVal ) {

	// Tell the user our reporting function is being called.
    // In other words - verify that the hook routine worked.

	printf("Inside the client-defined reporting function.\n");

    // When the report type is for an ASSERT,
    // we'll report some information, but we also
    // want _CrtDbgReport to get called -
    // so we'll return TRUE.
    //
    // When the report type is a WARNing or ERROR,
    // we'll take care of all of the reporting. We don't
    // want _CrtDbgReport to get called -
    // so we'll return FALSE.

 	if( reportType == _CRT_ASSERT ) {
      	gl_num_asserts++;
      	printf("This is the number of Assertion failures that have occurred: %d \n", gl_num_asserts );
      	printf("Returning TRUE from the client-defined reporting function.\n" );
      	return TRUE;
	}
	else {
      	printf("This is the debug user message: %s \n", userMessage );
      	printf("Returning FALSE from the client-defined reporting function.\n" );
      	return FALSE;
   	}

	// By setting retVal to zero, we are instructing _CrtDbgReport
    // to continue with normal execution after generating the report.
    // If we wanted _CrtDbgReport to start the debugger, we would set
    // retVal to one.

	retVal = 0;

	return TRUE;
}
*/




////// Bsp /////////////


// This is the number that is associated with a face that is of type "polygon"
#define FACE_POLYGON	1
#define MAX_TEXTURES 1000				// The maximum amount of textures to load



//
// This is our BSP header structure
//
typedef struct BSPHeader_s {

	char strID[4];				// This should always be 'IBSP'
	int version;				// This should be 0x2e for Quake 3 files

} BSPHeader_t;


//
// This is our BSP lump structure
//
typedef struct BSPLump_s {

	int offset;					// The offset into the file for the start of this lump
	int length;					// The length in bytes for this lump

} BSPLump_t;


//
// This is our BSP vertex structure
//
typedef struct BSPVertex_s {

	point3_t vPosition;			// (x, y, z) position.
    point3_t vTextureCoord;		// (u, v) texture coordinate
    point3_t vLightmapCoord;	// (u, v) lightmap coordinate
    point3_t vNormal;			// (x, y, z) normal vector
    UCHAR color[4];				// RGBA color for the vertex

} BSPVertex_t;


//
// This is our BSP face structure
//
typedef struct BSPFace_s {

    int textureID;				// The index into the texture array
    int effect;					// The index for the effects (or -1 = n/a)
    int type;					// 1=polygon, 2=patch, 3=mesh, 4=billboard
    int startVertIndex;			// The starting index into this face's first vertex
    int numOfVerts;				// The number of vertices for this face
    int meshVertIndex;			// The index into the first meshvertex
    int numMeshVerts;			// The number of mesh vertices
    int lightmapID;				// The texture index for the lightmap
    int lMapCorner[2];			// The face's lightmap corner in the image
    int lMapSize[2];			// The size of the lightmap section
    point3_t lMapPos;			// The 3D origin of lightmap.
    point3_t lMapVecs[2];		// The 3D space for s and t unit vectors.
    point3_t vNormal;			// The face normal.
    int size[2];				// The bezier patch dimensions.

} BSPFace_t;


//
// This is our BSP texture structure
//
typedef struct BSPTexture_s {

	char strName[64];			// The name of the texture w/o the extension
	int flags;				// The surface flags (unknown)
	int contents;				// The content flags (unknown)

} BSPTexture_t;



//
// This is our lumps enumeration
//
enum eLumps {

	kEntities = 0,				// Stores player/object positions, etc...
    kTextures,					// Stores texture information
    kPlanes,				    // Stores the splitting planes
    kNodes,						// Stores the BSP nodes
    kLeafs,						// Stores the leafs of the nodes
    kLeafFaces,					// Stores the leaf's indices into the faces
    kLeafBrushes,				// Stores the leaf's indices into the brushes
    kModels,					// Stores the info of world models
    kBrushes,					// Stores the brushes info (for collision)
    kBrushSides,				// Stores the brush surfaces info
    kVertices,					// Stores the level vertices
    kMeshVerts,					// Stores the model vertices offsets
    kShaders,					// Stores the shader files (blending, anims..)
    kFaces,						// Stores the faces for the level
    kLightmaps,					// Stores the lightmaps for the level
    kLightVolumes,				// Stores extra world lighting information
    kVisData,					// Stores PVS and cluster info (visibility)
    kMaxLumps					// A constant to store the number of lumps
};


//
// This is our bitset class for storing which face has already been drawn.
// The bitset functionality isn't really taken advantage of in this version
// since we aren't rendering by leafs and nodes.
//
class bitset_c {

public:

	// Initialize all the data members
    bitset_c() : m_bits(0), m_size(0) {}

	// This is our deconstructor
	~bitset_c() {

		// If we have valid memory, get rid of it
		if( m_bits ) {
			delete m_bits;
			m_bits = NULL;
		}
	}

	// This resizes our bitset to a size so each face has a bit associated with it
	void Resize( int count ) {

		// Get the size of integers we need
		m_size = count/32 + 1;

		// Make sure we haven't already allocated memory for the bits
        if(m_bits)
		{
			delete m_bits;
			m_bits = 0;
		}

		// Allocate the bits and initialize them
		m_bits = new unsigned int[m_size];
		ClearAll();
	}

	// This does the binary math to set the desired bit
	void Set( int i ) {

		m_bits[i >> 5] |= (1 << (i & 31));
	}

	// This returns if the desired bit slot is a 1 or a 0
	int On( int i ) {

		return m_bits[i >> 5] & (1 << (i & 31 ));
	}

	// This clears a bit to 0
	void Clear( int i ) {

		m_bits[i >> 5] &= ~(1 << (i & 31));
	}

	// This initializes the bits to 0
	void ClearAll() {

		memset(m_bits, 0, sizeof(unsigned int) * m_size);
	}

private:

	// Our private bit data that holds the bits and size
	unsigned int *m_bits;
	int m_size;
};



//
// bsp tree structure
//
typedef struct bsptree_s {

	int  numverts;					// The number of verts in the model
	int  numfaces;					// The number of faces in the model
	int  numtextures;				// The number of texture maps

	BSPVertex_t	*pVerts;			// The object's vertices
	BSPFace_t	*pFaces;			// The faces information of the object
									// The texture and lightmap array for the level
	UINT m_textures[MAX_TEXTURES];

	bitset_c m_FacesDrawn;			// The bitset for the faces that have/haven't been drawn

} bsptree_t, *bsptree_ptr;


/***
// Our constructor
CQuake3BSP();

// Our deconstructor
~CQuake3BSP();

// This loads a .bsp file by it's file name (Returns true if successful)
bool LoadBSP(const char *strFileName);

// This renders the level to the screen, currently the camera pos isn't being used
void RenderLevel(const CVector3 &vPos);

// This destroys the level data
void Destroy();

// This attaches the correct extension to the file name, if found
void FindTextureExtension(char *strFileName);

// This renders a single face to the screen
void RenderFace(int faceIndex);

int  m_numOfVerts;			// The number of verts in the model
int  m_numOfFaces;			// The number of faces in the model
int  m_numOfTextures;		// The number of texture maps

tBSPVertex  *m_pVerts;		// The object's vertices
tBSPFace *m_pFaces;			// The faces information of the object
							// The texture and lightmap array for the level
UINT m_textures[MAX_TEXTURES];

CBitset m_FacesDrawn;		// The bitset for the faces that have/haven't been drawn
***/



////////// whirling effect /////////


static int effect_texmapid = (-1);
static texmap_t *effect_texmap = NULL;

//
//
//
void InitWhirl( void ) {

	char *FXNAME = "WHIRL_TEXTURE1";

	// background
	if( (effect_texmapid = TexNumForName( FXNAME )) == (-1) ) {

		int flag,color;
		UCHAR pal[768],*spr;
		int width,height,c,x,y,i;

		width = 256;
		height = 256;

		ALLOCMEM( spr, SPRITEHEADER + (width*height) );
		memset( spr, 0L, SPRITEHEADER + (width*height) );
		MKSPRW( spr, width );
		MKSPRH( spr, height );
		MKSPR8( spr );

		memset( pal, 0L, 768 );

/*
        c = rgb (rand(32,128), rand(32,64), rand(32,64)) ;
        x = timer/5%100 ;
        IF (x > 50) x = 100-x; END
        xput (0, map, 160, 100, timer*-50, 200+x, 4, 0) ;
        FROM i = 0 TO 100:
                map_put_pixel (0, map, rand(0,320), rand(0, 200), c) ;
                map_put_pixel (0, map, rand(0,320), rand(0, 200), 1) ;
        END
        FRAME ;

		gr_rotated_blit (background, &regions[r], params[2], params[3],
			 params[6], params[4], params[5], params[5], map) ;

*/
		if( 1 ) {

			for( i=0; i<5000; i++ ) {

				c = 1 + i%250;

				pal[c*3+0] = 32 + rand()%10;
				pal[c*3+1] = 32 + rand()%50;
				pal[c*3+2] = 32 + rand()%90;

				pal[c*3+0] >>= 1;
				pal[c*3+1] >>= 1;
				pal[c*3+2] >>= 1;

				x = 1 + rand()%(width-2);
				y = 1 + rand()%(height-2);

				// SPR(spr,x,y-1) = c;
				SPR(spr,x,y) = c;
				SPR(spr,x,y+1) = c;
				//SPR(spr,x-1,y-1) = c;
				//SPR(spr,x-1,y) = c;
				//SPR(spr,x-1,y+1) = c;
				//SPR(spr,x+1,y-1) = c;
				SPR(spr,x+1,y) = c;
				SPR(spr,x+1,y+1) = c;
			}

			HighSprite( PTR(spr), pal );

			flag = 0L;
			color = 0L;

			SETFLAG( flag, TF_CHROMA );
			// SETFLAG( flag, TF_ALPHA );

			AddTexMapHigh( spr, pal, FXNAME, FXNAME, flag, color );

			if( (effect_texmapid = TexNumForName( FXNAME )) == (-1) ) {
				Quit("InitWhirl: no \"%s\" texture.", FXNAME );
				return;
			}
		}
		else {

			char *filename = "water1.jpg";

			flag = 0L;
			color = 0L;

			SETFLAG( flag, TF_CHROMA );
			// SETFLAG( flag, TF_ALPHA );

			if( (effect_texmapid = LoadTexmap( filename, flag, color )) == (-1) ) {
				Quit("InitWhirl: no %s named texture for object.", filename);
				return;
			}
		}


		effect_texmap = TexForNum( effect_texmapid );

		FREEMEM( spr );
	}
	else
		effect_texmap = TexForNum( effect_texmapid );

	return;
}




//
// x1 = x*cos(a) - y*sin(a)
// y1 = x*sin(a) + y*cos(a)
//
static void rotatePoint( FLOAT ang, point_t point[4], FLOAT centerx, FLOAT centery ) {

	int cx,cy,i;
	FLOAT dx,dy,sina,cosa;

	if( (ang == 0.0f) || (ang == 360.0f) )
		return;

	// cx = (int)((FLOAT)(point[0][0] + point[1][0] + point[2][0] + point[3][0]) / 4.0f);
	// cy = (int)((FLOAT)(point[0][1] + point[1][1] + point[2][1] + point[3][1]) / 4.0f);

	cx = centerx;
	cy = centery;

	sina = GetSin( deg2rad(ang) );
	cosa = GetCos( deg2rad(ang) );

	for( i=0; i<4; i++ ) {

		dx = point[i][0] - cx;
		dy = point[i][1] - cy;

		point[i][0] = cx + (dx*cosa - dy*sina);
		point[i][1] = cy + (dx*sina + dy*cosa);
	}

	return;
}





//
//
//
void DrawWhirl( void ) {

	polygon_t poly;
	point_t point[4];	// a négyzet mind a 4 csucsának külön
	rgb_t rgb[4] = { {255,255,255,255}, {255,255,255,255}, {255,255,255,255}, {255,255,255,255} };
	point3_t st[4] = {
		{0.0f,0.0f,0.0f},
		{0.0f,255.0f,0.0f},
		{255.0f,255.0f,0.0f},
		{255.0f,0.0f,0.0f},
	};
	FLOAT tic,tic1,ang,sina,cosa;
	int x,y,w,h,alpha,dx,dy,x0,y0,texw,texh;
	int centerx,centery;

	if( effect_texmapid == (-1) || (effect_texmap == NULL) )
		return;

#define FX_FREQ		(TICKBASE*60)	// forgás frekvencia
#define FX_FREQ1	(TICKBASE*10)	// lüktetés üteme
#define FX_OFFS		160				// a lüktetés
#define FX_OFFS1	50				// a kör sugara

	tic = (FLOAT)(GetTic() % FX_FREQ) / (FLOAT)FX_FREQ;
	tic1 = (FLOAT)(GetTic() % FX_FREQ1) / (FLOAT)FX_FREQ1;

	poly.npoints = 4;
	poly.point = point;

	if( tic1 <= 0.5f ) {
		// novekszik
		x = FX_OFFS * tic1;
		y = FX_OFFS * tic1;

		alpha = 255.0f*tic1;
	}
	else {
		// csökken
		x = FX_OFFS * (1.0f-tic1);
		y = FX_OFFS * (1.0f-tic1);

		alpha = 255.0f * (1.0f-tic1);
	}

	alpha = 255;

	sina = sqrt((float)(SCREENW*SCREENW+SCREENH*SCREENH));
	w = sina + (-x)*2;
	h = sina + (-y)*2;

	ang = 360.0f * tic;

	// xprintf("ang: %.2f\n", ang );

	sina = GetSin( deg2rad(-360.0f * tic1) );
	cosa = GetCos( deg2rad(-360.0f * tic1) );

	dx = FX_OFFS1;
	dy = FX_OFFS1;

	centerx = SCREENW/2 + (dx*cosa - dy*sina);
	centery = SCREENH/2 + (dx*sina + dy*cosa);

	rgb[0].a = alpha;
	rgb[1].a = alpha;
	rgb[2].a = alpha;
	rgb[3].a = alpha;

	texw = effect_texmap->width1 + dx*sina;
	texh = effect_texmap->height1 + dy*sina;

	for( y0 = centery - h; y0<SCREENW+h; y0 += texh )
	for( x0 = centerx - w; x0<SCREENW+w; x0 += texw ) {

		point[0][0] = x0;	point[0][1] = y0;
		point[1][0] = x0;	point[1][1] = y0 + texh;
		point[2][0] = x0+texw;	point[2][1] = y0 + texh;
		point[3][0] = x0+texw;	point[3][1] = y0;

		rotatePoint( ang, point, centerx, centery );

		PutSpritePoly( poly, st, effect_texmapid, rgb );
	}

#undef FX_FREQ
#undef FX_OFFS
#undef FX_FREQ1
#undef FX_OFFS1

	return;
}

// #define DRAWWHIRL

/*

Public Sub ResetIt(X As Single, Y As Single, XSpeed As Single, YSpeed As Single, XAcc As Single, YAcc As Single, sngResetSize As Single)
    sngX = X
    sngY = Y
    sngXSpeed = XSpeed
    sngYSpeed = YSpeed
    sngXAccel = XAcc
    sngYAccel = YAcc
    sngSize = sngResetSize
End Sub

Public Sub Reset(I As Long)
    Dim X As Single, Y As Single

    X = sngX
    Y = sngY

    '//This is were we will reset individual particles.
    With Particles(I)
        Call .ResetIt(X, Y, -10, -1 * Rnd, 0, Rnd, 2)
        Call .ResetColor(0.25, 0.25, 1, 1, 0.1 + (0.1 * Rnd)) '+ Rnd * 0.3)
    End With
End Sub


void UpdateParticle( sngTime As Single ) {

	sngX = sngX + sngXSpeed * sngTime
    sngY = sngY + sngYSpeed * sngTime

    sngXSpeed = sngXSpeed + sngXAccel * sngTime
    sngYSpeed = sngYSpeed + sngYAccel * sngTime

    sngA = sngA - sngAlphaDecay * sngTime

	return;
}
*/

//
//
//
void test22(void) {

#define MAXOBJ 500

	int i,j,k,r,g,b,nobjs,curr_obj=0,anim_wait;
	UCHAR *spr=NULL,*back=NULL,pal[768],key;
	object_ptr obj[MAXOBJ];
	object_t *inter = NULL;
	BOOL done = FALSE,back_on=FALSE,anim_flag=FALSE, collide=FALSE;
	point3_t cam_pos,cam_view,cam_up,cam_right,temp_cam_pos,temp_cam_view;
	FLOAT zoom=1.5f,anim_per_sec;
	FLOAT fi,theta,ro,roll;
	ULONG frame=0L,framecnt=0,tics=0L,clear_color,anim_tics;
	char *txt;
	FLOAT ROTVAL,MOVEVAL,CAM_ROTVAL,CAM_MOVEVAL,BODY_HEIGHT;
	int width,height,alpha = 255,depth;
	int texmapid = -1;
	light_t *sun;
	int flipy = 1;
	int direction = 0;

	// Quit("próba szöveg. %d",  binary<1001>::value );

/*
	int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );	// Get current flag

	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;						// Turn on leak-checking bit
    tmpFlag |= _CRTDBG_DELAY_FREE_MEM_DF;
	tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;						// Turn off CRT block checking bit

	_CrtSetDbgFlag( tmpFlag );								// Set flag to the new value

	_CrtSetReportHook( OurReportingFunction );

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
*/

	/*
	for( i=0;i<100; i++ ) {
		char *ptr = NULL;
		if( (ptr = (char *)malloc(1024*8)) != NULL )
			free( ptr );
	}
	*/

	/***
	HINSTANCE DIHinst = NULL;
	// __cdecl  __stdcall
	void (__stdcall *pShowStartupForm)( void ) = NULL;

	if( (DIHinst = LoadLibrary( "GUIForms.dll" )) == NULL )
		Quit( "dll." );
	if( (pShowStartupForm = (void (__stdcall *)(void))GetProcAddress( DIHinst, "ShowStartupForm" )) == NULL )
		Quit( "GetProcAddress" );
	pShowStartupForm();
	***/

//#define ROTVAL ( deg2rad(2.0) )
//#define MOVEVAL ( 0.05f )

//#define CAM_ROTVAL ( deg2rad(5.0) )
//#define CAM_MOVEVAL ( 5.0f )

	/***
	char szPath[MAX_PATH];
	if( winBrowseForFolder( szPath ) == TRUE )
		Message( szPath );
	***/

	// dumplif( "jaguar1.lif" );

	// ReadL3D( "A_horse_boned.l3d" );

	// InitScent();
	// EmitScent( SCID_RASPBERRY );
	// DeinitScent();
	// xprintf("name: %s \"%s\" %s\n", male_firstname[rand()%NUMMALEFIRST], callsign_name[rand()%NUMCALLSIGNS], general_lastname[rand()%NUMLASTNAMES] );
	// xprintf("city: %s\n", city_name[rand()%NUMCITYNAMES] );
	// xprintf("corp: %s\n", corp_name[rand()%NUMCORPNAMES] );
	// xprintf("star: %s\n", star_name[rand()%NUMSTARNAMES] );

	// extern void winReadDisk( void );
	// winReadDisk();

#ifdef COMMENT
	{
	char *str[] = {
	"PROTECTEDFILE",
	"PROTECTEDFILEPATH",
	"ALTUSERNAME",
	"LOADINGWINDOW",
	};

	FILE *f = fopen("str.h","wt");

	for( i=0; i<4; i++ ) {

		int j;
		UCHAR *lz = NULL;

		j = EncodeLZA( (UCHAR *)str[i], strlen(str[i]) + 1, PTR(lz) ) - 12;

		fprintf(f, "int len = %d; UCHAR str%d[] = /* \"%s\" */ { ", strlen(str[i])+1, i, str[i] );

		for( k=0; k<j; k++ )
			fprintf(f, ",0x%02x", lz[12+k] );

		fprintf(f, "};\n" );

		FREEMEM(lz);
	}

	fclose(f);
	}
#endif

	// if( !isArmadillo() ) Quit("no armadillo.");

	DumpProcessInfo();

	AddSearchDir( "DATA\\" );

	/***
	char *buf = NULL;
	OpenSegFile( "proba.seg" );
	OpenZipFile( "proba.zip" );
	buf[ LoadFile( "1.txt", PTR(buf) ) ] = 0;
	Message( buf );
	{ FILE *f;
	if( (f = ffopen( "1.txt", "rt" )) != NULL ) {
		REALLOCMEM( buf, 1024 );
		ffscanf( f, "%s", buf );
		ffclose( f );
		Message( buf );
	}}
	if( buf ) FREEMEM( buf );
	***/

	/***
	MA_Init();
	MA_SendMail( "bernie@bkgames.com", "próba levél", "remélem sikerrel jártam. :)" );
	MA_SendDoc( "c:\\config.sys;c:\\autoexec.bat" );
	MA_Deinit();
	***/

	ROTVAL = winReadProfileFloat( "OBJECT_rotation", 5.0f );
	winWriteProfileFloat( "OBJECT_rotation", ROTVAL );
	ROTVAL = deg2rad( ROTVAL );

	MOVEVAL = winReadProfileFloat( "OBJECT_move", 5.0f );
	winWriteProfileFloat( "OBJECT_move", MOVEVAL );

	CAM_ROTVAL = winReadProfileFloat( "camera_rotation", 5.0f );
	winWriteProfileFloat( "camera_rotation", CAM_ROTVAL );
	CAM_ROTVAL = deg2rad( CAM_ROTVAL );

	CAM_MOVEVAL = winReadProfileFloat( "camera_move", 2.0f );
	winWriteProfileFloat( "camera_move", CAM_MOVEVAL );

	BODY_HEIGHT = CAM_MOVEVAL;

	r = winReadProfileInt( "red", 212 );
	CLAMPMINMAX( r, 0, 0xff );
	winWriteProfileInt( "red", r );

	g = winReadProfileInt( "green", 208 );
	CLAMPMINMAX( g, 0, 0xff );
	winWriteProfileInt( "green", g );

	b = winReadProfileInt( "blue", 200 );
	CLAMPMINMAX( b, 0, 0xff );
	winWriteProfileInt( "blue", b );

	clear_color = RGBINT24( r,g,b );

	anim_per_sec = winReadProfileFloat( "anim_per_sec", 12.0f );
	CLAMPMIN( anim_per_sec, FLOAT_EPSILON );
	winWriteProfileFloat( "anim_per_sec", anim_per_sec );

	anim_wait = (int)( (FLOAT)TICKBASE / anim_per_sec );

	if( NumArgs() == 0 )
		if( !winOpenFile( "ASC Files (*.asc)\0*.asc\0All Files (*.*)\0*.*\0") )
			Quit( NULL );

	/***
	md2_model_t *model;
	md2_anim_t	*anim;
	loadTGA("model\\drfreak.tga", 1);
	model = q2_loadmodel( "tris.md2" );
	anim = q2_makeanim( model );
	q2_dumpmodel( model, anim, "drfreak.bmp" );
	***/

	width = winReadProfileInt( "xlib_width", 800 );
	height = winReadProfileInt( "xlib_height", 600 );
	depth = winReadProfileInt( "xlib_depth", 16 );

	SetX( width, height, depth, SETX_NORMAL );

	InitKeyboard();

	InitTooltip();

/*
	LoadPcx("proba.pcx",PTR(spr));
	AddTexMap( spr, "PROBA" ); FREEMEM( spr );
	LoadPcx("shaded.pcx",PTR(spr));
	AddTexMap( spr, "SHADED" ); FREEMEM( spr );
*/

	memset( obj, 0, MAXOBJ * sizeof(object_ptr) );

	if( LoadObject("axis.asc",&obj[0]) == FALSE )
		Quit("no axis.asc.");

	nobjs = 0;
	curr_obj = 1;
	anim_flag = BOOLEAN( CheckParm("anim") );

	if( NumArgs() == 0 ) {
		while( ((txt=winGetNextFile()) != NULL) && (nobjs < MAXOBJ) ) {
			char *p;
			char dir[PATH_MAX];
			strcpy( dir, txt );
			if( (p=strrchr(dir,'\\')) != NULL ) {
				*(++p) = 0;
				AddSearchDir( dir );
			}
			if( LoadObject(txt,&obj[nobjs+1]) )
				++nobjs;
		}
	}
	else {
		for( i=0; (i<NumArgs()) && (nobjs < MAXOBJ); i++ ) {
			char *p;
			char dir[PATH_MAX];
			txt = GetArg(i);
			strcpy( dir, txt );
			if( (p=strrchr(dir,'\\')) != NULL ) {
				*(++p) = 0;
				AddSearchDir( dir );
			}
			if( LoadObject(txt,&obj[nobjs+1]) )
				++nobjs;
		}
	}

	// TomFlag( NOFLAG );
	// SaveAsc( obj[1] );

	if( nobjs > 1 )
		DupObject( obj[1], &inter );

	// winCopyTextToClipboard( "próba clipboárd text" );

	// if( obj[1] ) for(i=0;i<obj[1]->npoints;i++) { SWAPF( obj[1]->point[i][0], obj[1]->point[i][1] ); }

	MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
	MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );
	MAKEVECTOR( cam_up, 0.0, 0.0, 1.0 );
	MAKEVECTOR( cam_right, 0.0, 1.0, 0.0 );
	roll = 0.0;

	RotateCamera( cam_pos, cam_view, cam_right, cam_up, 0,0, TRUE );

	SetupShading( (FLOAT_MAX-1), FLOAT_MAX );
	AddShadeColor( 16, COLORDEPTH );
	AddShadeColor( 32, COLORDEPTH );
	AddShadeColor( 48, COLORDEPTH );
	AddShadeColor( 64, COLORDEPTH );

	fi=theta=ro=0.0;

	/***
	temp = 20.0;
	if( obj[1] ) for(i=0;i<obj[1]->npoints;i++) { obj[1]->point[i][0] += temp;obj[1]->point[i][1] += temp; obj[1]->point[i][2] -= temp; }
	if( obj[2] ) for(i=0;i<obj[2]->npoints;i++) { obj[2]->point[i][0] -= temp;obj[2]->point[i][1] -= temp; obj[2]->point[i][2] += temp; }
	if( obj[1] ) NormalizeObject( obj[1] );
	if( obj[2] ) NormalizeObject( obj[2] );
	****/

	SetRgbPcx();

#define PAL Phong
//#define PAL Dither
	PAL( 16, COLORDEPTH, 0,0,0, 50,50,50 );
	PAL( 32, COLORDEPTH, 5,5,5, 55,55,55 );
	PAL( 48, COLORDEPTH, 0,0,0, 50,50,50 );
	PAL( 64, COLORDEPTH, 5,5,5, 55,55,55 );
	PAL( 80, COLORDEPTH, 0,0,0, 50,50,50 );
#undef PAL

	MakePhongMap(COLORDEPTH);
	MakeTransColor(FALSE);

	//AddLight( LT_NORMAL, 250,250,250,255, 100.0f, origo3, cam_pos );  // like SUN
	sun = AddLight( LT_AMBIENT, 200,200,200,255, 100.0f, NULL, NULL );

	if( FileExist("back.pcx") ) {
		back_on = TRUE;
		LoadPcx("back.pcx",PTR(spr));
		ScaleSprite( SCREENW,SCREENH, spr, PTR(back) );
		GetRgbPcx( pal );
		HighSprite( PTR(back), pal );
		FREEMEM(spr);
		//memcpy( (UCHAR*)line_offset[0], &back[SPRITEHEADER], SPRITEW(back)*SPRITEH(back)*SPRPIXELLEN(back) );
		PutSprite( 0,0, back );
	}

#define ENVMAPNAME "env.bmp"

	if( FileExist(ENVMAPNAME) ) {
		i = 0;
		SETFLAG( i, TF_CHROMA );
		SETFLAG( i, TF_ALPHA );
		if( (texmapid = LoadTexmap( ENVMAPNAME, i, 0L )) != (-1) ) {
			for( j=0; j<nobjs; j++ ) {
				xprintf("envmapping: %s object.\n", obj[j+1]->name );
				EnvmapObject( obj[j+1], texmapid, 127 );
			}
			if( inter ) EnvmapObject( inter, texmapid, 192 );
		}
		ERASEFLAG( obj[1]->face[0].flag, FF_ENVMAP );
		ERASEFLAG( obj[1]->face[1].flag, FF_ENVMAP );
	}

	i = 0;
	SETFLAG( i, MF_3DCURSOR );
	if(!InitMouse(i)) xprintf("no mouse!\n");
	//HideMouse();

	CV_SetValue( "zclipfar", 250.0f );
	CV_SetValue( "zclipnear", 0.0f );

	int f = 1;
	// f = LoadFont3D( "exh.ttf", TRUE, FALSE, NULL, 0 ) - 1;
	// f = LoadFont3D( "font\\tower\\gold0.bmp", TRUE, FALSE, gold0_bmp, dimof(gold0_bmp) ) - 1;
	// f = LoadFont3D( "font\\nfk\\nfkfont.bmp", TRUE, FALSE, nfkfont_bmp, dimof(nfkfont_bmp) ) - 1;
	// f = LoadFont3D( "font\\ascii.bmp", TRUE, FALSE, asciifont_bmp, dimof(asciifont_bmp) ) - 1;
	// f = LoadFont3D( "font\\nehe\\nehe.bmp", TRUE, FALSE, nehefont1_bmp, dimof(nehefont1_bmp) ) - 1;
	// f = LoadFont3D( "font\\nehe\\nehe.bmp", TRUE, FALSE, nehefont2_bmp, dimof(nehefont2_bmp) ) - 1;
	// f = LoadFont3D( "font\\exocet.bmp", TRUE, FALSE, exocetfont_bmp, dimof(exocetfont_bmp) ) - 1;

	// selfModify();

#ifdef DRAWWHIRL
	InitWhirl();
#endif

	anim_tics = GetTic();
	tics=0L;
	done=FALSE;

	Clear(clear_color);

	while( !done ) {

#ifndef DRAWWHIRL
		//for(i=0;i<SCREENH;i++) Line(0,i,SCREENW,i,i);
		if( back==NULL ) Clear(clear_color);
		else PutSprite( 0,0, back ); //memcpy( (UCHAR*)line_offset[0], &back[SPRITEHEADER], SPRITEW(back)*SPRITEH(back)*SPRPIXELLEN(back) );
#endif

		if( !display_zprintf )
			key = GetKey();
		else
			key = key_None;

		if( key == key_Escape ) done = TRUE;

		if( keys[sc_F2] ) ScreenSave();
		if( keys[sc_F12] ) {
			if( gamma_corr.value >= 100.0f )
				CV_SetValue( "gamma_corr", 0.0f );
			else {
				gamma_corr.value += 20.0f;
				CLAMPMINMAX( gamma_corr.value, 0.0f, 100.0f );
				CV_SetValue( "gamma_corr", gamma_corr.value );
			}
			winWriteProfileInt("gamma_corr", (int)gamma_corr.value );
			ReloadAllTexMap(0);
		}

		// reset camera
		if( key == key_Enter ) {

			MAKEVECTOR( cam_view, 90.0, 90.0, 90.0 );
			MAKEVECTOR( cam_pos, 100.0, 100.0, 100.0 );
			MAKEVECTOR( cam_up, 0.0, 0.0, 1.0 );
			MAKEVECTOR( cam_right, 0.0, 1.0, 0.0 );
			roll = 0.0;

			RotateCamera( cam_pos, cam_view, cam_right, cam_up, 0,0, TRUE );
		}


		direction = MC_NONE;
		if( keys[sc_Up] || ( joyz > maxjoyz * 1 / 2) || (mousedz>0) ) direction = MC_FORWARD;
		if( keys[sc_Down] || (mousedz<0) ) direction = MC_BACKWARD;
		if( keys[sc_Left] || (joypov == 3) ) direction = MC_LEFT;
		if( keys[sc_Right] || (joypov == 1) ) direction = MC_RIGHT;
		if( keys[sc_PageUp] || (joypov == 2) ) direction = MC_UP;
		if( keys[sc_PageDown] || (joypov == 4) ) direction = MC_DOWN;

		if( direction ) {

			// try moving camera, collision detection

			CopyVector( temp_cam_pos, cam_pos );
			CopyVector( temp_cam_view, cam_view );

			if( mousedz )
				MoveCamera( cam_pos, cam_view, direction, CAM_MOVEVAL*10 );
			else
				MoveCamera( cam_pos, cam_view, direction, CAM_MOVEVAL );

			collide = FALSE;
			for( i=0; i<=nobjs; i++ ) {
#ifdef INCCOLDET
				if( obj[i]->model != NULL )
					if( (collide = obj[i]->model->sphereCollision( cam_pos, BODY_HEIGHT )) )
						break;
#endif
			}

			if( collide ) {
				CopyVector( cam_pos, temp_cam_pos );
				CopyVector( cam_view, temp_cam_view );
			}
		}


#define NORM if( 0 ) NormalizeObject( obj[i+1] )

		// test.asc
		if( keys[sc_Q] || ( joyy < maxjoyy * 1 / 4) ) { for( i=0; i<nobjs; i++ ) { RotateObject( obj[i+1], ROTVAL, 0.0, 0.0 );  NORM; } }
		if( keys[sc_A] || ( joyy > maxjoyy * 3 / 4) ) { for( i=0; i<nobjs; i++ ) { RotateObject( obj[i+1], -ROTVAL, 0.0, 0.0 ); NORM; } }
		if( keys[sc_W] ) { for( i=0; i<nobjs; i++ ) { RotateObject( obj[i+1], 0.0, ROTVAL,  0.0 ); NORM; } }
		if( keys[sc_S] ) { for( i=0; i<nobjs; i++ ) { RotateObject( obj[i+1], 0.0, -ROTVAL, 0.0 ); NORM; } }
		if( keys[sc_E] || ( joyx < maxjoyx * 1 / 4) ) { for( i=0; i<nobjs; i++ ) { RotateObject( obj[i+1], 0.0, 0.0, ROTVAL );  NORM; } }
		if( keys[sc_D] || ( joyx > maxjoyx * 3 / 4) ) { for( i=0; i<nobjs; i++ ) { RotateObject( obj[i+1], 0.0, 0.0, -ROTVAL ); NORM; } }

		if( keys[sc_1] ) { for( i=0; i<nobjs; i++ ) { MoveObject( obj[i+1], MOVEVAL, 0.0, 0.0 ); NORM; } }
		if( keys[sc_2] ) { for( i=0; i<nobjs; i++ ) { MoveObject( obj[i+1], -MOVEVAL, 0.0, 0.0 ); NORM; } }
		if( keys[sc_3] ) { for( i=0; i<nobjs; i++ ) { MoveObject( obj[i+1], 0.0, MOVEVAL, 0.0 ); NORM; } }
		if( keys[sc_4] ) { for( i=0; i<nobjs; i++ ) { MoveObject( obj[i+1],	0.0,-MOVEVAL, 0.0 ); NORM; } }
		if( keys[sc_5] ) { for( i=0; i<nobjs; i++ ) { MoveObject( obj[i+1], 0.0, 0.0, MOVEVAL ); NORM; } }
		if( keys[sc_6] ) { for( i=0; i<nobjs; i++ ) { MoveObject( obj[i+1],	0.0, 0.0,-MOVEVAL ); NORM; } }

#undef NORM

		if( keys[sc_T] ) RotateAround( cam_view, cam_pos, 360.0, cam_up );
		if( keys[sc_R] ) RotateAround( cam_view, cam_pos, ROTVAL, cam_up );
		if( keys[sc_F] ) RotateAround( cam_view, cam_pos, -ROTVAL, cam_up );

		if( keys[sc_Z] ) roll += ROTVAL*2;
		if( keys[sc_X] ) roll -= ROTVAL*2;


		// X engatív
		if( key == 'c' ) {
			for( j=0; j<nobjs; j++ ) {
				for( i=0; i<obj[j+1]->npoints; i++ ) {
					obj[j+1]->point[i][0] *= -1;
				}
				NormalizeObject( obj[j+1] );
			}
		}

		// Y engatív
		if( key == 'g' ) {
			for( j=0; j<nobjs; j++ ) {
				for( i=0; i<obj[j+1]->npoints; i++ ) {
					obj[j+1]->point[i][1] *= -1;
				}
				NormalizeObject( obj[j+1] );
			}
		}

		// Z engatív
		if( key == 'n' ) {
			for( j=0; j<nobjs; j++ ) {
				for( i=0; i<obj[j+1]->npoints; i++ )
					obj[j+1]->point[i][2] *= -1;
				NormalizeObject( obj[j+1] );
			}
		}

		// 0 -> 1
		if( key == 'g' ) {
			for( j=0; j<nobjs; j++ ) {
				for( i=0; i<obj[j+1]->npoints; i++ ) {
					FLOAT temp = obj[j+1]->point[i][0];
					obj[j+1]->point[i][0] = obj[j+1]->point[i][1];
					obj[j+1]->point[i][1] = temp;
				}
				NormalizeObject( obj[j+1] );
			}
		}

		// 0 -> 2
		if( key == 'h' ) {
			for( j=0; j<nobjs; j++ ) {
				for( i=0; i<obj[j+1]->npoints; i++ ) {
					FLOAT temp = obj[j+1]->point[i][0];
					obj[j+1]->point[i][0] = obj[j+1]->point[i][2];
					obj[j+1]->point[i][2] = temp;
				}
				NormalizeObject( obj[j+1] );
			}
		}

		// 1 -> 2
		if( key == 'j' ) {
			for( j=0; j<nobjs; j++ ) {
				for( i=0; i<obj[j+1]->npoints; i++ ) {
					FLOAT temp = obj[j+1]->point[i][1];
					obj[j+1]->point[i][1] = obj[j+1]->point[i][2];
					obj[j+1]->point[i][2] = temp;
				}
				NormalizeObject( obj[j+1] );
			}
		}

/*		{
			point3_t rotAmnt = { deg2rad(1.2f), deg2rad(0.4f), deg2rad(-0.75f) };

			for( i=0; i<nobjs; i++ ) { RotateObject( obj[i+1], rotAmnt[0], 0.0, 0.0 );  NormalizeObject( obj[i+1] ); }
			for( i=0; i<nobjs; i++ ) { RotateObject( obj[i+1], 0.0, rotAmnt[1], 0.0 ); NormalizeObject( obj[i+1] ); }
			for( i=0; i<nobjs; i++ ) { RotateObject( obj[i+1], 0.0, 0.0, rotAmnt[2] );  NormalizeObject( obj[i+1] ); }
		}
*/

		if( (key == '-') || (key == ',') ) {
			for( j=0; j<nobjs; j++ ) {
				for( i=0; i<obj[j+1]->npoints; i++ ) {
					obj[j+1]->point[i][0] /= zoom;
					obj[j+1]->point[i][1] /= zoom;
					obj[j+1]->point[i][2] /= zoom;
				}
				NormalizeObject( obj[j+1] );
			}
		}

		if( (key == '+') || (key == '.') ) {
			for( j=0; j<nobjs; j++ ) {
				for( i=0; i<obj[j+1]->npoints; i++ ) {
					obj[j+1]->point[i][0] *= zoom;
					obj[j+1]->point[i][1] *= zoom;
					obj[j+1]->point[i][2] *= zoom;
				}
				NormalizeObject( obj[j+1] );
			}
		}

		if( (key == '{') || (key == '[') )
			if( --curr_obj < 1 )
				curr_obj = nobjs;

		if( (key == '}') || (key == ']') )
			if( ++curr_obj > nobjs )
				curr_obj = 1;

		if( key == ' ' ) {
			anim_flag ^= 1;
			if( anim_flag ) MakeDpMouse();
			else MakeHourGlassMouse();
		}

		if( anim_flag ) {

			int tic = GetTic();

			// funky interpolate

			FLOAT scale = (FLOAT)(tic - anim_tics) / (FLOAT)anim_wait;
			InterpolateObject( obj[curr_obj], obj[(curr_obj+1)>nobjs?1:(curr_obj+1)], inter, scale );
			if( inter ) NormalizeObject( inter );

			if( (tic - anim_tics) > anim_wait ) {
				anim_tics = tic;
				if( ++curr_obj > nobjs )
					curr_obj = 1;
			}
		}

		if( key == 'b' ) {
			FLIPFLAG( engine_flag, EF_BBOX );
		}

		if( key == EXT(sc_F12) ) {
			display_zprintf ^= 1;
		}

		CLAMPMINMAX( curr_obj, 1, nobjs );


		#define LIGHT_DELTA	30
		static int light = 0;

		// fény kapcsoló
		if( key == 'l' ) {

			if( sun->r >= 200 )
				light = -LIGHT_DELTA;
			else
			if( sun->r <= 20 )
				light = +LIGHT_DELTA;
			else
				light *= -1;
		}

		if( light != 0 ) {
			sun->r += light;
			sun->g += light;
			sun->b += light;

			if( (light > 0) && (sun->r >= 200) ) light = 0;
			if( (light < 0) && (sun->r <= 20 ) ) light = 0;

			CLAMPMINMAX( sun->r, 20, 200 );
			CLAMPMINMAX( sun->g, 20, 200 );
			CLAMPMINMAX( sun->b, 20, 200 );
		}

		if( mousebld == TRUE ) {

			/* Compute the rolling ball axis and angle from the incremental mouse
			 * displacements (dx,dy) and compute corresponding rotation matrix RMat.
			 * See text for full form of Rmat returned by Make3DRot.
			 * NOTE: For window systems using a left-handed screen
			 * coordinate system, the formula (-dy,dx,0) given
			 * in the text for the rotation axis direction must
			 * be changed to (+dy,dx,0) to give the desired effect!
			 * We explicitly use this coordinate system in the example
			 * code because so many systems possess this reversal.
			 *
			 * Make3DRot  is assumed to construct a 4x4 rotation matrix from
			 * the angle and axis parameters as shown in the text and in
			 * GGI "Rotation Tools," by M. Pique, p.466.
			 */

			if( mousedx || mousedy ) {

				point3_t n;
				matrix_t m;
				FLOAT cos_theta, sin_theta;

				cos_theta = mousedx*1;

				GetCameraVector( NULL, n, NULL );
				MatrixRotateAxis( n, -cos_theta, m );
				for( i=0; i<nobjs; i++ ) { MulMatrixObject( m, obj[i+1] ); NormalizeObject( obj[i+1] ); }

				sin_theta = mousedy*1;

				GetCameraVector( NULL, NULL, n );
				MatrixRotateAxis( n, sin_theta, m );
				for( i=0; i<nobjs; i++ ) { MulMatrixObject( m, obj[i+1] ); NormalizeObject( obj[i+1] ); }

				// xprintf("mouse: %d, %d  %.2f, %.2f\n", mousedx,mousedy, cos_theta, sin_theta );
			}
		}

		if( mousebrd == TRUE ) {
			// RotateVectorVector( cam_view, cam_pos, 0.0, 0.0, deg2rad(-mousedx/4.0) );
			// Nutacio( cam_view, cam_pos, deg2rad((flipy*mousedy)/8.0) );
					// cam_view[2] += (-mousedy/20.0);
		}

		RotateCamera( cam_pos, cam_view, cam_right, cam_up, (mousebrd&&mousedy)?((flipy*mousedy)/8.0f):0.0f, (mousebrd&&mousedx)?(mousedx/8.0f):0.0f );


		if( mousebmd == TRUE ) {

			alpha = 255 - mousey*255/SCREENH;

			CLAMPMINMAX( alpha, 0, 255 );

			for( i=0; i<nobjs; i++ )
				for( j=0; j<obj[i+1]->nfaces; j++ )
					for( k=0; k<obj[i+1]->face[j].nverts; k++ )
					obj[i+1]->face[j].rgb[k].a = alpha;
		}
		// mousedx = 0;
		// mousedy = 0;

		//fi = ROTVAL;
		//theta = ROTVAL;
		//ro = ROTVAL;
		//RotateVectorVector( cam_pos, cam_view /*obj->point[2]*/, fi, theta, ro );


		//NormalizeObject( obj[0] );
		//NormalizeObject( obj[1] );
		//NormalizeObject( obj[2] );

		BeginScene();

		SetupProjection( CLIPMINX, CLIPMINY,
				 CLIPMAXX - CLIPMINX,
				 CLIPMAXY - CLIPMINY );

		SetupCulling( cam_pos, cam_view );
		SetupFog( zclipfar.value/2.0f, zclipfar.value, 0x000000 );

		// GenMatrix( cam_pos, cam_view, NULL, NULL, 0.0f );
		GenMatrix( cam_pos, cam_view, cam_right, cam_up, 0.0f );

#ifdef DRAWWHIRL
		DrawWhirl();
#endif

		if( obj[0] ) TransformObject( obj[0] );
		if( anim_flag ) {
			if( inter ) TransformObject( inter );
		}
		else
		if( obj[curr_obj] )
			TransformObject( obj[curr_obj] );


		// if( mousebld ) { static int x=0, y=0; if( mousebl ) { x=mousex; y=mousey; } Line(x,y,mousex,mousey,RGBINT24(50,250,50) ); }
		// if( mousebrd ) { static int x=0, y=0; if( mousebr ) { x=mousex; y=mousey; } Line(mousex,mousey,x,y,RGBINT24(250,50,50) ); }

		FlushScene();

		// spinFlip();
		// FlushScene();

		// vertex
		if( keys[sc_V] ) {

			int dx=INT_MAX,dy=INT_MAX;
			int v = 0;

			for( i=0; i<obj[curr_obj]->npoints; i++ )
				if( (ABS(obj[curr_obj]->ppoint[i][0] - mousex) < dx) &&
					(ABS(obj[curr_obj]->ppoint[i][1] - mousey) < dy) ) {
					v = i;
					dx = ABS(obj[curr_obj]->ppoint[i][0] - mousex);
					dy = ABS(obj[curr_obj]->ppoint[i][1] - mousey);
				}

			// MoveMouse( 10,10 );

			rgb_t color = {255,255,255,255 };

			Line( obj[curr_obj]->ppoint[v][0] - 10, obj[curr_obj]->ppoint[v][1] - 10,
					obj[curr_obj]->ppoint[v][0] + 10, obj[curr_obj]->ppoint[v][1] + 10, color );
			Line( obj[curr_obj]->ppoint[v][0] + 10, obj[curr_obj]->ppoint[v][1] - 10,
					obj[curr_obj]->ppoint[v][0] - 10, obj[curr_obj]->ppoint[v][1] + 10, color );

			SetFont(f);
			SetFontAlpha( FA_ALL, 255,255,255, 255 );
			WriteString( 10,250,"vertex: %d", v );
		}


		DrawTooltip( mytooltip, dimof(mytooltip) );

		consoleAnimate();

		DrawMouse();

		// showNuke(mousex, mousey );

		SetFont(f);
		SetFontAlpha( FA_ALL, 255,255,255, 255 );
		WriteString(10,10,"memory used: %d\nframe/sec: %d\nvideo: %dx%dx%d\nobj: %d of %d \"%s\"\nalpha: %d\nobjs: %d, %d\nbbox: %d, %d",(int)mem_used,(int)frame,SCREENW,SCREENH,bpp,curr_obj,nobjs,obj[curr_obj]->name,alpha,renderinfo.rejectedobjs, renderinfo.objs,renderinfo.rejectedbboxfacenum, renderinfo.bboxfacenum);

		if( winJoystick() )
			WriteString( 10,100, "joyx: %d, %d\njoyy: %d, %d\njoyz: %d, %d\njoypov: %d\nbutt: %d, %d, %d, %d", joyx, maxjoyx, joyy, maxjoyy, joyz, maxjoyz, joypov, joyb[0], joyb[1], joyb[2], joyb[3] );

		EndScene();

		FlipPage();

		if(tics+TICKBASE*2<GetTic()) { tics = GetTic(); frame = ++framecnt/2; framecnt=0; }
		else { ++framecnt; }
	}

	// ScreenSave();

	TomFlag( NOFLAG );
	if( CheckParm("save") )
		for( i=0; i<nobjs; i++ )
			SaveAsc( obj[i+1] );

	ResetX();

	DiscardAllTexmap();
	DiscardObject( &obj[0] );
	for( i=0; i<nobjs; i++ ) DiscardObject( &obj[i+1] );
	if( inter ) DiscardObject( &inter );

	if( back ) FREEMEM(back);

	FREEMEM( transparent_rgb );

	return;
}


/***

Public Const HKEY_CLASSES_ROOT = &H80000000
Public Const ERROR_ACCESS_DENIED = 8
Public Const ERROR_BADDB = 1
Public Const ERROR_BADKEY = 2
Public Const ERROR_CANTOPEN = 3
Public Const ERROR_CANTREAD = 4
Public Const ERROR_CANTWRITE = 5
Public Const ERROR_INVALID_PARAMETER = 7
Public Const ERROR_OUTOFMEMORY = 6
Public Const ERROR_SUCCESS = 0
Public Const MAX_PATH = 256
Public Const REG_SZ = 1
Public Declare Function RegCreateKey Lib "advapi32.DLL" Alias "RegCreateKeyA" (ByVal hKey _
    As Long, ByVal lpszSubKey As String, lphKey As Long) As Long
Public Declare Function RegSetValue Lib "advapi32.DLL" Alias "RegSetValueA" (ByVal hKey As _
    Long, ByVal lpszSubKey As String, ByVal fdwType As Long, ByVal lpszValue As String, _
    ByVal dwLength As Long) As Long
Public Sub SetKey()
    Dim sKeyName As String
    Dim sKeyValue As String
    Dim ret As Long
    Dim lphKey As Long
    sKeyName = "MyApp"
    sKeyValue = "My Application"
    ret = RegCreateKey(HKEY_CLASSES_ROOT, sKeyName, lphKey)
    ret = RegSetValue(lphKey, "", REG_SZ, sKeyValue, 0)
    sKeyName = ".bar"
    sKeyValue = "MyApp"
    ret = RegCreateKey(HKEY_CLASSES_ROOT, sKeyName, lphKey)
    ret = RegSetValue(lphKey, "", REG_SZ, sKeyValue, 0)
    sKeyName = "MyApp"
    sKeyValue = "notepad.exe %1"
    ret = RegCreateKey(HKEY_CLASSES_ROOT, sKeyName, lphKey)
    ret = RegSetValue(lphKey, "shell\open\command", REG_SZ, sKeyValue, MAX_PATH)
End Sub


Public Function Adler32(B() As Byte, Optional ByVal LastCRC As Long = 1) As Long
    'NOTE: LastCRC is used to seed the function with a prior calculation, so
    'large byte sequences can be broken into more manageable sequences ...
    Const CrcBase As Long = 65521
    Dim S1 As Long
    Dim S2 As Long
    Dim N As Long
    S1 = LastCRC And &HFFFF
    S2 = (LastCRC / 65536) And &HFFFF
    For N = LBound(B) To UBound(B)
        S1 = (S1 + B(N)) Mod CrcBase
        S2 = (S2 + S1) Mod CrcBase
    Next
    Adler32 = (S2 * 65536) + S1
End Function

***/

/***

Print "Welcome to the twilight zone :)"
Print ""
Print ""
Print "for each 'seed' point of the grid ,"
Print "         do this n time  ( n=50 ) :"    ; n is itr
Print ""
Print "   f(x) = xold - t * ( Sin( xold + sin( 3*yold ) ) ) "
Print "   f(y) = yold + t * ( Sin( yold + sin( 3*xold ) ) ) "
Print ""
Print " [   ESC ]     quit"
Print " [    Up ]     zoom"
Print " [  Down ]     unzoom"
Print " [  Left ]     increase t"
Print " [ Right ]     decrease t"
Print " [ Pg Up ]     increase number of seeds"
Print " [ Pg Dn ]     decrease number of seeds"
Print ""
Print " press a key..."
WaitKey()

;--------------------------------------------------------------------------
 Const dwidth = 1024              ;display width
Const dheight = 768              ;display height
    Const nbc = 256               ;number of values for rgb
   Const drs% = dheight/2         ;max square resolution display
         res# = 10                ;seeds points number (per row)
   Const itr% = 50                ;iterations maximum (length of branches)
    Const xc% = 0                 ;center x value
    Const yc% = 0                 ;center y value
          ti# = 8                 ;(bnd*2)/res     ;increment constant (seeds points)
           z# = 8                 ;z will pass boundaries values (zoom)
           t# =.1                 ;function factor
Const deg_fac#=180/Pi             ;used by deg function
Const rad_fac#=Pi/180             ;used by rad function
     refresh% = 1                 ;wether screen refresh is needed (1) or not (0)
Print " Go!!"
;-Main---------------------------------------------------------------------
Graphics dwidth,dheight           ;enter graphic mode
SetBuffer BackBuffer()            ;use 'no hassle' double buffering :)
Origin dwidth/2,dheight/2         ;set the 0,0 point for drawing to middle of screen

 While Not KeyDown(1)             ;While not [ESC]

    If KeyDown(200)               ;if up arrow
      z = .98*z                   ;zoom
      refresh=1                   ;need refresh
    EndIf
    If KeyDown(208)               ;if down arrow
      z = z/.98                   ;unzoom
       refresh=1
    EndIf
    If KeyDown(203)               ;if left arrow
      t = t+.01                   ;increase t
      refresh=1
    EndIf
    If KeyDown(205)               ;if right arrow
      t = t-.01                   ;decrease t
      refresh=1
    EndIf
    If KeyDown(201)               ;if page up
      res = res+1                 ;increase res
      refresh=1
    EndIf
    If KeyDown(209)               ;if page down
      res = res-1                 ;decrease res
      If res<=1 Then res=1        ;to avoid division by zero in frame function
      refresh=1
    EndIf

    If refresh=1                  ;check if any refresh is needed
      Cls                         ;delete curent buffer
      refresh = 0
      doit(z,t,res)               ;calc a frame
      Flip                        ;flip drawbuffer
    EndIf
 Wend
EndGraphics                       ;quit graphic mode
End                               ;bye
;--------------------------------------------------------------------------
Function doit(bnd#,t#,res#)
 Local i%                         ;iteration counter
 Local xb# = bnd                  ;x boundaries
 Local yb# = bnd                  ;y boundaries
 Local df# = drs/bnd              ;display factor
 Local xi# = xc-xb                ;initial value of x (upper left seed)
 Local yi# = yc-yb                ;initial value of y
 Local  x# = 0                    ;maths' x,
 Local  y# = 0                    ;         y
 Local sx# = xi                   ;current seed
 Local sy# = yi                   ;            x,y
 Local xo# = 0 : yo# = 0          ;allow reference to former iteration x or y value
 Local ti# = (bnd*2)/res          ;seeds incrementation
  While sy <= yb
     While sx <= xb                ;seeds forms a grid
        x = sx : y = sy           ;current seed
        For i=0 To itr            ;
          xo = x : yo = y         ;remember former value
;-----------------                ;if you want to experiment with different pics try changing this
          x = xo-t*Sin(deg(Sin(deg(3*yo))+xo)) ;new x value
          y = yo+t*Sin(deg(Sin(deg(3*xo))+yo)) ;New y value
;-----------------
          Color (i*i) Mod nbc,(Abs(sx)*i) Mod nbc,(Abs(sy)*i) Mod nbc   ;calculate some color :)
;          Rect df*x,df*y,1,1      ;draw a point (fast)
         Plot df*x,df*y          ;draw a point
        Next
        sx=sx+ti                  ;next seed point
     Wend
     sy=sy+ti                     ;next seed row
     sx=xi                        ;beginning of row
   Wend
End Function
;--------------------------------------------------------------------------
Function rad#(deg_ang#)           ;convert degrees to radians
  deg_ang=deg_ang*rad_fac         ;Const rad_fac#=Pi/180
 Return deg_ang
End Function
;--------------------------------------------------------------------------
Function deg#(rad_ang#)           ;convert radians to degrees
  rad_ang=rad_ang*deg_fac         ;Const deg_fac#=180/Pi
 Return rad_ang
End Function
;--------------------------------------------------------------------------


//
//
//
void GenTorus( void ) {

	int nverts=0,nfaces=0;
	int HorAngle,VertAngle,Faces[3];
	double CX,CY,X,Y,Z;

	// verts: 160
	// facez: 320

	printf("\n; generated by GenTorus by (c) 1997 bernie\n\n%d %d\n\n\n",160,320);

	for( HorAngle=0; HorAngle<16; HorAngle++ ) {

		CX = cos(HorAngle/2.546479089)*10;
		CY = sin(HorAngle/2.546479089)*10;

		for( VertAngle=0; VertAngle<10; VertAngle++ ) {

			X = CX+cos(VertAngle/1.592)*cos(HorAngle/2.546)*5;
			Y = CY+cos(VertAngle/1.592)*sin(HorAngle/2.546)*5;
			Z = sin(VertAngle/1.59154931)*5;

			printf("%.3f %.3f %.3f\n",X,Y,Z);
			++nverts;
		}
	}


	printf("\n\n");


	for( HorAngle=0; HorAngle<16; HorAngle++ ) {

		for( VertAngle=0; VertAngle<10; VertAngle++ ) {

			Faces[0] = HorAngle*10+VertAngle+1;
			Faces[1] = HorAngle*10+(VertAngle+1) % 10+1;
			Faces[2] = (HorAngle*10+VertAngle+10) % 160+1;

			Faces[0] -= 1;
			Faces[1] -= 1;
			Faces[2] -= 1;

			printf("3  %d %d %d  texture 0  63 0 0 63 63 63\n",Faces[1],Faces[2],Faces[0]);

			Faces[0] = HorAngle*10+(VertAngle+1) % 10+1;
			Faces[1] = (HorAngle*10+(VertAngle+1) % 10+10) % 160+1;
			Faces[2] = (HorAngle*10+VertAngle+10) % 160+1;

			Faces[0] -= 1;
			Faces[1] -= 1;
			Faces[2] -= 1;

			printf("3  %d %d %d  texture 0  0 0 0 63 63 0\n",Faces[1],Faces[2],Faces[0]);

			++nfaces;
			++nfaces;

		}
	}

	printf("\n\n; nverts: %d  nfaces: %d\n",nverts,nfaces);


	return 0;
}

***/


/***

Public Function CapturePicture(ByVal hDC As Long, ByVal Left As Long, ByVal Top As Long, ByVal Width As Long, ByVal Height As Long) As IPictureDisp
    Dim picGuid As IID
    Dim picDesc As PICTDESC
    Dim hdcMem As Long
    Dim hBmp As Long
    Dim hOldBmp As Long
    Dim Pic As IPictureDisp
    Dim rcBitmap As RECT

    ' IID_IPictureDisp
    picGuid.X = &H7BF80981
    picGuid.s1 = &HBF32
    picGuid.s2 = &H101A
    picGuid.C(0) = &H8B
    picGuid.C(1) = &HBB
    picGuid.C(2) = &H0
    picGuid.C(3) = &HAA
    picGuid.C(4) = &H0
    picGuid.C(5) = &H30
    picGuid.C(6) = &HC
    picGuid.C(7) = &HAB

    picDesc.cbSizeOfStruct = Len(picDesc)
    hdcMem = CreateCompatibleDC(hDC)
    If hdcMem = 0 Then
        Exit Function
    End If
    hBmp = CreateCompatibleBitmap(hDC, Width, Height)
    If hBmp = 0 Then
        DeleteDC hdcMem
        Exit Function
    End If
    hOldBmp = SelectObject(hdcMem, hBmp)
    If Left >= 0 And Top >= 0 Then
        If BitBlt(hdcMem, 0, 0, Width, Height, hDC, Left, Top, SRCCOPY) = 0 Then
            SelectObject hdcMem, hOldBmp
            DeleteDC hdcMem
            DeleteObject hBmp
            Exit Function
        End If
    Else
        With rcBitmap
           .Left = 0
           .Top = 0
           .Right = Width
           .Bottom = Height
        End With
        FillRect hdcMem, rcBitmap, GetStockObject(BLACK_BRUSH)
    End If
    SelectObject hdcMem, hOldBmp
    picDesc.hBitmap = hBmp
    picDesc.hpal = GetCurrentObject(hDC, OBJ_PAL)
    picDesc.picType = PICTYPE_BITMAP
    If OleCreatePictureIndirect(picDesc, picGuid, True, Pic) <> S_OK Then
        DeleteDC hdcMem
        DeleteObject hBmp
        Exit Function
    End If
    Set CapturePicture = Pic
    Set Pic = Nothing
    DeleteDC hdcMem
End Function


BOOL FindRegisteredGameDev(char *szOutBuffer, UINT cbBuf)
{
   CLSID clsid;
   LPOLESTR pClsId;
   char szBuf[128];
   char szBuf2[128];
   HKEY hKey;
   long cbRegValSize = cbBuf;

   if (S_OK != CLSIDFromProgID(L"GameDev.Engine", &clsid))
      return FALSE;

   StringFromCLSID(clsid, &pClsId);
   WideCharToMultiByte(CP_ACP, 0, pClsId, -1, szBuf, sizeof(szBuf), NULL, NULL);
   CoTaskMemFree(pClsId);

   wsprintf(szBuf2, "CLSID\\%s\\LocalServer32", szBuf);

   if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CLASSES_ROOT, szBuf2, 0, KEY_QUERY_VALUE, &hKey))
      return FALSE;

   if (ERROR_SUCCESS != RegQueryValue(hKey, NULL, szOutBuffer, &cbRegValSize))
   {
      RegCloseKey(hKey);
      return FALSE;
   }

   RegCloseKey(hKey);

   return TRUE;
}

***/


// Plants versus zombies
// Bejeweled
// Minecraft
// Angry Birds
