CC               = cl
CFLAGS           = -W3 -DCRTAPI1=_cdecl -DCRTAPI2=_cdecl -nologo \
		   -D_WINNT -D_WIN32_WINNT=0x0400 -DWINVER=0x0400 /I.
CVARS            = -DWIN32 -DWIN64 -D_WIN32 -D_MT -D_DLL -MD -DWINDOWS -DUSE_TCL_STUBS -DUSE_INTERP_RESULT
INCLUDES	 = -I c:\usr\local\include -I..\src

LINK             = link
LINKDEBUG        = -debug:full -debugtype:cv
LDFLAGS          = /INCREMENTAL:NO /PDB:NONE /RELEASE /NOLOGO \
		 /MACHINE:X64 -dll /NODEFAULTLIB:LIBCMT \
		/LIBPATH:"c:\\usr\\local\\lib"

TCLLIBS          = tclstub86.lib
ZLIB		 = zdll.lib
PDFLIB		 = pdflib64.lib
LABLIB           = lablib64.lib
OBJ	= .obj
EXE	= .exe

LIBDIR = c:\\usr\\local\\lib
DLLDIR = c:\\usr\\local\\bin

DLLS = dlsh64.dll

OBJECTS = base_cg.obj cg_mswin.obj dfana.obj dfevt.obj dlarith.obj dlsh_pkg.obj dmana.obj tcl_df.obj tcl_dl.obj tcl_dlg.obj tcl_dm.obj

all: $(DLLS)

dlsh64.dll:	$(OBJECTS) dlsh.def
	$(LINK) $(LDFLAGS) -out:dlsh64.dll -def:dlsh.def $(OBJECTS) $(TCLLIBS) $(LABLIB) $(LZ4LIB) $(PDFLIB) user32.lib gdi32.lib

%$(OBJ): ..\src\%.c
	$(CC) -c $(CFLAGS) $(CVARS) $(INCLUDES) $<

