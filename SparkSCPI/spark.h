#ifndef SPARK_H
#define SPARK_H

#include <QMainWindow>
#include <QProcess>
#include <QTimer>
#include <QDateTime>
#include <qwt-qt4/qwt_plot.h>
#include <qwt-qt4/qwt_plot_curve.h>
#include <qwt-qt4/qwt_plot_grid.h>
#include <kiss_fft130/kiss_fft.h>
#include <QFont>
#include "about.h"
#include "console.h"
#include "reboot.h"
#include "help.h"

namespace Ui {
class Spark;
}

class Spark : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Spark(QWidget *parent = 0);
    ~Spark();

    QProcess process1, process2;
    QString env;
    QwtText tbt_axis, tbt_axis_iq, tbt_axis_sum, tbt_axis_ampl, fft_axis, tbt_axis_y, tbt_axis_iq_y, tbt_axis_sum_y, tbt_axis_ampl_y, fft_axis_y, adc_axis, adc_axis_y, tdp_axis_sum, tdp_axis_sum_y, tdp_axis_xy, tdp_axis_xy_y, tdp_axis_ampl, tdp_axis_ampl_y;

    QByteArray oval;
    float frequency, rf_frq, adc_sp_x, adc_sp_y;
    int harmonic_nr, signal_nr, atom_size, atom_bytes, sp_threshold, sp_window;
    double mc_frq, adc_frq, frq;

    int a;
    QTimer *timer;
    QTimer *exttimer;
    QTimer *local_time;
    QDateTime *current_time;

    QString runCMD(QString command);
    QString runACQ(QString command);

    QString acq_command, ip_address;

    QwtPlotCurve curve1_adc_a, curve2_adc_b, curve3_adc_c, curve4_adc_d;
    QwtPlotCurve adcsp_a, adcsp_b, adcsp_c, adcsp_d;
    QwtPlotCurve curve1_tbtqsum, curve2_tbtqsum, curve1_tbt_a, curve2_tbt_b, curve3_tbt_c, curve4_tbt_d, curve1_tbt_x, curve2_tbt_y;
    QwtPlotCurve curveIA, curveQA, curveIB, curveQB, curveIC, curveQC, curveID, curveQD;
    QwtPlotCurve curve1_tbtqsum_d, curve2_tbtqsum_d, curve1_tbt_a_d, curve2_tbt_b_d, curve3_tbt_c_d, curve4_tbt_d_d, curve1_tbt_x_d, curve2_tbt_y_d;
    QwtPlotCurve curve1_tdpxy, curve2_tdpxy, curve1_tdpqsum, curve2_tdpqsum, curve1_tdp_a, curve2_tdp_b, curve3_tdp_c, curve4_tdp_d;
    QwtPlotCurve curveIA_d, curveQA_d, curveIB_d, curveQB_d, curveIC_d, curveQC_d, curveID_d, curveQD_d;
    QwtPlotCurve curvefft_x, curvefft_y;

    QwtPlotGrid *grid;
    QFont font_plot, font_axis, font_alarm;

    Ui::Spark *ui;


private slots:
    void on_ParamSet_clicked();
    void on_ParamRead_clicked();
    void on_FreqRead_clicked();
    void on_ReadADC_clicked();
    void on_TRIG_clicked();
    void on_procParamSet_clicked();
    void on_procParamRead_clicked();
    void on_ReadIQ_clicked();
    void on_ReadTBTAMPL_clicked();
    void on_tbt_ampl_a_clicked();
    void on_ReadTBTSUM_clicked();
    void on_tbt_sum_clicked();
    void on_tbt_q_clicked();
    void on_adc_a_clicked();
    void on_adc_b_clicked();
    void on_adc_c_clicked();
    void on_adc_d_clicked();
    void on_tbt_ampl_b_clicked();
    void on_tbt_ampl_c_clicked();
    void on_tbt_ampl_d_clicked();
    void on_ReadTBTXY_clicked();
    void on_tbt_x_clicked();
    void on_tbt_y_clicked();
    void on_SetTrigDelay_clicked();
    void on_ReadTrigDelay_clicked();
    void on_ReadSTT_clicked();
    void on_START_clicked();
    void on_ia_clicked();
    void on_qa_clicked();
    void on_ib_clicked();
    void on_qb_clicked();
    void on_ic_clicked();
    void on_qc_clicked();
    void on_id_clicked();
    void on_qd_clicked();
    void AutoTrigger();
    void ExtTrigger();
    void on_AutoTRIG_clicked();
    void on_AUTOTRIG_valueChanged(double arg1);
    void on_IP_textEdited(const QString &arg1);
    void on_resetCNT_clicked();
    void startGUI();
    void on_pushButton_clicked();
    void showLocalTime();
    void on_ExtTRIG_clicked();
    void on_EXTTRIG_valueChanged(double arg1);
    void on_ReadFFT_clicked();
    void on_RF_textEdited(const QString &arg1);
    void on_HN_textEdited(const QString &arg1);
    void on_signalFFTselect_currentIndexChanged(int index);
    void on_adcsizeSet_clicked();
    void on_adcsizeRead_clicked();
    void setFonts();
    void on_saveButton_clicked();
    void on_filename_time_clicked();
    void on_aboutSpark_clicked();
    void on_goConsole_clicked();
    void on_showSP_clicked();
    void on_REBOOT_clicked();
    void on_FreqSet_clicked();
    void on_helpButton_clicked();
    void on_ReadTDPSUM_clicked();
    void on_tdp_sum_clicked();
    void on_tdp_q_clicked();
    void on_ReadTDPXY_clicked();
    void on_tdp_x_clicked();
    void on_tdp_y_clicked();
    void on_ReadTDPAMPL_clicked();
    void on_tdp_ampl_a_toggled(bool checked);
    void on_tdp_ampl_b_toggled(bool checked);
    void on_tdp_ampl_c_toggled(bool checked);
    void on_tdp_ampl_d_toggled(bool checked);

private:
//    Ui::Spark *ui;
    About *aboutwindow;
    console *consolewindow;
    Reboot *rebootwindow;
    Help *helpwindow;
};

#endif // SPARK_H
