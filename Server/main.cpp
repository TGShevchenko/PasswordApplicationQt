/****************************************************************************
 ** Using of Server class. Creating a Server instance and call Run method
 ** Author: Taras Shevchenko
 ****************************************************************************/

 #include <QApplication>
 #include <QtCore>
 #include <stdlib.h>
 #include "server.h"

 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);
     Server server;
     return server.Run();
 }
