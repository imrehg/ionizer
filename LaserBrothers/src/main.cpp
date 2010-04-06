#include <qapplication.h>
#include "data_plot.h"

#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <iostream>


#include "CmdLineArgs.h"

using namespace std;



int main(int argc, char **argv)
{
    QApplication a(argc, argv);

	 CmdLineArgs cla(argc, argv);

	 string params_file = "laserbrothers.conf";

	 if(argc > 1)
		 params_file = argv[1];

	 cout << "params file = " << params_file << endl;
	 TxtParameters params(params_file);

    MainWindow mainWindow(&params);
#if QT_VERSION < 0x040000
    a.setMainWidget(&mainWindow);
#endif

    mainWindow.resize(1100,400);
    mainWindow.show();

    return a.exec(); 
}

