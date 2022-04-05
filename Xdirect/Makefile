
OS = \
dx.obj \
network.obj \
mem.obj \
gfx.obj \
misc.obj \
rgb.obj \
file.obj \
ffile.obj \
segshell.obj \
rle.obj \
lza.obj \
lzh.obj \
lzs.obj \
uu.obj \
bitmap.obj \
timer.obj \
mouse.obj \
spr.obj \
font.obj \
kbd.obj \
pcx.obj \
trig.obj \
selector.obj \
raw.obj \
bmp.obj \
xmain.obj \
win.obj \
fixed.obj \
getdxver.obj \
xlib.res

XLIB = C:\Xdirect\Lib\xlib.lib

CFLAGS = -W3 -Zi -Gf -DXLIB -D__MSC__ -D__WIN32__ -MD
## CFLAGS = -W3 -Ot -G5 -Zi -Gf -DXLIB -D__MSC__ -D__WIN32__ -MD
## CFLAGS = -W4 -Zi -Gf -DXLIB

all: $(XLIB)

$(XLIB): $(OS)



.cpp.obj:
	cl -c $(CFLAGS) $** -Fo$@
	lib $(XLIB) $@

.c.obj:
	cl -c $(CFLAGS) $** -Fo$@
	lib $(XLIB) $@

.rc.res:
	rc -r -DWIN32 -D_WIN32 -DWINVER=0x0400 -fo $@ $**

LDTEMP = link.rsp

clean:
	@echo $(OS) > $(LDTEMP)
	@echo vc50.pdb >> $(LDTEMP)
	@echo vc60.pdb >> $(LDTEMP)
	@echo $(LDTEMP) >> $(LDTEMP)
	xdel @$(LDTEMP)

