#include "MainForm.h"

using namespace PasswordServer;

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}

