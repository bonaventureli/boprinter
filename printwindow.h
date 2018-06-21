#ifndef PRINTWINDOW_H
#define PRINTWINDOW_H

#include <QWidget>
#include <cycleprintsetting.h>
#include <print.h>
#include <crushbitmap.h>
#include <terminal.h>
#include <showslices.h>
#include "mainwindow.h"

namespace Ui {
class PrintWindow;
}

class MainWindow;
class PrintWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PrintWindow(MainWindow* main,Terminal* pTerminal,QWidget *parent = 0);
    ~PrintWindow();
    void updateTimes();
    void setCPJ(CrushedPrintJob* pCPJ);

signals:
    void accepted();


private slots:
    void onBC_ConnectionStatusDetailed(QString sText);

    void on_pushButtonMaterialCatalog_clicked();

    void on_pushButtonCycSetting_clicked();

    void on_pushButtonPrint_clicked();

    void on_pushButtonFindHomePos_clicked();

    void on_lineEditTgtPos_returnPressed();

    void on_pushButtonSelectAll_clicked();

    void on_spinBoxLayersToPrint_valueChanged(int arg1);


    void on_doubleSpinBoxTattachMS_valueChanged(double arg1);

    void on_spinBoxNumAttach_valueChanged(int arg1);

    void on_doubleSpinBoxTbaseMS_valueChanged(double arg1);

    void on_doubleSpinBoxToverMS_valueChanged(double arg1);

    void on_doubleSpinBoxZHeightToPrint_valueChanged(double arg1);

    void on_comboBoxMaterial_currentIndexChanged(const QString &arg1);

    void on_pushButtonStop_clicked();

    void on_pushButtonCmdReset_clicked();

    void on_pushButtonShowSlice_clicked();

    void on_pushButtonBrowse_clicked();

    void on_pushButtonProjPowerON_clicked();

    void on_pushButtonProjPowerOFF_clicked();

    void on_pushButtonBoardON_clicked();

    void on_pushButtonBoardOFF_clicked();

    void on_pushButtonResetHomePos_clicked();

    void on_checkBox_toggled(bool checked);

    void on_pushButtonRestoreDefaults_clicked();

    void on_spinBoxRaiseSpd1_valueChanged(int arg1);

    void on_spinBoxLowerSpd1_valueChanged(int arg1);

    void on_doubleSpinBoxOverlift1_valueChanged(double arg1);

    void on_doubleSpinBoxBreathe1_valueChanged(double arg1);

public:
    int m_iTattachMS;
    int m_iNumAttach;
    int m_iTbaseMS;
    int m_iToverMS;
    bool m_bMirrored;
    bool m_bDryRun;
    int m_iLastLayer;
    CrushedPrintJob *newCPJ;
    MainWindow* pMain;


private:
    Ui::PrintWindow *ui;
    CrushedPrintJob *m_pCPJ;
    ShowSlices *pSliceView;
    Terminal *m_pTerminal;
    bool m_bInitializing;

};

#endif // PRINTWINDOW_H
