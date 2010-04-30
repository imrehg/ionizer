set FNAME1=gsl-1.8
set FNAME2=gsl

..\bin\wget -nc http://downloads.sourceforge.net/project/gnuwin32/gsl/1.8/%FNAME1%-lib.zip?use_mirror=jaist
..\bin\wget -nc http://downloads.sourceforge.net/project/gnuwin32/gsl/1.8/%FNAME1%-doc.zip?use_mirror=jaist
..\bin\wget -nc http://downloads.sourceforge.net/project/gnuwin32/gsl/1.8/%FNAME1%-bin.zip?use_mirror=jaist

mkdir %FNAME2%
copy /Y %FNAME1%*.zip %FNAME2%

cd %FNAME2%
..\..\bin\unzip %FNAME1%-lib.zip
..\..\bin\unzip %FNAME1%-doc.zip
..\..\bin\unzip %FNAME1%-bin.zip
cd ..