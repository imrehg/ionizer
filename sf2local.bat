echo Copy SourceForge updates to local source tree
svn up
xcopy /D /S /Y aluminizer\*.cpp ..\..\control_trunk\ionizer
xcopy /D /S /Y aluminizer\*.c ..\..\control_trunk\ionizer
xcopy /D /S /Y aluminizer\*.h ..\..\control_trunk\ionizer
xcopy /D /S /Y aluminizer\*.ico ..\..\control_trunk\ionizer
xcopy /D /S /Y aluminizer\*.pro ..\..\control_trunk\ionizer

xcopy /D /S /Y aluminizerFPGA\*.cpp ..\..\control_trunk\ionizerES
xcopy /D /S /Y aluminizerFPGA\*.c ..\..\control_trunk\ionizerES
xcopy /D /S /Y aluminizerFPGA\*.h ..\..\control_trunk\ionizerES

xcopy /D /S /Y shared\src\*.cpp ..\..\control_trunk\shared\src
xcopy /D /S /Y shared\src\*.h ..\..\control_trunk\shared\src

xcopy /D /S /Y deploy\*.bat ..\..\control_trunk\deploy
xcopy /D /S /Y deploy\*.pri ..\..\control_trunk\deploy


cd ..\..\control_trunk\ionizer
cmd /C qupdate.bat
cd ..\..\sf\ionizer