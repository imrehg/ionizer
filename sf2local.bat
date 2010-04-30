echo Copy SourceForge updates to local source tree
svn up

set aluminizer_dir=..\..\control_trunk\Ionizer
set aluminizerFPGA_dir=..\..\control_trunk\IonizerES

xcopy /D /S /Y aluminizer\*.cpp %aluminizer_dir%
xcopy /D /S /Y aluminizer\*.c %aluminizer_dir%
xcopy /D /S /Y aluminizer\*.h %aluminizer_dir%
xcopy /D /S /Y aluminizer\*.ico %aluminizer_dir%
xcopy /D /S /Y aluminizer\*.pro %aluminizer_dir%

xcopy /D /S /Y aluminizerFPGA\*.cpp %aluminizerFPGA_dir%
xcopy /D /S /Y aluminizerFPGA\*.c %aluminizerFPGA_dir%
xcopy /D /S /Y aluminizerFPGA\*.h %aluminizerFPGA_dir%

xcopy /D /S /Y shared\src\*.cpp ..\..\control_trunk\shared\src
xcopy /D /S /Y shared\src\*.h ..\..\control_trunk\shared\src

xcopy /D /S /Y deploy\*.bat ..\..\control_trunk\deploy
xcopy /D /S /Y deploy\*.pri ..\..\control_trunk\deploy


cd ..\..\control_trunk\ionizer
cmd /C qupdate.bat
cd ..\..\sf\ionizer