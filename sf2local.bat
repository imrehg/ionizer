echo Copy SourceForge updates to local source tree
svn up
xcopy /D /S /Y aluminizer\*.cpp ..\..\control_trunk\ionizer
xcopy /D /S /Y aluminizer\*.c ..\..\control_trunk\ionizer
xcopy /D /S /Y aluminizer\*.h ..\..\control_trunk\ionizer

xcopy /D /S /Y aluminizerFPGA\*.cpp ..\..\control_trunk\ionizerES
xcopy /D /S /Y aluminizerFPGA\*.c ..\..\control_trunk\ionizerES
xcopy /D /S /Y aluminizerFPGA\*.h ..\..\control_trunk\ionizerES

cd ..\..\control_trunk\ionizer
qupdate
cd ..\..\sf\ionizer