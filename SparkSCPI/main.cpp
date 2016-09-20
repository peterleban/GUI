#include <QtGui/QApplication>
#include "spark.h"
#include <qwt-qt4/qwt_plot.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Spark w;
    w.show();
    
    return a.exec();
}
