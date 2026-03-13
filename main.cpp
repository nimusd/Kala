#include "kalamain.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("Kala");
    a.setApplicationName("Kala");
    KalaMain w;
    w.show();
    return a.exec();
}
