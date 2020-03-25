/* main.cpp */


#include "APOViewer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    APOViewer w;
    w.show();

    return a.exec();
}
