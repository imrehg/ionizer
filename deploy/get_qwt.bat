echo Make sure that the environment variable QtInstallDir is set correctly.
echo QtInstallDir = %QtInstallDir%

rmdir /S /Q qwt-5.2
svn co https://qwt.svn.sourceforge.net/svnroot/qwt/branches/qwt-5.2
copy /Y qwtconfig.pri qwt-5.2

cd qwt-5.2

qmake
nmake

copy /Y lib\*.dll %QtInstallDir%\bin
copy /Y lib\*.lib %QtInstallDir%\lib

mkdir  %QtInstallDir%\include\qwt
copy /Y src\*.h %QtInstallDir%\include\qwt

nmake clean
cd ..
