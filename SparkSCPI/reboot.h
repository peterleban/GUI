#ifndef REBOOT_H
#define REBOOT_H

#include <QMainWindow>
#include <QProcess>

namespace Ui {
class Reboot;
}

class Reboot : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Reboot(QWidget *parent = 0);
    ~Reboot();

    QString runCMD(QString command);
    QByteArray oval;
    QProcess process1, process2;
    QString ipaddress;
    int time;
    QTimer *timer3;

    Ui::Reboot *ui;


private slots:

    void on_rebootYes_clicked();
    void update_timetocomeback();
    
};

#endif // REBOOT_H
