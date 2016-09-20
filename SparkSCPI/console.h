#ifndef CONSOLE_H
#define CONSOLE_H

#include <QMainWindow>
#include <QProcess>
#include <QLabel>

namespace Ui {
class console;
}

class console : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit console(QWidget *parent = 0);
    ~console();

    QString runCMD(QString command);
    QByteArray oval;
    QProcess process1, process2;
    QString ip;
    
        Ui::console *ui;
private slots:
    void on_scpi_command_returnPressed();
    void setSCPI(QString a);

private:
   // Ui::console *ui;
};

#endif // CONSOLE_H
