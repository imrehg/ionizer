; example1.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply 
; prompts the user asking them where to install, and drops a copy of example1.nsi
; there. 

;--------------------------------

; The name of the installer
Name "LaserBrothers"

; The file to write
OutFile "LaserBrothersSetup.exe"

; The default installation directory
InstallDir $desktop\LaserBrothers

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

 PageEx license
   LicenseData license.txt
 PageExEnd

Page directory
Page instfiles


;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File *.dll
  File *.exe
  
SectionEnd ; end the section
