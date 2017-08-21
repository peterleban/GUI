#include "help.h"
#include "ui_help.h"

Help::Help(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Help)
{
    ui->setupUi(this);
    ui->toolBox->setCurrentIndex(0);
}

Help::~Help()
{
    delete ui;
}
