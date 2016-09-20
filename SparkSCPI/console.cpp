#include "console.h"
#include "ui_console.h"
#include <QProcess>

console::console(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::console)
{
    ui->setupUi(this);
    QStringList SCPIcommands;
    SCPIcommands << "ADC_SIZE " << "ADC_SIZE?" << "KX " << "KX?" << "KY " << "KY?" << "KQ " << "KQ?";
    SCPIcommands << "XOFF " << "XOFF?" << "YOFF " << "YOFF?" << "YOFF?" << "QOFF " << "QOFF?";
    SCPIcommands << "TRIG_DELAY " << "TRIG_DELAY?" << "TRIG_TM?" << "FILL_CNT?" << "FILL_CNT:RST" << "MAF_LENGTH " << "MAF_LENGTH?";
    SCPIcommands << "STT?" << "START" << "FREQ?" << "VER?";
    SCPIcommands << "ADC " << "TBT_IQ " << "TBT_IQ_DEC " << "TBT_AMPL " << "TBT_AMPL_DEC " << "TBT_QSUM " << "TBT_QSUM_DEC " << "TBT_XY " << "TBT_XY_DEC ";

    ui->comboSCPI->addItems(SCPIcommands);
    connect(ui->comboSCPI, SIGNAL(currentIndexChanged(QString)), SLOT(setSCPI(QString)));
}

console::~console()
{
    delete ui;
}

QString console::runCMD(QString command)
{
// Used to set/read parameters
// 2 processes have to be called in order to retrieve standard output

    process1.setStandardOutputProcess(&process2);
    process1.start("echo " + command);
    process2.start("nc " + ip + " 23 -q1");
    process1.waitForFinished(1000);
    process2.waitForFinished(1000);
    process2.waitForReadyRead(1000);
    oval = process2.readAllStandardOutput();
    return oval;
}

void console::on_scpi_command_returnPressed()
{
    runCMD(ui->scpi_command->text());
    ui->console_output->append("-> " + oval);
    ui->console_history->append(ui->scpi_command->text());
    ui->scpi_command->clear();
}

void console::setSCPI(QString a)
{
    ui->scpi_command->setText(a);
    ui->scpi_command->setFocus();
}
