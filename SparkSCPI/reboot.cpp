#include "reboot.h"
#include "ui_reboot.h"
#include "spark.h"
#include "ui_spark.h"
#include <QTimer>


Reboot::Reboot(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Reboot)
{
    ui->setupUi(this);
    timer3 = new QTimer(this);
    connect(timer3, SIGNAL(timeout()), this, SLOT(update_timetocomeback()));
}

Reboot::~Reboot()
{
    delete ui;
}

void Reboot::on_rebootYes_clicked()
{

 process1.start("plink -ssh -l root -pw Jungle " + ipaddress + " /sbin/reboot");
 process1.waitForFinished(1000);
 oval = process1.readAllStandardOutput();

 timer3->start(1000);
 time = 30;

 ui->label->setText("Reboot in progress...");
 ui->rebootNo->hide();
 ui->rebootYes->hide();

}

void Reboot::update_timetocomeback()
{
    ui->label->setText(QString::number(time));
    ui->label->setStyleSheet("color: rgb(255, 0, 0); background-color: rgb(255, 255, 255); font: 40pt 'Arial Black';");
    time -=1;

    if (time == 0) {
        timer3->stop();
        ui->label->setStyleSheet("color: rgb(255, 0, 0); background-color: rgb(255, 255, 255); font: 10pt 'Arial';");
        ui->label->setText("Close this window now.");
    }
}
