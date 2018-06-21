#include "printwindow.h"
#include "ui_printwindow.h"
#include <QPushButton>
#include <QSettings>
#include <QMessageBox>

PrintWindow::PrintWindow(MainWindow* main,Terminal* pTerminal,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrintWindow)
{
    ui->setupUi(this);

    pMain = main;

    m_pCPJ = NULL;
    newCPJ = NULL;
    pSliceView = NULL;
    m_pTerminal = pTerminal;
    m_iTattachMS=10000;//cyp
    m_iNumAttach = 3;
    m_iTbaseMS=3000;
    m_iToverMS=1000;
    m_bDryRun = true;//false;
    m_bMirrored = false;
    m_iLastLayer = 0;

    QObject::connect(m_pTerminal, SIGNAL(updateConnectionStatus(QString)), ui->commStatus, SLOT(showMessage(QString)));

    connect(m_pTerminal->pPrinterComm,SIGNAL(BC_CurrentZPosInPU(int)), ui->lineEditTgtPos, SLOT(setText(QString::number(int))));
    connect(m_pTerminal,SIGNAL(BC_ConnectionStatusDetailed(QString)),this, SLOT(onBC_ConnectionStatusDetailed(QString)));

    m_pTerminal->pSettings->loadSettings();
    ui->spinBoxRaiseSpd1->setValue(m_pTerminal->pSettings->m_iRSpd1);
    ui->spinBoxLowerSpd1->setValue(m_pTerminal->pSettings->m_iLSpd1);
    ui->doubleSpinBoxBreathe1->setValue(m_pTerminal->pSettings->m_dBreatheClosed1);
    ui->doubleSpinBoxOverlift1->setValue(m_pTerminal->pSettings->m_dOverLift1);

//    //模型信息
//    ui->lineEditName->setText(m_pCPJ->getName());
//    ui->lineEditXYPixelSizeMicrons->setText(QString::number(1000*m_pCPJ->getXYPixelmm()));
//    ui->lineEditNumOfLayers->setText(QString::number(m_pCPJ->getTotalLayers()));
//    ui->lineEditZSizeMicrons->setText(QString::number(1000*m_pCPJ->getZLayermm()));
//    ui->lineEditZHeight->setText(QString::number(m_pCPJ->getZLayermm()*m_pCPJ->getTotalLayers()));

//    ui->pushButtonSelectAll->setEnabled(false);
//    ui->spinBoxLayersToPrint->setMinimum(1);
//    ui->spinBoxLayersToPrint->setMaximum(m_pCPJ->getTotalLayers());
//    ui->spinBoxLayersToPrint->setValue(m_pCPJ->getTotalLayers());
//    ui->doubleSpinBoxZHeightToPrint->setValue(m_pCPJ->getZLayermm()*ui->spinBoxLayersToPrint->value());

//    ui->doubleSpinBoxTattachMS->setValue(10);
//    ui->spinBoxNumAttach->setValue(3);
//    ui->doubleSpinBoxTbaseMS->setValue(3);
//    ui->doubleSpinBoxToverMS->setValue(1);


//    m_pTerminal->getMatCat()->setCurXYIndex(((ui->lineEditXYPixelSizeMicrons->text().toInt()-25)/25)-1);

//    for(int i=0; i<m_pTerminal->getMatCat()->getMaterialCount(); i++){
//        ui->comboBoxMaterial->addItem(m_pTerminal->getMatCat()->getMaterialLabel(i));
//    }

//    QSettings settings;
//    int index = ui->comboBoxMaterial->findText(QString::number(1000*m_pCPJ->getZLayermm()));
//    if(index<0)index=0;
//    ui->comboBoxMaterial->setCurrentIndex(index);
////    m_pTerminal->getMatCat()->setCurMatIndex(index);
//    settings.setValue("CurrentXYLabel",ui->lineEditXYPixelSizeMicrons->text()+" (祄)");

//    updateTimes();
}

PrintWindow::~PrintWindow()
{
    if(pSliceView)
        delete pSliceView;
//    if(newCPJ){
//        delete newCPJ;
//        newCPJ =NULL;
//    }
    delete ui;
}

void PrintWindow::setCPJ(CrushedPrintJob* pCPJ)
{
    m_pCPJ = pCPJ;
    pMain->m_pCPJ =  pCPJ;
    //模型信息
    ui->lineEditName->setText(m_pCPJ->getName());
    ui->lineEditXYPixelSizeMicrons->setText(QString::number(1000*m_pCPJ->getXYPixelmm()));
    ui->lineEditNumOfLayers->setText(QString::number(m_pCPJ->getTotalLayers()));
    ui->lineEditZSizeMicrons->setText(QString::number(1000*m_pCPJ->getZLayermm()));
    ui->lineEditZHeight->setText(QString::number(m_pCPJ->getZLayermm()*m_pCPJ->getTotalLayers()));

    ui->pushButtonSelectAll->setEnabled(false);
    ui->spinBoxLayersToPrint->setMinimum(1);
    ui->spinBoxLayersToPrint->setMaximum(m_pCPJ->getTotalLayers());
    ui->spinBoxLayersToPrint->setValue(m_pCPJ->getTotalLayers());
    ui->doubleSpinBoxZHeightToPrint->setValue(m_pCPJ->getZLayermm()*ui->spinBoxLayersToPrint->value());

    ui->doubleSpinBoxTattachMS->setValue(10);
    ui->spinBoxNumAttach->setValue(3);
    ui->doubleSpinBoxTbaseMS->setValue(3);
    ui->doubleSpinBoxToverMS->setValue(1);

    m_pTerminal->getMatCat()->setCurXYIndex(((ui->lineEditXYPixelSizeMicrons->text().toInt()-25)/25)-1);

//    QSettings settings;
    int index = ui->comboBoxMaterial->findText(QString::number(1000*m_pCPJ->getZLayermm()));
    if(index<0)index=0;
    ui->comboBoxMaterial->setCurrentIndex(index);
//    m_pTerminal->getMatCat()->setCurMatIndex(index);
//    settings.setValue("CurrentXYLabel",ui->lineEditXYPixelSizeMicrons->text()+" (祄)");

    updateTimes();

}

void PrintWindow::updateTimes()
{
    if(m_pCPJ != NULL){
        QTime vTimeRemains, t;
        int iTime = m_pTerminal->getEstCompleteTimeMS(0,m_iLastLayer,m_pCPJ->getZLayermm(),m_iTbaseMS+m_iToverMS);
        int iM = iTime/60000;
        int iH = iM/60;
        iM = (int)((double)iM+0.5) - iH*60;
        QString sLZ = ":0"; if(iM>9)sLZ = ":";
        QString sTimeRemaining = QString::number(iH)+sLZ+QString::number(iM);
        t.setHMS(0,0,0);
        vTimeRemains = t.addMSecs(iTime);
        ui->lcdNumberTimeRequired->display(vTimeRemains.toString("hh:mm"));
    }

}

void PrintWindow::onBC_ConnectionStatusDetailed(QString sText)
{
    ui->commStatus->setText(sText);
    if(m_pTerminal->pPrinterComm->isConnected()){
        ui->pushButtonPrint->setEnabled(true);
        ui->lineEditTgtPos->setEnabled(true);
        ui->pushButtonStop->setEnabled(true);
        ui->pushButtonCmdReset->setEnabled(true);
        ui->pushButtonFindHomePos->setEnabled(true);
        ui->pushButtonProjPowerON->setEnabled(true);
        ui->pushButtonProjPowerOFF->setEnabled(true);
        ui->pushButtonBoardON->setEnabled(true);
        ui->pushButtonBoardOFF->setEnabled(true);
        ui->checkBox->setEnabled(true);
    }
    else{
        ui->pushButtonPrint->setEnabled(false);
        ui->lineEditTgtPos->setEnabled(false);
        ui->pushButtonStop->setEnabled(false);
        ui->pushButtonCmdReset->setEnabled(false);
        ui->pushButtonFindHomePos->setEnabled(false);
        ui->pushButtonProjPowerON->setEnabled(false);
        ui->pushButtonProjPowerOFF->setEnabled(false);
        ui->pushButtonBoardON->setEnabled(false);
        ui->pushButtonBoardOFF->setEnabled(false);
        ui->checkBox->setEnabled(false);
    }
}

void PrintWindow::on_pushButtonMaterialCatalog_clicked()
{
//    QSettings settings;
//    settings.setValue("CurrentMaterialLabel",ui->comboBoxMaterial->currentText());
////    settings.setValue("CurrentXYLabel",ui->lineEditXYPixelSizeMicrons->text()+" (祄)");
////    m_pTerminal->dlgEditMatCat();

//    m_bInitializing = true;
//    int t = ui->comboBoxMaterial->count();
//    for(int i=0; i<m_pTerminal->getMatCat()->getMaterialCount(); i++){
//        ui->comboBoxMaterial->addItem(m_pTerminal->getMatCat()->getMaterialLabel(i));
//    }
//    for(int i=0; i<t; i++)ui->comboBoxMaterial->removeItem(0);
//    int index = ui->comboBoxMaterial->findText(settings.value("CurrentMaterialLabel","100").toString());
//    if(index<0)index=0;
//    ui->comboBoxMaterial->setCurrentIndex(index);
//    m_bInitializing = false;
}

void PrintWindow::on_pushButtonCycSetting_clicked()
{
    m_pTerminal->updateCycleValues();
    updateTimes();
}

void PrintWindow::on_pushButtonPrint_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Hint");//提示
//    msgBox.setIcon(QMessageBox::Warning);
//    msgBox.setText("\n请确认:\n\t投影仪是否开启\n\t电机是否在零点位置\t\n\t树脂是否备好\n");
    msgBox.setText("\nPlease Sure:\n\tProjector is opend\n\tthe motor found home\t\n\tThe material has fallen in\n");
    msgBox.setToolTip("\n请确认:\n\t投影仪是否开启\n\t电机是否在零点位置\t\n\t树脂是否备好\n");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//    msgBox.setButtonText(QMessageBox::Yes,QString("是"));
//    msgBox.setButtonText(QMessageBox::No,QString("否"));
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if(ret == QMessageBox::No)
        return;
    else
        emit accepted();
}

void PrintWindow::on_pushButtonFindHomePos_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Hint");//提示
//    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("\nThe motor is over the sensor\n");//
    msgBox.setToolTip("请确认电机位置是否在传感器上方");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//    msgBox.setButtonText(QMessageBox::Yes,QString("是"));
//    msgBox.setButtonText(QMessageBox::No,QString("否"));
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if(ret == QMessageBox::No)
        return;
    else{
        m_pTerminal->rcResetHomePos();//cyp
        m_pTerminal->rcGotoFillAfterReset(50);
    }

}


void PrintWindow::on_lineEditTgtPos_returnPressed()
{
    int iValue=ui->lineEditTgtPos->text().toInt();
    if(QString::number(iValue)!=ui->lineEditTgtPos->text()|| iValue<0 || iValue >15000){
        // Bad Value, just return
        ui->lineEditTgtPos->setText("Bad Value");
        return;
    }
    m_pTerminal->rcSendCmd("G"+QString::number(iValue));
}

void PrintWindow::on_pushButtonSelectAll_clicked()
{
    if(m_pCPJ){
        ui->spinBoxLayersToPrint->setValue(m_pCPJ->getTotalLayers());
        ui->doubleSpinBoxZHeightToPrint->setValue(m_pCPJ->getZLayermm()*ui->spinBoxLayersToPrint->value());
    }
}

void PrintWindow::on_spinBoxLayersToPrint_valueChanged(int arg1)
{
    if(m_pCPJ){
        m_iLastLayer = arg1;
        ui->pushButtonSelectAll->setEnabled(m_pCPJ->getTotalLayers()!=m_iLastLayer);
        ui->doubleSpinBoxZHeightToPrint->setValue(m_pCPJ->getZLayermm()*ui->spinBoxLayersToPrint->value());

        updateTimes();

    }
    else
        ui->spinBoxLayersToPrint->setValue(0);
}

void PrintWindow::on_doubleSpinBoxTattachMS_valueChanged(double arg1)
{
    m_iTattachMS = arg1 * 1000;
    updateTimes();
}

void PrintWindow::on_spinBoxNumAttach_valueChanged(int arg1)
{
    m_iNumAttach = arg1;
    updateTimes();
}

void PrintWindow::on_doubleSpinBoxTbaseMS_valueChanged(double arg1)
{
    m_iTbaseMS = arg1 * 1000;
    updateTimes();
}

void PrintWindow::on_doubleSpinBoxToverMS_valueChanged(double arg1)
{
    m_iToverMS = arg1 * 1000;
    updateTimes();
}

void PrintWindow::on_doubleSpinBoxZHeightToPrint_valueChanged(double arg1)
{
//    ui->spinBoxLayersToPrint->setValue(arg1 / m_pCPJ->getZLayermm());
}

void PrintWindow::on_comboBoxMaterial_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "10"){
        ui->doubleSpinBoxTattachMS->setValue(5);
        ui->doubleSpinBoxTbaseMS->setValue(2);
    }
    else if(arg1 == "50" || arg1 == "100" || arg1 == "150"){
        ui->doubleSpinBoxTattachMS->setValue(10);
        ui->doubleSpinBoxTbaseMS->setValue(3);
    }
    else if(arg1 == "200"){
        ui->doubleSpinBoxTattachMS->setValue(15);
        ui->doubleSpinBoxTbaseMS->setValue(3);
    }
}

void PrintWindow::on_pushButtonStop_clicked()
{
    m_pTerminal->on_pushButtonStop_clicked();
}

void PrintWindow::on_pushButtonCmdReset_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Hint");//提示
//    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("\nThe motor is over the sensor\n");//
    msgBox.setToolTip("请确认电机位置是否在传感器上方");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//    msgBox.setButtonText(QMessageBox::Yes,QString("是"));
//    msgBox.setButtonText(QMessageBox::No,QString("否"));
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if(ret == QMessageBox::No)
        return;
    else
        m_pTerminal->on_pushButtonCmdReset_clicked();
}

void PrintWindow::on_pushButtonShowSlice_clicked()
{
    if(m_pCPJ){
        pSliceView = new ShowSlices();
        pSliceView->pCPJ = m_pCPJ;
        pSliceView->GoToSlice(0);
        pSliceView->UpdateWidgets();

        pSliceView->show();
    }
    else{
        QMessageBox msgBox;
        msgBox.setWindowTitle("Hint");//提示
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("\n Please select (.ws) file\n");//
        msgBox.setToolTip("请选择切片文件！");
        msgBox.exec();
    }

}

void PrintWindow::on_pushButtonBrowse_clicked()
{
    QFileDialog dialog(0);
    QSettings settings("Dir");
    QString openFile = dialog.getOpenFileName(this,"Select WINSUN File", settings.value("WorkingDir").toString(), tr("WINSUN (*.ws)"));//cyp .b9j
    if(openFile.isEmpty()) return;
    settings.setValue("WorkingDir", QFileInfo(openFile).absolutePath());


//    m_pCPJ->clearAll();

    newCPJ = new CrushedPrintJob;
//    m_pCPJ = new CrushedPrintJob;
    QFile file(openFile);
    if(!newCPJ->loadCPJ(&file)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("WINSUN");//
        msgBox.setText("Error Loading File.  Unknown Version?");//"");
        msgBox.setToolTip("文件加载错误。未知版本？");
        msgBox.exec();
        delete newCPJ;
        newCPJ = NULL;
        return;
    }
    if(m_pCPJ){//释放之前的m_pCPJ内存空间
        delete m_pCPJ;
        m_pCPJ = NULL;
        pMain->m_pCPJ = NULL;
    }
    ui->checkBox->setChecked(false);
    newCPJ->showSupports(true);
    newCPJ->setName(openFile);//cyp
    setCPJ(newCPJ);
}

void PrintWindow::on_pushButtonProjPowerON_clicked()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Hint");//提示
//    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Is the board closed?");//
    msgBox.setInformativeText("Are you sure you want to open the projector?");//
    msgBox.setToolTip("请确认挡板是否关上！\n确定要打开投影仪吗？");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//    msgBox.setButtonText(QMessageBox::Yes,QString("是"));
//    msgBox.setButtonText(QMessageBox::No,QString("否"));
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if(ret == QMessageBox::No)
        return;
    else
        m_pTerminal->rcSendCmd("P1");//open

}

void PrintWindow::on_pushButtonProjPowerOFF_clicked()
{
    m_pTerminal->rcSendCmd("P0");//close
}

void PrintWindow::on_pushButtonBoardON_clicked()
{
    m_pTerminal->rcSendCmd("#0");//open
}

void PrintWindow::on_pushButtonBoardOFF_clicked()
{
    m_pTerminal->rcSendCmd("#1");//close
}

void PrintWindow::on_pushButtonResetHomePos_clicked()
{
    int iValue=ui->lineEditHomePos->text().toInt();//感应器到零点的补偿值
    if(QString::number(iValue)!=ui->lineEditHomePos->text()|| iValue<0 || iValue >15000){
        // Bad Value, just return
        ui->lineEditHomePos->setText("Bad Value");
        return;
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle("Hint");//提示
//    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Reset the home position");//
    msgBox.setToolTip("重设零点位置\n确定设置零点为传感器向下补偿"+ui->lineEditHomePos->text()+"步的位置吗？");//Are you sure you want to orientate?
    msgBox.setInformativeText("Are you sure you want to reset the zero is located "+ui->lineEditHomePos->text()+" steps below the sensor?");//Are you sure you want to orientate?
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//    msgBox.setButtonText(QMessageBox::Yes,QString("是"));
//    msgBox.setButtonText(QMessageBox::No,QString("否"));
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if(ret == QMessageBox::No)
        return;
    else{
        QString cmd = "Y" + ui->lineEditHomePos->text();
        m_pTerminal->rcSendCmd(cmd);
    }

}

void PrintWindow::on_checkBox_toggled(bool checked)
{
    ui->lineEditHomePos->setEnabled(checked);
    ui->pushButtonResetHomePos->setEnabled(checked);

    if(!checked){
        ui->lineEditHomePos->setText("");
    }
}

void PrintWindow::on_pushButtonRestoreDefaults_clicked()
{
    ui->spinBoxRaiseSpd1->setValue(130);
    ui->spinBoxLowerSpd1->setValue(130);
    ui->doubleSpinBoxOverlift1->setValue(4);
    ui->doubleSpinBoxBreathe1->setValue(1.5);
}

void PrintWindow::on_spinBoxRaiseSpd1_valueChanged(int arg1)
{
    QSettings settings("cycsettings");
    settings.setValue("RSpd1",arg1);
    m_pTerminal->pSettings->loadSettings();
    updateTimes();
}

void PrintWindow::on_spinBoxLowerSpd1_valueChanged(int arg1)
{
    QSettings settings("cycsettings");
    settings.setValue("LSpd1",arg1);
    m_pTerminal->pSettings->loadSettings();
    updateTimes();
}

void PrintWindow::on_doubleSpinBoxOverlift1_valueChanged(double arg1)
{
    QSettings settings("cycsettings");
    settings.setValue("OverLift1",arg1);
    m_pTerminal->pSettings->loadSettings();
    updateTimes();
}

void PrintWindow::on_doubleSpinBoxBreathe1_valueChanged(double arg1)
{
    QSettings settings("cycsettings");
    settings.setValue("BreatheClosed1",arg1);
    settings.setValue("SettleOpen1",arg1);
    m_pTerminal->pSettings->loadSettings();
    updateTimes();
}
