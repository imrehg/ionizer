set FNAME1=fftw-3.2.1-dll.zip
set FNAME2=..\thirdparty\fftw
wget -nc ftp://ftp.fftw.org/pub/fftw/%FNAME1%
mkdir %FNAME2%
unzip %FNAME1% -d %FNAME2%

cd %FNAME2%
lib /machine:i386 /def:libfftw3-3.def
lib /machine:i386 /def:libfftw3f-3.def
lib /machine:i386 /def:libfftw3l-3.def
copy libfftw3-3.dll ..\..\aluminizer
cd ..\..\deploy
