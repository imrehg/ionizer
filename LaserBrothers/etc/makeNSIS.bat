RMDIR /S /Q NSIS
MKDIR NSIS
copy C:\QT\4.6.2\BIN\QtCore4.DLL nsis
copy C:\QT\4.6.2\BIN\QtGUI4.DLL nsis
copy C:\QT\4.6.2\BIN\qwt.DLL nsis
copy ..\src\Release\LaserBrothers.exe nsis
COPY LB.NSI NSIS
copy license.txt nsis
CD NSIS
"c:\program files\nsis\makensis.exe" lb.nsi
CD ..
move /Y NSIS\LaserBrothersSetup.exe ..\..\installers