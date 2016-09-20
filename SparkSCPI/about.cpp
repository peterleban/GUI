#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    ui->toolBox->setCurrentIndex(0);
}

About::~About()
{
    delete ui;
}
