;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "dlsh"
  OutFile "dlsh-win-arm64.exe"
  Unicode True

  ;Default installation folder
  InstallDir "$LOCALAPPDATA\dlsh"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\dlsh" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "dlsh" SecDlsh

  SetOutPath "$INSTDIR"
  
  File "dlsh.exe"
  File "tcl90.dll"
  File "dlsh.zip"
  File "zlib1.dll"
  File "libtommath.dll"
  File "fltk_dll.dll"
  File "fltk_z_dll.dll"
  File "fltk_png_dll.dll"
  File "fltk_jpeg_dll.dll"

  ;Store installation folder
  WriteRegStr HKCU "Software\dlsh" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "Desktop Shortcut" SectionX
    SetShellVarContext current
    CreateShortCut "$DESKTOP\dlsh.lnk" "$INSTDIR\dlsh.exe"
SectionEnd


;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\dlsh.exe"
  Delete "$INSTDIR\tcl90.dll"
  Delete "$INSTDIR\dlsh.zip"
  Delete "$INSTDIR\zlib1.dll"
  Delete "$INSTDIR\libtommath.dll"
  Delete "$INSTDIR\fltk_dll.dll"
  Delete "$INSTDIR\fltk_z_dll.dll"
  Delete "$INSTDIR\fltk_png_dll.dll"
  Delete "$INSTDIR\fltk_jpeg_dll.dll"
  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  DeleteRegKey /ifempty HKCU "Software\Essgui"

SectionEnd
 
