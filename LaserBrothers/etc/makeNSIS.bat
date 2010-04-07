copy ..\src\Release\LaserBrothers.exe .
"c:\program files\nsis\makensis.exe" lb.nsi
del LaserBrothers.exe
copy /Y LaserBrothersSetup.exe ..\..\installers