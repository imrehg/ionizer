cd ..
mkdir thirdparty
cd thirdparty

svn co https://qwt.svn.sourceforge.net/svnroot/qwt/branches/qwt-5.2
copy /Y ../deploy/qwtconfig.pri qwt-5.2

cd qwt-5.2

qmake
nmake

copy lib\*.dll %QtInstallDir%\bin
copy lib\*.lib %QtInstallDir%\lib

mkdir  %QtInstallDir%\include\qwt
copy /Y src/*.h %QtInstallDir%\include\qwt

nmake clean
cd ..\..\deploy

