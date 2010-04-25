echo Copy SourceForge updates to local source tree
svn up
xcopy /D /S /Y aluminizer\*.cpp ..\..\control_trunk\ionizer
xcopy /D /S /Y aluminizer\*.c ..\..\control_trunk\ionizer
xcopy /D /S /Y aluminizer\*.h ..\..\control_trunk\ionizer
xcopy /D /S /Y aluminizer\*.ico ..\..\control_trunk\ionizer

xcopy /D /S /Y aluminizerFPGA\*.cpp ..\..\control_trunk\ionizerES
xcopy /D /S /Y aluminizerFPGA\*.c ..\..\control_trunk\ionizerES
xcopy /D /S /Y aluminizerFPGA\*.h ..\..\control_trunk\ionizerES

xcopy /U /S /Y shared\src\data_plot.cpp ..\..\control_trunk\shared\src
xcopy /U /S /Y shared\src\data_plot.h ..\..\control_trunk\shared\src

cd ..\..\control_trunk\ionizer
cmd /C qupdate.bat
cd ..\..\sf\ionizer