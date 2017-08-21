#include "spark.h"
#include "ui_spark.h"
#include "ui_about.h"
#include "ui_reboot.h"
#include "ui_console.h"
#include "ui_help.h"
#include "console.h"
#include "reboot.h"
#include "help.h"
#include <QProcess>
#include <qwt-qt4/qwt_plot.h>
#include <qwt-qt4/qwt_plot_curve.h>
#include <qwt-qt4/qwt_plot_grid.h>
#include <qwt-qt4/qwt_scale_engine.h>
#include <qwt-qt4/qwt_plot_magnifier.h>
#include <qwt-qt4/qwt_plot_zoomer.h>
#include <qwt-qt4/qwt_plot_picker.h>
#include <qwt-qt4/qwt_picker_machine.h>
#include <qwt-qt4/qwt_picker.h>
#include <QTimer>
#include <QDateTime>
#include <kiss_fft130/kiss_fft.h>
#include <math.h>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QTabWidget>

// This GUI has been developed gradually by adding functionalities step by step
// as requirements came in. The code is not written by a software engineer so
// don't judge the non-effective implementation. It could be done better!
//
// What calls for improvement is:
// (a) read binary data instead of ASCII ... that will improve data transfer speed
// (b) direct TCP calls to avoid the use of Netcat
// (c) implement zoom-in functionality for plots


Spark::Spark(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Spark)
{
    ui->setupUi(this);

    // timer is used for issuing a periodic sw trigger
    timer = new QTimer(this);

    // exttimer is used for periodic readout
    exttimer = new QTimer(this);

    // Here we link both timers with their corresponding slots
    connect(timer, SIGNAL(timeout()), this, SLOT(AutoTrigger()));
    connect(exttimer, SIGNAL(timeout()), this, SLOT(ExtTrigger()));

    // Display current time in the main screen
    current_time = new QDateTime();
    local_time = new QTimer(this);
    connect(local_time, SIGNAL(timeout()), this, SLOT(showLocalTime()));
    local_time->start(1000);

    // Show Parameters widget on startup, calculate MC
    ui->tabWidget->setCurrentIndex(0);
    on_RF_textEdited("abc");

    setFonts();

    // Set window title with IP address
    ip_address = ui->IP->text();
    setWindowTitle("Libera Spark / " + ip_address);
}

QString Spark::runCMD(QString command)
{
// Used to set/read parameters
// 2 processes have to be called in order to retrieve standard output

    process1.setStandardOutputProcess(&process2);
    process1.start("echo " + command);
    process2.start("nc " + ui->IP->text() + " 23 -q1");
    process1.waitForFinished(5000);
    process2.waitForFinished(5000);
    process2.waitForReadyRead(6000);
    oval = process2.readAllStandardOutput();
    return oval;
}

QString Spark::runACQ(QString command)
{
// Used to trigger an acquisition and read output
// 2 processes have to be called in order to retrieve standard output

    process1.setStandardOutputProcess(&process2);
    process1.start("echo " + command);
    process2.start("nc " + ui->IP->text() + " 23 -q1");
    process1.waitForFinished(5000);
    process2.waitForFinished(5000);
    process2.waitForReadyRead(6000);
    oval = process2.readAllStandardOutput();
    return oval;
}

void Spark::setFonts()
{
  QFont font_plot("Arial", 9);
  QFont font_axis("Arial", 9, QFont::Bold);
  QFont font_alarm("Arial", 9, QFont::Bold);

  tbt_axis.setFont(font_axis);
  tbt_axis_y.setFont(font_axis);

  tbt_axis_iq.setFont(font_axis);
  tbt_axis_iq_y.setFont(font_axis);

  tbt_axis_sum.setFont(font_axis);
  tbt_axis_sum_y.setFont(font_axis);

  fft_axis.setFont(font_axis);
  fft_axis_y.setFont(font_axis);

  adc_axis.setFont(font_axis);
  adc_axis_y.setFont(font_axis);
}

void Spark::startGUI()
{
    on_ParamRead_clicked();
    on_adcsizeRead_clicked();
    on_FreqRead_clicked();
    on_procParamRead_clicked();
    on_ReadTrigDelay_clicked();
    on_ReadSTT_clicked();
}

Spark::~Spark()
{
    delete ui;
}

void Spark::showLocalTime()
{
    ui->local_time->setText(current_time->currentDateTime().toString());
}

void Spark::on_RF_textEdited(const QString &arg1)
{
    mc_frq = ui->RF->text().toFloat() / ui->HN->text().toFloat();
    ui->MC_FREQ->setText(QString::number(mc_frq, 'g', 7));
    ui->lMC_FREQ->setText("MC freq. [Hz]: "+ QString::number(mc_frq, 'g', 7));
    arg1.size(); // We don't actually need it as we read the value from the text field
}

void Spark::on_HN_textEdited(const QString &arg1)
{
    mc_frq = ui->RF->text().toFloat() / ui->HN->text().toFloat();
    ui->MC_FREQ->setText(QString::number(mc_frq, 'g', 7));
    ui->lMC_FREQ->setText("MC freq. [Hz]: "+ QString::number(mc_frq, 'g', 7));
    arg1.size(); // We don't actually need it as we read the value from the text field
}

void Spark::on_ParamSet_clicked()
{
    runCMD("KX "+ ui->KX->text());
    runCMD("KX?");
    ui->CONSOLE->append("KX set to: "+ QString("%1").arg(QString(oval).toDouble()));

    runCMD("KY "+ ui->KY->text());
    runCMD("KY?");
    ui->CONSOLE->append("KY set to: "+ QString("%1").arg(QString(oval).toDouble()));

    runCMD("KQ "+ ui->KQ->text());
    runCMD("KQ?");
    ui->CONSOLE->append("KQ set to: "+ QString("%1").arg(QString(oval).toDouble()));

    runCMD("XOFF "+ ui->OFFX->text());
    runCMD("XOFF?");
    ui->CONSOLE->append("XOFF set to: "+ QString("%1").arg(QString(oval).toDouble()));

    runCMD("YOFF "+ ui->OFFY->text());
    runCMD("YOFF?");
    ui->CONSOLE->append("YOFF set to: "+ QString("%1").arg(QString(oval).toDouble()));

    runCMD("QOFF "+ ui->OFFQ->text());
    runCMD("QOFF?");
    ui->CONSOLE->append("QOFF set to: "+ QString("%1").arg(QString(oval).toDouble()));
}

void Spark::on_ParamRead_clicked()
{
    runCMD("KX?");
    ui->KX->setValue(QString(oval).toInt(0,10));

    runCMD("KY?");
    ui->KY->setValue(QString(oval).toInt(0,10));

    runCMD("KQ?");
    ui->KQ->setValue(QString(oval).toInt(0,10));

    runCMD("XOFF?");
    ui->OFFX->setValue(QString(oval).toInt(0,10));

    runCMD("YOFF?");
    ui->OFFY->setValue(QString(oval).toInt(0,10));

    runCMD("QOFF?");
    ui->OFFQ->setValue(QString(oval).toInt(0,10));
}

void Spark::on_FreqRead_clicked()
{
    runCMD("FREQ?");
    frequency = QString(oval).toDouble();
    ui->FREQ->setText(QString::number(frequency, 'g', 9));
    adc_frq = frequency;
}

void Spark::on_FreqSet_clicked()
{
// We can change the ADC sampling frequency ONLY when in NON_INITIALIZED state.
// We check for the state and don't do anything if "RUNNING".
// Otherwise, we set the FREQ

    runCMD("STT?");
    if (QString(oval.at(0)) == "R") {
        ui->CONSOLE->append("Device is RUNNING. Reboot and set the ADC sampling frequency before starting the device.");
    }
    else {
        runCMD("FREQ " + ui->FREQ->text());
        if (QString(oval.at(0)) != "E") {
            ui->CONSOLE->append("ADC sampling frequency set to " + ui->FREQ->text() + "Hz. Start device now.");
            adc_frq = frequency;
        }
        else {
            ui->CONSOLE->append("Not a valid ADC sampling frequency.");
        }
    }
}

void Spark::on_procParamSet_clicked()
{
    // Here we set the MAF_LENGTH parameter

    runCMD("MAF_LENGTH "+ ui->MAF_LENGTH->text());
    if (QString(oval.at(0)) == "E")
        ui->CONSOLE->append("Invalid MAF_LENGTH");
    else
        ui->CONSOLE->append("MAF_LENGTH set to " + QString("%1").arg(ui->MAF_LENGTH->text().toLongLong()));
}

void Spark::on_procParamRead_clicked()
{
    // Here we read the MAF_LENGTH parameter

    runCMD("MAF_LENGTH?");
    ui->MAF_LENGTH->setText(QString(oval));
}

void Spark::on_adcsizeSet_clicked()
{
    // Here we set the ADC_SIZE parameter and read the FREQ.
    // From both, we calculate the time [ms] between the trigger and increase of the fill counter.

    runCMD("FREQ?");
        float a = QString(oval).toFloat();
    runCMD("ADC_SIZE "+ ui->ADC_SIZE->text());
        if (QString(oval.at(0)) == "E")
            ui->CONSOLE->append("Invalid ADC_SIZE");
        else {
            ui->CONSOLE->append("ADC_SIZE set to " + QString("%1").arg(ui->ADC_SIZE->text().toLongLong()));
            ui->l_ADCSIZE_calculated->setText(QString("FILL_CNT will increase " + QString::number((ui->ADC_SIZE->text().toFloat()*1000 / a)) + " ms after Trigger"));
        }
}

void Spark::on_adcsizeRead_clicked()
{
    // Here we read the ADC_SIZE parameter and FREQ.
    // From both, we calculate the time [ms] between the trigger and increase of the fill counter.

    runCMD("FREQ?");
        float a = QString(oval).toFloat();
    runCMD("ADC_SIZE?");
        ui->l_ADCSIZE_calculated->setText(QString("FILL_CNT will increase " + QString::number((QString(oval).toFloat()*1000 / a)) + " ms after Trigger"));
        ui->ADC_SIZE->setText(QString(oval));
}


void Spark::on_ReadADC_clicked()
{
    // This will request <N> samples from ADC buffer.
    // In the code, the data is parsed (4 columns) to channels A, B, C and D
    // We'll check the peak ADC count (MaxADC) and report saturation, if detected.
    // In the end, we'll calculate the "single-pass" position.

    runACQ("ADC " + QString::number(ui->ADC_SMPL->value()));

    int a = ui->ADC_SMPL->value();

    QPolygonF points1, points2, points3, points4;

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    grid->enableX(true);
    grid->enableXMin(true);
    grid->enableY(true);
    grid->enableYMin(true);
    grid->attach(ui->PlotADC);

    int b;
    int maxadc = 0;
    int i = 0;

    // For single Pass calculation

    double chA, chB, chC, chD;
    chA = 0;
    chB = 0;
    chC = 0;
    chD = 0;
    int thr_start = 0;

    if (ui->showSP->isChecked() == true) {
        sp_threshold = ui->adcsp_threshold->text().toInt();
        sp_window = ui->adcsp_window->text().toInt();

        while (abs(thr_start) < abs(sp_threshold))
        {
            thr_start = QString(oval.mid(i*12,12)).toInt(0,10);
            i++;
            if (i == (a-1)) {
                ui->CONSOLE->append("Threshold not detected. Single pass calculation not possible.");
                break;
            }
        }

        // Divide the threshold marker by 4 since the check runs on 4 channels
        thr_start = i/4;

        for (int i = thr_start; i < (thr_start+sp_window); i++)
        {
            b = QString(oval.mid(i*48,12)).toInt(0,10);
            points1 << QPointF(i,b);
            chA += b*b;

            b = QString(oval.mid(i*48+12,12)).toInt(0,10);
            points1 << QPointF(i,b);
            chB += b*b;

            b = QString(oval.mid(i*48+24,12)).toInt(0,10);
            points3 << QPointF(i,b);
            chC += b*b;

            b = QString(oval.mid(i*48+36,12)).toInt(0,10);
            points4 << QPointF(i,b);
            chD += b*b;
         }

        adcsp_a.setData(points1);
        adcsp_b.setData(points2);
        adcsp_c.setData(points3);
        adcsp_d.setData(points4);

        adcsp_a.setPen(QPen(Qt::blue, 3,Qt::SolidLine) );
        adcsp_b.setPen(QPen(Qt::red, 3,Qt::SolidLine) );
        adcsp_c.setPen(QPen(Qt::green, 3,Qt::SolidLine) );
        adcsp_d.setPen(QPen(Qt::magenta, 3,Qt::SolidLine) );

        points1.clear();
        points2.clear();
        points3.clear();
        points4.clear();
    }

    // Now we'll attach ADC data to curves
    for (int i = 0; i<a; i++)
    {
        b = QString(oval.mid(i*48,12)).toInt(0,10);
        points1 << QPointF(i,b);
        if (b > maxadc) maxadc = b;

        b = QString(oval.mid(i*48+12,12)).toInt(0,10);
        points2 << QPointF(i,b);
        if (b > maxadc) maxadc = b;

        b = QString(oval.mid(i*48+24,12)).toInt(0,10);
        points3 << QPointF(i,b);
        if (b > maxadc) maxadc = b;

        b = QString(oval.mid(i*48+36,12)).toInt(0,10);
        points4 << QPointF(i,b);
        if (b > maxadc) maxadc = b;
     }

    curve1_adc_a.setData(points1);
    curve2_adc_b.setData(points2);
    curve3_adc_c.setData(points3);
    curve4_adc_d.setData(points4);

    curve1_adc_a.setPen(QPen(Qt::blue, 1,Qt::SolidLine) );
    curve2_adc_b.setPen(QPen(Qt::red, 1,Qt::SolidLine) );
    curve3_adc_c.setPen(QPen(Qt::green, 1,Qt::SolidLine) );
    curve4_adc_d.setPen(QPen(Qt::magenta, 1,Qt::SolidLine) );

    if (ui->adc_a->isChecked() == true) {
        curve1_adc_a.attach(ui->PlotADC);
        adcsp_a.attach(ui->PlotADC);
    }

    if (ui->adc_b->isChecked() == true) {
        curve2_adc_b.attach(ui->PlotADC);
        adcsp_b.attach(ui->PlotADC);
    }

    if (ui->adc_c->isChecked() == true) {
        curve3_adc_c.attach(ui->PlotADC);
        adcsp_c.attach(ui->PlotADC);
    }

    if (ui->adc_d->isChecked() == true) {
        curve4_adc_d.attach(ui->PlotADC);
        adcsp_d.attach(ui->PlotADC);
    }

    if (ui->showSP->isChecked() == false) {
        adcsp_a.detach();
        adcsp_b.detach();
        adcsp_c.detach();
        adcsp_d.detach();
    }

    adc_axis.setText("ADC samples");
    adc_axis_y.setText("ADC counts");

    ui->PlotADC->setAxisTitle(0, adc_axis_y);
    ui->PlotADC->setAxisTitle(2, adc_axis);
    ui->PlotADC->setAxisFont(0, font_plot);
    ui->PlotADC->setAxisFont(2, font_plot);


    ui->PlotADC->replot();

    QwtPlotZoomer* zoomer = new QwtPlotZoomer( ui->PlotADC->canvas(),true);
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
     Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
     Qt::RightButton );
    //ui->PlotADC->setAutoReplot(true);



    ui->MAXADC->setStyleSheet("");
    ui->MAXADC->setText("MaxADC: " + QString::number(maxadc) + " / " + QString::number(100*maxadc/8192, 'f', 0) + " % FS");
    if (maxadc > 8100) {
        ui->MAXADC->setText("SATURATED!");
        ui->MAXADC->setStyleSheet("color: rgb(255, 0, 0);background-color: rgb(255, 255, 0);");
    }

    ui->INPUTdBm->setText("Input at ~" + QString::number( 10 + 20*log10(maxadc * 0.003162275 / ui->OdBmCNT->text().toInt()),'f',1) + " dBm");

    // Single Pass calculation

    if (chA == 0 && chB == 0 && chC == 0 && chD == 0) {
        ui->ladcsp_x->setText("No data");
        ui->ladcsp_y->setText("No data");
    }
    else if (ui->KX->text() == "1" && ui->KY->text() == "1" && ui->KQ->text() == "1") {
        ui->ladcsp_threshold->setText("Check your KX,KY parameters!");
        ui->ladcsp_x->setText("X = ");
        ui->ladcsp_y->setText("Y = ");
    }
    else {
    ui->ladcsp_x->setText("X = " + QString::number( (ui->KX->text().toInt() * ( sqrt(chA) + sqrt(chD) - sqrt(chB) - sqrt(chC) ) / ( sqrt(chA) + sqrt(chB) + sqrt(chC) + sqrt(chD) )), 'f', 2  ) + " um");
    ui->ladcsp_y->setText("Y = " + QString::number( (ui->KY->text().toInt() * ( sqrt(chA) + sqrt(chB) - sqrt(chC) - sqrt(chD) ) / ( sqrt(chA) + sqrt(chB) + sqrt(chC) + sqrt(chD) )), 'f', 2  ) + " um");
    ui->ladcsp_threshold->setText("Threshold at sample #" + QString::number(thr_start));
    }
}

void Spark::on_adc_a_clicked()
{
    // This will show/hide the channel A of ADC data

    if (ui->adc_a->isChecked() == true) {
        curve1_adc_a.attach(ui->PlotADC);
        adcsp_a.attach(ui->PlotADC);
    }
    else {
        curve1_adc_a.detach();
        adcsp_a.detach();
    }
    ui->PlotADC->replot();
}

void Spark::on_adc_b_clicked()
{
    // This will show/hide the channel B of ADC data

    if (ui->adc_b->isChecked() == true) {
        curve2_adc_b.attach(ui->PlotADC);
        adcsp_b.attach(ui->PlotADC);
    }
    else {
        curve2_adc_b.detach();
        adcsp_b.detach();
    }
    ui->PlotADC->replot();
}

void Spark::on_adc_c_clicked()
{
    // This will show/hide the channel C of ADC data

    if (ui->adc_c->isChecked() == true) {
        curve3_adc_c.attach(ui->PlotADC);
        adcsp_c.attach(ui->PlotADC);
    }
    else {
        curve3_adc_c.detach();
        adcsp_c.detach();
    }
    ui->PlotADC->replot();
}

void Spark::on_adc_d_clicked()
{
    // This will show/hide the channel D of ADC data

    if (ui->adc_d->isChecked() == true) {
        curve4_adc_d.attach(ui->PlotADC);
        adcsp_d.attach(ui->PlotADC);
    }
    else {
        curve4_adc_d.detach();
        adcsp_d.detach();
    }

    ui->PlotADC->replot();
}

void Spark::on_ReadIQ_clicked()
{
    // This will request <N> samples from TBT IQ buffer.
    // In the code, the data is parsed (8 columns) to channels IA, QA, IB, QB, IC, QC, ID, QD
    // We check the radio button if decimated data is requested.

    if (ui->tbtiq_normal->isChecked() == true) {
        runACQ("TBT_IQ " + QString::number(ui->IQ_SMPL->value()));
        tbt_axis_iq.setText("Turn-by-turn samples");
        tbt_axis_iq_y.setText("Amplitudes [a.u.]");
    }
    else {
        runACQ("TBT_IQ_DEC " + QString::number(ui->IQ_SMPL->value()));
        tbt_axis_iq.setText("Decimated turn-by-turn samples");
        tbt_axis_iq_y.setText("Amplitudes [a.u.]");
    }

    double a = ui->IQ_SMPL->value();

    QPolygonF points1, points2, points3, points4, points5, points6, points7, points8;

    QwtPlotGrid *gridIQ = new QwtPlotGrid();
    gridIQ->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    gridIQ->enableX(true);
    gridIQ->enableXMin(true);
    gridIQ->enableY(true);
    gridIQ->enableYMin(false);
    gridIQ->attach(ui->PlotIQ);

    int b;
    for (double i = 0; i<a; i++)
    {
        b = QString(oval.mid(i*56,7)).toInt(0,10);
        points1 << QPointF(i,b);
        b = QString(oval.mid(i*56+7,7)).toInt(0,10);
        points2 << QPointF(i,b);
        b = QString(oval.mid(i*56+14,7)).toInt(0,10);
        points3 << QPointF(i,b);
        b = QString(oval.mid(i*56+21,7)).toInt(0,10);
        points4 << QPointF(i,b);
        b = QString(oval.mid(i*56+28,7)).toInt(0,10);
        points5 << QPointF(i,b);
        b = QString(oval.mid(i*56+35,7)).toInt(0,10);
        points6 << QPointF(i,b);
        b = QString(oval.mid(i*56+42,7)).toInt(0,10);
        points7 << QPointF(i,b);
        b = QString(oval.mid(i*56+49,7)).toInt(0,10);
        points8 << QPointF(i,b);
    }

    curveIA.setData(points1);
    curveQA.setData(points2);
    curveIB.setData(points3);
    curveQB.setData(points4);
    curveIC.setData(points5);
    curveQC.setData(points6);
    curveID.setData(points7);
    curveQD.setData(points8);

    curveIA.setPen(QPen(Qt::blue, 1,Qt::SolidLine) );
    curveQA.setPen(QPen(Qt::blue, 2,Qt::DashLine) );
    curveIB.setPen(QPen(Qt::red, 1,Qt::SolidLine) );
    curveQB.setPen(QPen(Qt::red, 2,Qt::DashLine) );
    curveIC.setPen(QPen(Qt::green, 1,Qt::SolidLine) );
    curveQC.setPen(QPen(Qt::green, 2,Qt::DashLine) );
    curveID.setPen(QPen(Qt::magenta, 1,Qt::SolidLine) );
    curveQD.setPen(QPen(Qt::magenta, 2,Qt::DashLine) );

    if (ui->ia->isChecked() == true)
        curveIA.attach(ui->PlotIQ);

    if (ui->qa->isChecked() == true)
        curveQA.attach(ui->PlotIQ);

    if (ui->ib->isChecked() == true)
        curveIB.attach(ui->PlotIQ);

    if (ui->qb->isChecked() == true)
        curveQB.attach(ui->PlotIQ);

    if (ui->ic->isChecked() == true)
        curveIC.attach(ui->PlotIQ);

    if (ui->qc->isChecked() == true)
        curveQC.attach(ui->PlotIQ);

    if (ui->id->isChecked() == true)
        curveID.attach(ui->PlotIQ);

    if (ui->qd->isChecked() == true)
        curveQD.attach(ui->PlotIQ);

    ui->PlotIQ->setAxisTitle(0, tbt_axis_iq_y);
    ui->PlotIQ->setAxisTitle(2, tbt_axis_iq);
    ui->PlotIQ->setAxisFont(0,font_plot);
    ui->PlotIQ->setAxisFont(2,font_plot);

    QwtPlotZoomer* zoomer = new QwtPlotZoomer( ui->PlotIQ->canvas(),true);
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
     Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
     Qt::RightButton );
    ui->PlotIQ->setAutoReplot(true);

    ui->PlotIQ->replot();
}

void Spark::on_ia_clicked()
{
    // This will show/hide the channel IA of TBT data

    if (ui->ia->isChecked() == true)
        curveIA.attach(ui->PlotIQ);
    else
        curveIA.detach();

    ui->PlotIQ->replot();
}

void Spark::on_qa_clicked()
{
    // This will show/hide the channel QA of TBT data

    if (ui->qa->isChecked() == true)
        curveQA.attach(ui->PlotIQ);
    else
        curveQA.detach();

    ui->PlotIQ->replot();
}

void Spark::on_ib_clicked()
{
    // This will show/hide the channel IB of TBT data

    if (ui->ib->isChecked() == true)
        curveIB.attach(ui->PlotIQ);
    else
        curveIB.detach();

    ui->PlotIQ->replot();
}

void Spark::on_qb_clicked()
{
    // This will show/hide the channel QB of TBT data

    if (ui->qb->isChecked() == true)
        curveQB.attach(ui->PlotIQ);
    else
        curveQB.detach();

    ui->PlotIQ->replot();
}

void Spark::on_ic_clicked()
{
    // This will show/hide the channel IC of TBT data

    if (ui->ic->isChecked() == true)
        curveIC.attach(ui->PlotIQ);
    else
        curveIC.detach();

    ui->PlotIQ->replot();
}

void Spark::on_qc_clicked()
{
    // This will show/hide the channel QC of TBT data

    if (ui->qc->isChecked() == true)
        curveQC.attach(ui->PlotIQ);
    else
        curveQC.detach();

    ui->PlotIQ->replot();
}

void Spark::on_id_clicked()
{
    // This will show/hide the channel ID of TBT data

    if (ui->id->isChecked() == true)
        curveID.attach(ui->PlotIQ);
    else
        curveID.detach();

    ui->PlotIQ->replot();
}

void Spark::on_qd_clicked()
{
    // This will show/hide the channel QD of TBT data

    if (ui->qd->isChecked() == true)
        curveQD.attach(ui->PlotIQ);
    else
        curveQD.detach();

    ui->PlotIQ->replot();
}

void Spark::on_ReadTBTAMPL_clicked()
{
    // This will request <N> samples from TBT AMPL buffer.
    // In the code, the data is parsed (4 columns) to channels VA, VB, VC, VD
    // We check the radio button if decimated data is requested.

    if (ui->tbtampl_normal->isChecked() == true) {
        runACQ("TBT_AMPL " + QString::number(ui->TBTAMPL_SMPL->value()));
        tbt_axis_ampl.setText("Turn-by-turn samples");
        tbt_axis_ampl_y.setText("Amplitudes [a.u.]");
    }
    else {
        runACQ("TBT_AMPL_DEC " + QString::number(ui->TBTAMPL_SMPL->value()));
        tbt_axis_ampl.setText("Decimated turn-by-turn samples");
        tbt_axis_ampl_y.setText("Amplitudes [a.u.]");
    }

    int a = ui->TBTAMPL_SMPL->value();

    QPolygonF points1, points2, points3, points4;

    QwtPlotGrid *gridAMPL = new QwtPlotGrid();
    gridAMPL->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    gridAMPL->enableX(true);
    gridAMPL->enableXMin(true);
    gridAMPL->enableY(true);
    gridAMPL->enableYMin(false);
    gridAMPL->attach(ui->PlotTBTAMPL);

    int b;
    for (int i = 0; i<a; i++)
    {
        b = QString(oval.mid(i*28,7)).toInt(0,10);
        points1 << QPointF(i,b);
        b = QString(oval.mid(i*28+7,7)).toInt(0,10);
        points2 << QPointF(i,b);
        b = QString(oval.mid(i*28+14,7)).toInt(0,10);
        points3 << QPointF(i,b);
        b = QString(oval.mid(i*28+21,7)).toInt(0,10);
        points4 << QPointF(i,b);
    }

    curve1_tbt_a.setData(points1);
    curve2_tbt_b.setData(points2);
    curve3_tbt_c.setData(points3);
    curve4_tbt_d.setData(points4);

    curve1_tbt_a.setPen(QPen(Qt::blue, 1,Qt::SolidLine) );
    curve2_tbt_b.setPen(QPen(Qt::red, 1,Qt::SolidLine) );
    curve3_tbt_c.setPen(QPen(Qt::green, 1,Qt::SolidLine) );
    curve4_tbt_d.setPen(QPen(Qt::magenta, 1,Qt::SolidLine) );

    if (ui->tbt_ampl_a->isChecked() == true)
        curve1_tbt_a.attach(ui->PlotTBTAMPL);

    if (ui->tbt_ampl_b->isChecked() == true)
        curve2_tbt_b.attach(ui->PlotTBTAMPL);

    if (ui->tbt_ampl_c->isChecked() == true)
        curve3_tbt_c.attach(ui->PlotTBTAMPL);

    if (ui->tbt_ampl_d->isChecked() == true)
        curve4_tbt_d.attach(ui->PlotTBTAMPL);

    ui->PlotTBTAMPL->setAxisTitle(0, tbt_axis_ampl_y);
    ui->PlotTBTAMPL->setAxisTitle(2, tbt_axis_ampl);
    ui->PlotTBTAMPL->setAxisFont(0,font_plot);
    ui->PlotTBTAMPL->setAxisFont(2,font_plot);

    QwtPlotZoomer* zoomer = new QwtPlotZoomer( ui->PlotTBTAMPL->canvas(),true);
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
     Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
     Qt::RightButton );
    ui->PlotTBTAMPL->setAutoReplot(true);

    ui->PlotTBTAMPL->replot();
}

void Spark::on_tbt_ampl_a_clicked()
{
    // This will show/hide the channel VA of TBT data

    if (ui->tbt_ampl_a->isChecked() == true)
        curve1_tbt_a.attach(ui->PlotTBTAMPL);
    else
        curve1_tbt_a.detach();

    ui->PlotTBTAMPL->replot();
}

void Spark::on_tbt_ampl_b_clicked()
{
    // This will show/hide the channel VB of TBT data

    if (ui->tbt_ampl_b->isChecked() == true)
        curve2_tbt_b.attach(ui->PlotTBTAMPL);
    else
        curve2_tbt_b.detach();

    ui->PlotTBTAMPL->replot();
}

void Spark::on_tbt_ampl_c_clicked()
{
    // This will show/hide the channel VC of TBT data

    if (ui->tbt_ampl_c->isChecked() == true)
        curve3_tbt_c.attach(ui->PlotTBTAMPL);
    else
        curve3_tbt_c.detach();

    ui->PlotTBTAMPL->replot();
}

void Spark::on_tbt_ampl_d_clicked()
{
    // This will show/hide the channel VD of TBT data

    if (ui->tbt_ampl_d->isChecked() == true)
        curve4_tbt_d.attach(ui->PlotTBTAMPL);
    else
        curve4_tbt_d.detach();

    ui->PlotTBTAMPL->replot();
}

void Spark::on_ReadTBTSUM_clicked()
{
    // This will request <N> samples from TBT SUM buffer.
    // In the code, the data is parsed (2 columns) to channels SUM, Q
    // We check the radio button if decimated data is requested.

    if (ui->tbtsum_normal->isChecked() == true) {
        runACQ("TBT_QSUM " + QString::number(ui->TBTSUM_SMPL->value()));
        tbt_axis_sum.setText("Turn-by-turn samples");
        tbt_axis_sum_y.setText("SUM [A.U.],\n Q [um]");
    }
    else {
        runACQ("TBT_QSUM_DEC " + QString::number(ui->TBTSUM_SMPL->value()));
        tbt_axis_sum.setText("Decimated turn-by-turn samples");
        tbt_axis_sum_y.setText("SUM [A.U.],\n Q [um]");
    }

    int a = ui->TBTSUM_SMPL->value();

    QPolygonF points1, points2;

    QwtPlotGrid *gridSUM = new QwtPlotGrid();
    gridSUM->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    gridSUM->enableX(true);
    gridSUM->enableXMin(true);
    gridSUM->enableY(true);
    gridSUM->enableYMin(false);
    gridSUM->attach(ui->PlotTBTSUM);

    int b;
    for (int i = 0; i<a; i++)
    {
        b = QString(oval.mid(i*14+7,7)).toInt(0,10);
        points1 << QPointF(i,b);
        b = QString(oval.mid(i*14,7)).toInt(0,10);
        points2 << QPointF(i,b);
    }

    curve1_tbtqsum.setData(points1);
    curve2_tbtqsum.setData(points2);

    curve1_tbtqsum.setPen(QPen(Qt::blue, 1,Qt::SolidLine) );
    curve2_tbtqsum.setPen(QPen(Qt::red, 1,Qt::SolidLine) );

    if (ui->tbt_sum->isChecked() == true)
        curve1_tbtqsum.attach(ui->PlotTBTSUM);

    if (ui->tbt_q->isChecked() == true)
        curve2_tbtqsum.attach(ui->PlotTBTSUM);

    ui->PlotTBTSUM->setAxisTitle(0, tbt_axis_sum_y);
    ui->PlotTBTSUM->setAxisTitle(2, tbt_axis_sum);
    ui->PlotTBTSUM->setAxisFont(0,font_plot);
    ui->PlotTBTSUM->setAxisFont(2,font_plot);

    QwtPlotZoomer* zoomer = new QwtPlotZoomer( ui->PlotTBTSUM->canvas(),true);
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
     Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
     Qt::RightButton );
    ui->PlotTBTSUM->setAutoReplot(true);

    ui->PlotTBTSUM->replot();
}

void Spark::on_tbt_sum_clicked()
{
    // This will show/hide the channel SUM of TBT data

    if (ui->tbt_sum->isChecked() == true)
        curve1_tbtqsum.attach(ui->PlotTBTSUM);
    else
        curve1_tbtqsum.detach();

    ui->PlotTBTSUM->replot();
}

void Spark::on_tbt_q_clicked()
{
    // This will show/hide the channel Q of TBT data

    if (ui->tbt_q->isChecked() == true)
        curve2_tbtqsum.attach(ui->PlotTBTSUM);
    else
        curve2_tbtqsum.detach();

    ui->PlotTBTSUM->replot();
}

void Spark::on_ReadTBTXY_clicked()
{
    // This will request <N> samples from TBT XY buffer.
    // In the code, the data is parsed (2 columns) to channels X, Y
    // We check the radio button if decimated data is requested.

    if (ui->tbtxy_normal->isChecked() == true) {
        runACQ("TBT_XY " + QString::number(ui->TBTXY_SMPL->value()));
        tbt_axis.setText("Turn-by-turn samples");
        tbt_axis_y.setText("Position [um]");
    }
    else {
        runACQ("TBT_XY_DEC " + QString::number(ui->TBTXY_SMPL->value()));
        tbt_axis.setText("Decimated turn-by-turn samples");
        tbt_axis_y.setText("Position [um]");
    }

    int a = ui->TBTXY_SMPL->value();

    QPolygonF points1, points2;

    QwtPlotGrid *gridXY = new QwtPlotGrid();
    gridXY->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    gridXY->enableX(true);
    gridXY->enableXMin(true);
    gridXY->enableY(true);
    gridXY->enableYMin(false);
    gridXY->attach(ui->PlotTBTXY);

    int b;
    float x_mean = 0;
    float y_mean = 0;
    float x_rms = 0;
    float y_rms = 0;
    float x[a], y[a];

    for (int i = 0; i<a; i++)
    {
        b = QString(oval.mid(i*14,7)).toInt(0,10);
        points1 << QPointF(i,b);
        x_mean += b;
        x[i] = b;

        b = QString(oval.mid(i*14+7,7)).toInt(0,10);
        points2 << QPointF(i,b);
        y_mean += b;
        y[i] = b;
    }

    curve1_tbt_x.setData(points1);
    curve2_tbt_y.setData(points2);

    curve1_tbt_x.setPen(QPen(Qt::blue, 1,Qt::SolidLine) );
    curve2_tbt_y.setPen(QPen(Qt::red, 1,Qt::SolidLine) );

    if (ui->tbt_x->isChecked() == true)
        curve1_tbt_x.attach(ui->PlotTBTXY);

    if (ui->tbt_y->isChecked() == true)
        curve2_tbt_y.attach(ui->PlotTBTXY);

    ui->PlotTBTXY->setAxisTitle(0, tbt_axis_y);
    ui->PlotTBTXY->setAxisTitle(2, tbt_axis);
    ui->PlotTBTXY->setAxisFont(0,font_plot);
    ui->PlotTBTXY->setAxisFont(2,font_plot);

    QwtPlotZoomer* zoomer = new QwtPlotZoomer( ui->PlotTBTXY->canvas(),true);
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
     Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
     Qt::RightButton );
    ui->PlotTBTXY->setAutoReplot(true);

    ui->PlotTBTXY->replot();

    // Calculate mean and standard deviation
    x_mean /= a;
    y_mean /= a;

    for (int i = 0; i<a; i++)
    {
        x_rms += (x[i]-x_mean)*(x[i]-x_mean) ;
        y_rms += (y[i]-y_mean)*(y[i]-y_mean) ;
    }

    x_rms /= (a-1);
    y_rms /= (a-1);

    ui->xmean->setText("X mean: " + QString::number(x_mean, 'f', 2) + " um");
    ui->xrms->setText("X RMS: " + QString::number(sqrt(x_rms), 'f', 2) + " um");
    ui->ymean->setText("Y mean: " + QString::number(y_mean, 'f', 2) + " um");
    ui->yrms->setText("Y RMS: " + QString::number(sqrt(y_rms), 'f', 2) + " um");
}

void Spark::on_tbt_x_clicked()
{
    // This will show/hide the channel X of TBT data

    if (ui->tbt_x->isChecked() == true)
        curve1_tbt_x.attach(ui->PlotTBTXY);
    else
        curve1_tbt_x.detach();

    ui->PlotTBTXY->replot();
}

void Spark::on_tbt_y_clicked()
{
    // This will show/hide the channel Y of TBT data

    if (ui->tbt_y->isChecked() == true)
        curve2_tbt_y.attach(ui->PlotTBTXY);
    else
        curve2_tbt_y.detach();

    ui->PlotTBTXY->replot();
}

void Spark::on_SetTrigDelay_clicked()
{
    // Will set TRIG_DELAY parameter and calculate its time [us] equivalent

    runCMD("TRIG_DELAY "+ ui->TRIG_DELAY->text());
    if (QString(oval.at(0)) == "E")
        ui->CONSOLE->append("Invalid trigger delay value.");
    else {
        runCMD("FREQ?");
            float a = QString(oval).toFloat();
            ui->CONSOLE->append("TRIG_DELAY set to " + ui->TRIG_DELAY->text());
            ui->calcTrigdelay->setText(QString("Equals " + QString::number((ui->TRIG_DELAY->text().toFloat()*1000000 / a), 'g', 3) + " us"));
    }
}

void Spark::on_ReadTrigDelay_clicked()
{
    // Will read TRIG_DELAY parameter and calculate its time [us] equivalent

    runCMD("FREQ?");
        float a = QString(oval).toFloat();
    runCMD("TRIG_DELAY?");
        ui->TRIG_DELAY->setText(QString(oval));
        ui->calcTrigdelay->setText(QString("Equals " + QString::number((ui->TRIG_DELAY->text().toFloat()*1000000 / a), 'g', 3) + " us"));
}

void Spark::on_ReadSTT_clicked()
{
    // Will read device status and version number

    runCMD("STT?");
    ui->STT->setText(QString(oval));
    runCMD("VER?");
    ui->VERSION->setText(QString(oval));
}

void Spark::on_START_clicked()
{
    // When START is clicked, it will check the device status first.
    // If RUNNING already, it will do nothing otherwise it will
    // START it.

    runCMD("STT?");
    if (QString(oval.at(0)) == "R") {
        ui->CONSOLE->append("Device already RUNNING");
        ui->STT->setText(QString(oval));
    }
    else {
        runCMD("START");
        runCMD("STT?");
        if (QString(oval.at(0)) == "R") {
            ui->STT->setText(QString(oval));
            ui->CONSOLE->append("Device started");
            }
    }
}

void Spark::on_AutoTRIG_clicked()
{
    // Enables / disables autotriggering (sending the software trigger).
    // First, it checks the device status. If not started, this function
    // would block the GUI since there would be QProcess timeouts.
    // ... So, if device is not started, it will not enable autotrigger.
    // If it's OK, it will call the on_TRIG_clicked function which checks
    // the requested trigger period and starts the timer.

    runCMD("STT?");
    if (QString(oval.at(0)) == "N")
        ui->CONSOLE->append("Device not started.");

    else if (ui->AutoTRIG->isChecked() == true) {
        on_TRIG_clicked();
    }
    else
        timer->stop();
}

void Spark::on_TRIG_clicked()
{
    // This will trigger a software trigger
    // if autotrigger is unchecked.
    // Then it reads the trigger counter and its timestamp
    // and displays both.

    if (ui->AutoTRIG->isChecked() == false) {
        runCMD("TRIG");
        ui->CONSOLE->append("SW trigger sent");

        runCMD("FILL_CNT?");
        ui->FILL_CNT->setText("Trigger count: "+ QString("%1").arg(QString(oval).toDouble()));
        ui->CONSOLE->append("Trigger count: "+ QString("%1").arg(QString(oval).toDouble()));

        runCMD("TRIG_TM?");
        ui->TRIG_TM->setText("Trigger timestamp: " + QString("%1").arg(QString(oval).toLongLong()));
        ui->CONSOLE->append("Trigger timestamp: " + QString("%1").arg(QString(oval).toLongLong()));


    // After issuing the software trigger it will refresh
    // all plots that have auto refresh checked (enabled).

        if (ui->adc_auto_refresh->isChecked() == true)
            on_ReadADC_clicked();

        if (ui->tbtiq_auto_refresh->isChecked() == true)
            on_ReadIQ_clicked();

        if (ui->tbtamp_auto_refresh->isChecked() == true)
            on_ReadTBTAMPL_clicked();

        if (ui->tbtsum_auto_refresh->isChecked() == true)
            on_ReadTBTSUM_clicked();

        if (ui->tbtxy_auto_refresh->isChecked() == true)
            on_ReadTBTXY_clicked();

        if (ui->tdpamp_auto_refresh->isChecked() == true)
            on_ReadTDPAMPL_clicked();

        if (ui->tdpsum_auto_refresh->isChecked() == true)
            on_ReadTDPSUM_clicked();

        if (ui->tdpxy_auto_refresh->isChecked() == true)
            on_ReadTDPXY_clicked();
    }

    else {

     // If the autotrigger is already checked, it will simply restart the autotriggering with
     // time defined in the spinbox (multiply by 1000 to get microseconds).

        timer->start((ui->AUTOTRIG->value() * 1000));

     // I make sure the autoread is disabled not to mix 2 readouts. I check the checkbox first
     // and then send a click to uncheck... It's really not a good solution but I didn't know
     // how to do it better.
     // While writing this comment, I have a clean solution but will not change it in this code.

        ui->ExtTRIG->setChecked(true);
        ui->ExtTRIG->click();
    }
}

void Spark::AutoTrigger()
{
    // This function is called when the "timer" timeouts. "timer" is assigned to autotrigger.
    // We issue the software trigger, read the trigger count and timestamp and refresh
    // all plots that have autorefresh checked.

    runCMD("TRIG");
    ui->CONSOLE->append("SW trigger sent (auto)");

    runCMD("FILL_CNT?");
    ui->FILL_CNT->setText("Trigger count: "+ QString("%1").arg(QString(oval).toDouble()));
    ui->CONSOLE->append("Trigger count: "+ QString("%1").arg(QString(oval).toDouble()));

    runCMD("TRIG_TM?");
    ui->TRIG_TM->setText("Trigger timestamp: " + QString("%1").arg(QString(oval).toLongLong()));
    ui->CONSOLE->append("Trigger timestamp: " + QString("%1").arg(QString(oval).toLongLong()));

    if (ui->adc_auto_refresh->isChecked() == true)
        on_ReadADC_clicked();

    if (ui->tbtiq_auto_refresh->isChecked() == true)
        on_ReadIQ_clicked();

    if (ui->tbtamp_auto_refresh->isChecked() == true)
        on_ReadTBTAMPL_clicked();

    if (ui->tbtsum_auto_refresh->isChecked() == true)
        on_ReadTBTSUM_clicked();

    if (ui->tbtxy_auto_refresh->isChecked() == true)
        on_ReadTBTXY_clicked();

    if (ui->tdpamp_auto_refresh->isChecked() == true)
        on_ReadTDPAMPL_clicked();

    if (ui->tdpsum_auto_refresh->isChecked() == true)
        on_ReadTDPSUM_clicked();

    if (ui->tdpxy_auto_refresh->isChecked() == true)
        on_ReadTDPXY_clicked();

}

void Spark::on_AUTOTRIG_valueChanged(double arg1)
{
    // Restarts the autotrigger with new spinbox value.
    // We multiply the value with 1000 to get microseconds.

    if (ui->AutoTRIG->isChecked() == true)
        timer->start((arg1 * 1000));
}

void Spark::on_EXTTRIG_valueChanged(double arg1)
{
    // Restarts the auto readout with new spinbox value.
    // We multiply the value with 1000 to get microseconds.

    if (ui->ExtTRIG->isChecked() == true)
        exttimer->start((arg1 * 1000));
}

void Spark::on_ExtTRIG_clicked()
{
    // Similar to AutoTRIG, this functions calls periodic
    // readouts but does not issue and software trigger.
    // If unchecked, it will stop the periodic readout.
    // If checked, it starts the periodic readout (exttimer)
    // and makes sure autotriggering is disabled.
    // Implementation is same as in on_AutoTRIG_clicked function
    // it's not really nice... but I'll leave it because it works OK.

    if (ui->ExtTRIG->isChecked() == false) {
        exttimer->stop();
    }
    else {
        ui->CONSOLE->append("Reading on external trigger");
        exttimer->start((ui->EXTTRIG->value() * 1000));
        ui->AutoTRIG->setChecked(true);
        ui->AutoTRIG->click();
    }
}

void Spark::ExtTrigger()
{
    // This function is called when the "exttimer" timeouts. "exttimer" is assigned to auto readout.
    // We simply do buffer readout, read the trigger count and timestamp and refresh
    // all plots that have autorefresh checked.

    runCMD("FILL_CNT?");
    ui->FILL_CNT->setText("Trigger count: "+ QString("%1").arg(QString(oval).toDouble()));

    runCMD("TRIG_TM?");
    ui->TRIG_TM->setText("Trigger timestamp: " + QString("%1").arg(QString(oval).toLongLong()));

    if (ui->adc_auto_refresh->isChecked() == true)
        on_ReadADC_clicked();

    if (ui->tbtiq_auto_refresh->isChecked() == true)
        on_ReadIQ_clicked();

    if (ui->tbtamp_auto_refresh->isChecked() == true)
        on_ReadTBTAMPL_clicked();

    if (ui->tbtsum_auto_refresh->isChecked() == true)
        on_ReadTBTSUM_clicked();

    if (ui->tbtxy_auto_refresh->isChecked() == true)
        on_ReadTBTXY_clicked();

    if (ui->tdpamp_auto_refresh->isChecked() == true)
        on_ReadTDPAMPL_clicked();

    if (ui->tdpsum_auto_refresh->isChecked() == true)
        on_ReadTDPSUM_clicked();

    if (ui->tdpxy_auto_refresh->isChecked() == true)
        on_ReadTDPXY_clicked();

}


void Spark::on_IP_textEdited(const QString &arg1)
{
    // It will update the ip_address variable and window title.
    ip_address = arg1;
    setWindowTitle("Libera Spark / " + ip_address);
}

void Spark::on_resetCNT_clicked()
{
    // It will reset the trigger counter.

    runCMD("FILL_CNT:RST");
    runCMD("FILL_CNT?");
    ui->FILL_CNT->setText("Trigger count: "+ QString("%1").arg(QString(oval).toDouble()));
    ui->CONSOLE->append("Trigger counter reset");
}

void Spark::on_pushButton_clicked()
{
    // Reads all parameters
    startGUI();
}

void Spark::on_ReadFFT_clicked()
{
    // This function will read data (defined in the combobox) and
    // plot the FFT.
    // In the signalFFTselect we define several parameters like:
    //
    // acq_command    ...   which data we will read from device
    // signal_nr      ...   column with the signal we'll use for FFT
    // atom_bytes     ...   number of characters of the selected signal
    // atom_size      ...   number of characters that belong to "1 sample"
    // frq            ...   the signal's frequency

    // Example:
    // ADC signal contains 4 channels (A, B, C, D)
    // Each of them uses 12 characters (atom_bytes).
    // They're four, hence atom_size = 48 (12 * 4)
    // To plot channel C, say signal_nr = 2 (0 based)

    acq_command = "TBT XY ";
    signal_nr = 0;
    atom_bytes = 7;
    atom_size = 14;

    switch (ui->signalFFTselect->currentIndex())
    {
    case 0: // TBT X
    {
        acq_command = "TBT_XY ";
        signal_nr = 0;
        atom_bytes = 7;
        atom_size = 14;
        frq = mc_frq;
        break;
    }
    case 1:  // TBT Y
    {
        acq_command = "TBT_XY ";
        signal_nr = 1;
        atom_bytes = 7;
        atom_size = 14;
        frq = mc_frq;
        break;
    }
    case 2:  // TBT VA
    {
        acq_command = "TBT_AMPL ";
        signal_nr = 0;
        atom_bytes = 7;
        atom_size = 28;
        frq = mc_frq;
        break;
    }
    case 3:  // TBT VB
    {
        acq_command = "TBT_AMPL ";
        signal_nr = 1;
        atom_bytes = 7;
        atom_size = 28;
        frq = mc_frq;
        break;
    }
    case 4:  // TBT VC
    {
        acq_command = "TBT_AMPL ";
        signal_nr = 2;
        atom_bytes = 7;
        atom_size = 28;
        frq = mc_frq;
        break;
    }
    case 5:  // TBT VD
    {
        acq_command = "TBT_AMPL ";
        signal_nr = 3;
        atom_bytes = 7;
        atom_size = 28;
        frq = mc_frq;
        break;
    }
    case 6:  // TBT SUM
    {
        acq_command = "TBT_QSUM ";
        signal_nr = 1;
        atom_bytes = 7;
        atom_size = 14;
        frq = mc_frq;
        break;
    }
    case 7:  // TBT VD
    {
        acq_command = "TBT_QSUM ";
        signal_nr = 0;
        atom_bytes = 7;
        atom_size = 14;
        frq = mc_frq;
        break;
    }
    case 8:  // ADC A
    {
        acq_command = "ADC ";
        signal_nr = 0;
        atom_bytes = 12;
        atom_size = 48;
        frq = adc_frq;
        break;
    }
    case 9:  // ADC B
    {
        acq_command = "ADC ";
        signal_nr = 1;
        atom_bytes = 12;
        atom_size = 48;
        frq = adc_frq;
        break;
    }
    case 10:  // ADC C
    {
        acq_command = "ADC ";
        signal_nr = 2;
        atom_bytes = 12;
        atom_size = 48;
        frq = adc_frq;
        break;
    }
    case 11:  // ADC D
    {
        acq_command = "ADC ";
        signal_nr = 3;
        atom_bytes = 12;
        atom_size = 48;
        frq = adc_frq;
        break;
    }
    }

    // We'll use 2^n samples for FFT. We're not flexible for buffer size
    // but rather offer 4 options (2^10, 2^12, 2^14 and 2^16).

    QString fft_samples;
    switch (ui->FFT_SMPL->currentIndex())
    {
    case 0:
        fft_samples = "1024";
        break;
    case 1:
        fft_samples = "4096";
        break;
    case 2:
        fft_samples = "16384";
        break;
    case 3:
        fft_samples = "65536";
        break;
    }

    runACQ(acq_command + fft_samples);

    fft_axis.setText("Frequency [Hz]");
    int a = fft_samples.toInt();

    QPolygonF points_fft;
    QwtPlotGrid *gridXY = new QwtPlotGrid();
    gridXY->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    gridXY->enableX(true);
    gridXY->enableXMin(true);
    gridXY->enableY(true);
    gridXY->enableYMin(false);
    gridXY->attach(ui->PlotFFT);

    double b;
    float fft_current;

    // We use KISS FFT library for FFT calculation.

    kiss_fft_cpx* in_x_buffer = new kiss_fft_cpx[a];
    kiss_fft_cpx* out_x_buffer = new kiss_fft_cpx[a];

    for (int i = 0; i<a; i++)
    {
        b = QString(oval.mid(i*atom_size + signal_nr*atom_bytes, atom_bytes)).toInt(0,10);
        in_x_buffer[i].r = b;
        in_x_buffer[i].i = 0;
    }

    kiss_fft_cfg cfg = kiss_fft_alloc(a, 0, NULL, NULL);
    kiss_fft(cfg, in_x_buffer, out_x_buffer);

    double maxamp = 20*log10(abs( sqrt(out_x_buffer[0].r * out_x_buffer[0].r + out_x_buffer[0].i * out_x_buffer[0].i)));
    double max1 = 0;
    double max2 = 0;
    double max3 = 0;
    double fftfrq1 = 0;
    double fftfrq2 = 0;
    double fftfrq3 = 0;

    for (int i = 0; i< (a/2 + 1); i++)
    {
        fft_current = 20*log10(abs( sqrt(out_x_buffer[i].r * out_x_buffer[i].r + out_x_buffer[i].i * out_x_buffer[i].i)));
        if (fft_current > maxamp)
            maxamp = fft_current;
    }

    for (int i = 0; i< (a/2 + 1); i++)
    {
        fft_current = (20*log10(abs( sqrt(out_x_buffer[i].r * out_x_buffer[i].r + out_x_buffer[i].i * out_x_buffer[i].i))));
        points_fft << QPointF ((frq / a)*i , (fft_current-maxamp) );

        // Look for 3 max magnitudes -------
                if ((fft_current) > max3) {
                    if ((fft_current) > max2) {
                        if ((fft_current) > max1) {
                            fftfrq3 = fftfrq2;
                            max3 = max2;
                            fftfrq2 = fftfrq1;
                            max2 = max1;
                            fftfrq1 = (frq / a)*i;
                            max1 = fft_current;
                        }
                        else {
                            fftfrq3 = fftfrq2;
                            max3 = max2;
                            fftfrq2 = (frq / a)*i;
                            max2 = fft_current;
                        }
                    }
                    else {
                          fftfrq3 = (frq / a)*i;
                          max3 = fft_current;
                        }
                }
    }

    free(cfg);

    curvefft_x.setData(points_fft);
    curvefft_x.setPen(QPen(Qt::blue, 1,Qt::SolidLine) );
    curvefft_x.attach(ui->PlotFFT);
    ui->PlotFFT->setAxisFont(0,font_plot);
    ui->PlotFFT->setAxisFont(2,font_plot);

    fft_axis_y.setText("Magnitude [dB]");

    ui->PlotFFT->setAxisTitle(0, fft_axis_y);
    ui->PlotFFT->setAxisTitle(2, fft_axis);

    // Print out the 3 max magnitudes
    ui->lFFTpeak1->setText("1st @" + QString::number(fftfrq1, 'g', 6) + " Hz");
    ui->lFFTpeak3->setText("3rd @" + QString::number(fftfrq3, 'g', 6) + " Hz");
    ui->lFFTpeak2->setText("2nd @" + QString::number(fftfrq2, 'g', 6) + " Hz");

    QwtPlotZoomer* zoomer = new QwtPlotZoomer( ui->PlotFFT->canvas(),true);
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
     Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
     Qt::RightButton );
    ui->PlotFFT->setAutoReplot(true);

    ui->PlotFFT->replot();
}

void Spark::on_signalFFTselect_currentIndexChanged(int index)
{
    on_ReadFFT_clicked();
    index = 0;  // We don't actually need it as we read the value directly a bit later
}

void Spark::on_saveButton_clicked()
{
    // Check the currently shown data in the plot
    int data_selected = ui->tabWidget->currentIndex();
    QString tab_name = ui->tabWidget->tabText(data_selected);

    // Set the file name
    QString filename;
    if (ui->filename_time->isChecked() == true) filename = tab_name + "@" + current_time->currentDateTime().toString();
    else
        filename = ui->filename_custom->text();

    QFile file(filename + ".dat");
     file.open(QFile::Truncate | QFile::WriteOnly);
     QTextStream out(&file);

    switch (data_selected)
    {
    case 1:
    {
        int i = curve1_adc_a.dataSize();
        for (int j=0; j<i; j++)
        {
            out << curve1_adc_a.y(j);
            out << " ";
            out << curve2_adc_b.y(j);
            out << " ";
            out << curve3_adc_c.y(j);
            out << " ";
            out << curve4_adc_d.y(j);
            out << " \n";
        }
    }
    case 2:
    {
        int i = curveIA.dataSize();
        for (int j=0; j<i; j++)
        {
            out << curveIA.y(j);
            out << " ";
            out << curveQA.y(j);
            out << " ";
            out << curveIB.y(j);
            out << " ";
            out << curveQB.y(j);
            out << " ";
            out << curveIC.y(j);
            out << " ";
            out << curveQC.y(j);
            out << " ";
            out << curveID.y(j);
            out << " ";
            out << curveQD.y(j);
            out << " \n";
        }
    }
    case 3:
    {
        int i = curve1_tbt_a.dataSize();
        for (int j=0; j<i; j++)
        {
            out << curve1_tbt_a.y(j);
            out << " ";
            out << curve2_tbt_b.y(j);
            out << " ";
            out << curve3_tbt_c.y(j);
            out << " ";
            out << curve4_tbt_d.y(j);
            out << " \n";
        }
    }
    case 4:
    {
        int i = curve1_tbtqsum.dataSize();
        for (int j=0; j<i; j++)
        {
            out << curve1_tbtqsum.y(j);
            out << " ";
            out << curve2_tbtqsum.y(j);
            out << " \n";
        }
    }
    case 5:
    {
        int i = curve1_tbt_x.dataSize();
        for (int j=0; j<i; j++)
        {
            out << curve1_tbt_x.y(j);
            out << " ";
            out << curve2_tbt_y.y(j);
            out << " \n";
        }
    }
    case 6:
    {
        int i = curvefft_x.dataSize();
        for (int j=0; j<i; j++)
        {
            out << curvefft_x.x(j);
            out << " ";
            out << curvefft_x.y(j);
            out << " \n";
        }
    }
    }

    file.close();

    ui->CONSOLE->append(tab_name + " data written to file: " + file.fileName());

}

void Spark::on_filename_time_clicked()
{
    // Switch between custom / automatic file naming

    if (ui->filename_time->isChecked() == true)
        ui->filename_custom->setEnabled(false);
    else
        ui->filename_custom->setEnabled(true);
}

void Spark::on_aboutSpark_clicked()
{
    // Will open About window

    aboutwindow = new About();
    aboutwindow->show();
}

void Spark::on_goConsole_clicked()
{
    // Will open console window

    consolewindow = new console();
    consolewindow->ip = ui->IP->text();
    consolewindow->ui->label->setText("Console on " + ui->IP->text());
    consolewindow->show();
}

void Spark::on_showSP_clicked()
{
    // Will remove SP curves (bold ones)

    if (ui->showSP->isChecked() == false) {
        adcsp_a.detach();
        adcsp_b.detach();
        adcsp_c.detach();
        adcsp_d.detach();
    }
    ui->PlotADC->replot();
}

void Spark::on_REBOOT_clicked()
{
    // Stop auto triggering and readout before rebooting

    timer->stop();
    exttimer->stop();

    ui->CONSOLE->append("Auto trigger and readout disabled.");
    ui->CONSOLE->append("Going to reboot window now.");

    // Open the reboot window

    rebootwindow = new Reboot();
    rebootwindow->ipaddress = ui->IP->text();
    rebootwindow->show();
}

void Spark::on_helpButton_clicked()
{
    // Will open Help window

    helpwindow = new Help();
    helpwindow->show();
}



void Spark::on_ReadTDPSUM_clicked()
{
    // This will request <N> samples from TDPSUM buffer.
    // In the code, the data is parsed (2 columns) to channels SUM, Q
    // We check the radio button if decimated data is requested.

    int a = ui->TDPSUM_SMPL->value();

    if (ui->tdpsum_normal->isChecked() == true) {
        runACQ("TDP_QSUM " + QString::number(a));
        tdp_axis_sum.setText("TDP Turn-by-turn samples");
        tdp_axis_sum_y.setText("SUM [A.U.],\n Q [um]");
    }
    else {
        runACQ("TDP_QSUM_DEC " + QString::number(a));
        tdp_axis_sum.setText("Decimated TDP turn-by-turn samples");
        tdp_axis_sum_y.setText("SUM [A.U.],\n Q [um]");
    }

    QPolygonF points1, points2;

    QwtPlotGrid *gridSUM = new QwtPlotGrid();
    gridSUM->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    gridSUM->enableX(true);
    gridSUM->enableXMin(true);
    gridSUM->enableY(true);
    gridSUM->enableYMin(false);
    gridSUM->attach(ui->PlotTDPSUM);

    int b;
    for (int i = 0; i<a; i++)
    {
        b = QString(oval.mid(i*24,12)).toInt(0,10);
        points1 << QPointF(i,b);
        b = QString(oval.mid(i*24+12,12)).toInt(0,10);
        points2 << QPointF(i,b);
    }

    curve1_tdpqsum.setData(points1);
    curve2_tdpqsum.setData(points2);

    curve1_tdpqsum.setPen(QPen(Qt::blue, 1,Qt::SolidLine) );
    curve2_tdpqsum.setPen(QPen(Qt::red, 1,Qt::SolidLine) );

    if (ui->tdp_sum->isChecked() == true)
        curve1_tdpqsum.attach(ui->PlotTDPSUM);

    if (ui->tdp_q->isChecked() == true)
        curve2_tdpqsum.attach(ui->PlotTDPSUM);

    ui->PlotTDPSUM->setAxisTitle(0, tdp_axis_sum_y);
    ui->PlotTDPSUM->setAxisTitle(2, tdp_axis_sum);
    ui->PlotTDPSUM->setAxisFont(0,font_plot);
    ui->PlotTDPSUM->setAxisFont(2,font_plot);

    QwtPlotZoomer* zoomer = new QwtPlotZoomer( ui->PlotTDPSUM->canvas(),true);
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
     Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
     Qt::RightButton );
    ui->PlotTDPSUM->setAutoReplot(true);

    ui->PlotTDPSUM->replot();
}

void Spark::on_tdp_sum_clicked()
{
    // This will show/hide the channel SUM of TDP data

    if (ui->tdp_sum->isChecked() == true)
        curve1_tdpqsum.attach(ui->PlotTDPSUM);
    else
        curve1_tdpqsum.detach();

    ui->PlotTDPSUM->replot();
}

void Spark::on_tdp_q_clicked()
{
    // This will show/hide the channel Q of TDP data

    if (ui->tdp_q->isChecked() == true)
        curve2_tdpqsum.attach(ui->PlotTDPSUM);
    else
        curve2_tdpqsum.detach();

    ui->PlotTDPSUM->replot();
}



void Spark::on_ReadTDPXY_clicked()
{
    // This will request <N> samples from TDPXY buffer.
    // In the code, the data is parsed (2 columns) to channels X, Y
    // We check the radio button if decimated data is requested.

    if (ui->tdpxy_normal->isChecked() == true) {
        runACQ("TDP_XY " + QString::number(ui->TDPXY_SMPL->value()));
        tdp_axis_xy.setText("TDP Turn-by-turn samples");
        tdp_axis_xy_y.setText("X,Y [um]");
    }
    else {
        runACQ("TDP_XY_DEC " + QString::number(ui->TDPXY_SMPL->value()));
        tdp_axis_xy.setText("Decimated TDP turn-by-turn samples");
        tdp_axis_xy_y.setText("X, Y [um]");
    }

    int a = ui->TDPXY_SMPL->value();

    float x_mean = 0;
    float y_mean = 0;
    float x_rms = 0;
    float y_rms = 0;
    float x[a], y[a];

    QPolygonF points1, points2;

    QwtPlotGrid *gridSUM = new QwtPlotGrid();
    gridSUM->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    gridSUM->enableX(true);
    gridSUM->enableXMin(true);
    gridSUM->enableY(true);
    gridSUM->enableYMin(false);
    gridSUM->attach(ui->PlotTDPXY);


    int b;
    for (int i = 0; i<a; i++)
    {
        b = QString(oval.mid(i*24,12)).toInt(0,10);
        points1 << QPointF(i,b);
        x_mean += b;
        x[i] = b;

        b = QString(oval.mid(i*24+12,12)).toInt(0,10);
        points2 << QPointF(i,b);
        y_mean += b;
        y[i] = b;
    }

    curve1_tdpxy.setData(points1);
    curve2_tdpxy.setData(points2);

    curve1_tdpxy.setPen(QPen(Qt::blue, 1,Qt::SolidLine) );
    curve2_tdpxy.setPen(QPen(Qt::red, 1,Qt::SolidLine) );

    if (ui->tdp_x->isChecked() == true)
        curve1_tdpxy.attach(ui->PlotTDPXY);

    if (ui->tdp_y->isChecked() == true)
        curve2_tdpxy.attach(ui->PlotTDPXY);

    ui->PlotTDPXY->setAxisTitle(0, tdp_axis_xy_y);
    ui->PlotTDPXY->setAxisTitle(2, tdp_axis_xy);
    ui->PlotTDPXY->setAxisFont(0,font_plot);
    ui->PlotTDPXY->setAxisFont(2,font_plot);

    QwtPlotZoomer* zoomer = new QwtPlotZoomer( ui->PlotTDPXY->canvas(),true);
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
     Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
     Qt::RightButton );
    ui->PlotTDPXY->setAutoReplot(true);

    ui->PlotTDPXY->replot();

    // Calculate mean and standard deviation
    x_mean /= a;
    y_mean /= a;

    for (int i = 0; i<a; i++)
    {
        x_rms += (x[i]-x_mean)*(x[i]-x_mean) ;
        y_rms += (y[i]-y_mean)*(y[i]-y_mean) ;
    }

    x_rms /= (a-1);
    y_rms /= (a-1);

    ui->xmean_tdp->setText("X mean: " + QString::number(x_mean, 'f', 2) + " um");
    ui->xrms_tdp->setText("X RMS: " + QString::number(sqrt(x_rms), 'f', 2) + " um");
    ui->ymean_tdp->setText("Y mean: " + QString::number(y_mean, 'f', 2) + " um");
    ui->yrms_tdp->setText("Y RMS: " + QString::number(sqrt(y_rms), 'f', 2) + " um");

}

void Spark::on_tdp_x_clicked()
{
    // This will show/hide the channel X of TDP data

    if (ui->tdp_x->isChecked() == true)
        curve1_tdpxy.attach(ui->PlotTDPXY);
    else
        curve1_tdpxy.detach();

    ui->PlotTDPXY->replot();
}


void Spark::on_tdp_y_clicked()
{
    // This will show/hide the channel Y of TDP data

    if (ui->tdp_y->isChecked() == true)
        curve2_tdpxy.attach(ui->PlotTDPXY);
    else
        curve2_tdpxy.detach();

    ui->PlotTDPXY->replot();
}

void Spark::on_ReadTDPAMPL_clicked()
{
    // This will request <N> samples from TDP AMPL buffer.
    // In the code, the data is parsed (4 columns) to channels VA, VB, VC, VD
    // We check the radio button if decimated data is requested.

    int a = ui->TDPAMPL_SMPL->value();

    if (ui->tdpampl_normal->isChecked() == true) {
        runACQ("TDP_AMPL " + QString::number(a));
        tdp_axis_ampl.setText("TDP Turn-by-turn samples");
        tdp_axis_ampl_y.setText("Amplitudes [a.u.]");
    }
    else {
        runACQ("TDP_AMPL_DEC " + QString::number(a));
        tdp_axis_ampl.setText("Decimated TDP turn-by-turn samples");
        tdp_axis_ampl_y.setText("Amplitudes [a.u.]");
    }

    QPolygonF points1, points2, points3, points4;

    QwtPlotGrid *gridAMPL = new QwtPlotGrid();
    gridAMPL->setPen(QPen(Qt::gray, 0.0, Qt::DotLine));
    gridAMPL->enableX(true);
    gridAMPL->enableXMin(true);
    gridAMPL->enableY(true);
    gridAMPL->enableYMin(false);
    gridAMPL->attach(ui->PlotTDPAMPL);

    int b;
    for (int i = 0; i<a; i++)
    {
        b = QString(oval.mid(i*48,12)).toInt(0,10);
        points1 << QPointF(i,b);
        b = QString(oval.mid(i*48+12,12)).toInt(0,10);
        points2 << QPointF(i,b);
        b = QString(oval.mid(i*48+24,12)).toInt(0,10);
        points3 << QPointF(i,b);
        b = QString(oval.mid(i*48+36,12)).toInt(0,10);
        points4 << QPointF(i,b);
    }

    curve1_tdp_a.setData(points1);
    curve2_tdp_b.setData(points2);
    curve3_tdp_c.setData(points3);
    curve4_tdp_d.setData(points4);

    curve1_tdp_a.setPen(QPen(Qt::blue, 1,Qt::SolidLine) );
    curve2_tdp_b.setPen(QPen(Qt::red, 1,Qt::SolidLine) );
    curve3_tdp_c.setPen(QPen(Qt::green, 1,Qt::SolidLine) );
    curve4_tdp_d.setPen(QPen(Qt::magenta, 1,Qt::SolidLine) );

    if (ui->tdp_ampl_a->isChecked() == true)
        curve1_tdp_a.attach(ui->PlotTDPAMPL);

    if (ui->tdp_ampl_b->isChecked() == true)
        curve2_tdp_b.attach(ui->PlotTDPAMPL);

    if (ui->tdp_ampl_c->isChecked() == true)
        curve3_tdp_c.attach(ui->PlotTDPAMPL);

    if (ui->tdp_ampl_d->isChecked() == true)
        curve4_tdp_d.attach(ui->PlotTDPAMPL);

    ui->PlotTDPAMPL->setAxisTitle(0, tdp_axis_ampl_y);
    ui->PlotTDPAMPL->setAxisTitle(2, tdp_axis_ampl);
    ui->PlotTDPAMPL->setAxisFont(0,font_plot);
    ui->PlotTDPAMPL->setAxisFont(2,font_plot);

    QwtPlotZoomer* zoomer = new QwtPlotZoomer( ui->PlotTDPAMPL->canvas(),true);
    zoomer->setRubberBandPen( QColor( Qt::black ) );
    zoomer->setTrackerPen( QColor( Qt::black ) );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
     Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
     Qt::RightButton );
    ui->PlotTDPAMPL->setAutoReplot(true);

    ui->PlotTDPAMPL->replot();


}

void Spark::on_tdp_ampl_a_toggled(bool checked)
{
    // This will show/hide the channel VA of TDP data

    if (checked == true) {
        curve1_tdp_a.attach(ui->PlotTDPAMPL);
    }
    else
        curve1_tdp_a.detach();

    ui->PlotTDPAMPL->replot();
}

void Spark::on_tdp_ampl_b_toggled(bool checked)
{
    // This will show/hide the channel VB of TDP data

    if (checked == true) {
        curve2_tdp_b.attach(ui->PlotTDPAMPL);
    }
    else
        curve2_tdp_b.detach();

    ui->PlotTDPAMPL->replot();
}

void Spark::on_tdp_ampl_c_toggled(bool checked)
{
    // This will show/hide the channel VC of TDP data

    if (checked == true) {
        curve3_tdp_c.attach(ui->PlotTDPAMPL);
    }
    else
        curve3_tdp_c.detach();

    ui->PlotTDPAMPL->replot();
}

void Spark::on_tdp_ampl_d_toggled(bool checked)
{
    // This will show/hide the channel VD of TDP data

    if (checked == true) {
        curve4_tdp_d.attach(ui->PlotTDPAMPL);
    }
    else
        curve4_tdp_d.detach();

    ui->PlotTDPAMPL->replot();
}

