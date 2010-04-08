set FNAME1=gsl-1.8
set FNAME2=gsl-1.8

cd ..

mkdir thirdparty
cd thirdparty

wget -nc http://downloads.sourceforge.net/project/gnuwin32/gsl/1.8/%FNAME1%-lib.zip?use_mirror=jaist
wget -nc http://downloads.sourceforge.net/project/gnuwin32/gsl/1.8/%FNAME1%-src.zip?use_mirror=jaist

mkdir %FNAME2%
copy /Y %FNAME1%*.zip %FNAME2%

cd %FNAME2%
unzip %FNAME1%-lib.zip
unzip %FNAME1%-src.zip
cd ..\..\deploy