
#include <QtDebug>
#include <QMessageBox>
#include <QSettings>
#include "terminal.h"
#include "ui_terminal.h"
#include "cycleprintsetting.h"


#define RAISE1 80
#define LOWER1 100
#define CLOSE1 100
#define OPEN1 100
#define BREATHE1 2.0
#define SETTLE1 3.0
#define OVERLIFT1 4.0


void PCycleSettings::updateValues()
{
    CyclePrintSetting dlg(this);
    dlg.exec();
}

void PCycleSettings::loadSettings()
{

    QSettings settings("cycsettings");
    m_iRSpd1 = settings.value("RSpd1",RAISE1).toInt();
    m_iLSpd1 = settings.value("LSpd1",LOWER1).toInt();
    m_iCloseSpd1 = settings.value("CloseSpd1",CLOSE1).toInt();
    m_iOpenSpd1 = settings.value("OpenSpd1",OPEN1).toInt();
    m_dBreatheClosed1 = settings.value("BreatheClosed1",BREATHE1).toDouble();
    m_dSettleOpen1 = settings.value("SettleOpen1",SETTLE1).toDouble();
    m_dOverLift1 = settings.value("OverLift1",OVERLIFT1).toDouble();

}

void PCycleSettings::saveSettings()
{
    //cyp 速度百分比改成实际转速
    QSettings settings("cycsettings");
    settings.setValue("RSpd1",m_iRSpd1);

    settings.setValue("LSpd1",m_iLSpd1);
    settings.setValue("CloseSpd1",m_iCloseSpd1);
    settings.setValue("OpenSpd1",m_iOpenSpd1);
    settings.setValue("BreatheClosed1",m_dBreatheClosed1);
    settings.setValue("SettleOpen1",m_dSettleOpen1);
    settings.setValue("OverLift1",m_dOverLift1);

}

void PCycleSettings::setFactorySettings()
{
    m_iRSpd1 = RAISE1;
    m_iLSpd1 = LOWER1;
    m_iOpenSpd1 = OPEN1;
    m_iCloseSpd1 = CLOSE1;
    m_dBreatheClosed1 = BREATHE1;
    m_dSettleOpen1 = SETTLE1;
    m_dOverLift1 = OVERLIFT1;
}



Terminal::Terminal(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Terminal)
{
    m_bWaiverPresented = false;
    m_bWaiverAccepted = false;
    m_bWavierActive = false;
    m_bNeedsWarned = true;
    m_iFillLevel = -1;

    ui->setupUi(this);

    ui->commStatus->setText("Searching for WINSUN printer...");//B9Creator

    m_pCatalog = new B9MatCat;
    onBC_ModelInfo("B9C1");

    pSettings = new PCycleSettings;
    resetLastSentCycleSettings();

    pPrinterComm = new B9PrinterComm;
    pPrinterComm->enableBlankCloning(true); // Allow for firmware update of suspected "blank" B9Creator Arduino's

    // Always set up the B9Projector in the Terminal constructor
    m_pDesktop = QApplication::desktop();
    pProjector = NULL;
    m_bPrimaryScreen = false;
    m_bPrintPreview = false;
    m_bUsePrimaryMonitor = false;

    connect(m_pDesktop, SIGNAL(screenCountChanged(int)),this, SLOT(onScreenCountChanged(int)));

    connect(pPrinterComm,SIGNAL(updateConnectionStatus(QString)), this, SLOT(onUpdateConnectionStatus(QString)));
    connect(pPrinterComm,SIGNAL(BC_ConnectionStatusDetailed(QString)), this, SLOT(onBC_ConnectionStatusDetailed(QString)));
    connect(pPrinterComm,SIGNAL(BC_LostCOMM()),this,SLOT(onBC_LostCOMM()));

    connect(pPrinterComm,SIGNAL(BC_RawData(QString)), this, SLOT(onUpdateRAWPrinterComm(QString)));
    connect(pPrinterComm,SIGNAL(BC_Comment(QString)), this, SLOT(onUpdatePrinterComm(QString)));

    connect(pPrinterComm,SIGNAL(BC_ModelInfo(QString)),this,SLOT(onBC_ModelInfo(QString)));
    connect(pPrinterComm,SIGNAL(BC_FirmVersion(QString)),this,SLOT(onBC_FirmVersion(QString)));
    connect(pPrinterComm,SIGNAL(BC_ProjectorRemoteCapable(bool)), this, SLOT(onBC_ProjectorRemoteCapable(bool)));
    connect(pPrinterComm,SIGNAL(BC_HasShutter(bool)), this, SLOT(onBC_HasShutter(bool)));
    connect(pPrinterComm,SIGNAL(BC_ProjectorStatusChanged()), this, SLOT(onBC_ProjStatusChanged()));
    connect(pPrinterComm,SIGNAL(BC_ProjectorFAIL()), this, SLOT(onBC_ProjStatusFAIL()));

    // Z Position Control
    connect(pPrinterComm, SIGNAL(BC_PU(int)), this, SLOT(onBC_PU(int)));
    connect(pPrinterComm, SIGNAL(BC_UpperZLimPU(int)), this, SLOT(onBC_UpperZLimPU(int)));
    m_pResetTimer = new QTimer(this);
    connect(m_pResetTimer, SIGNAL(timeout()), this, SLOT(onMotionResetTimeout()));
    connect(pPrinterComm, SIGNAL(BC_HomeFound()), this, SLOT(onMotionResetComplete()));
    connect(pPrinterComm, SIGNAL(BC_CurrentZPosInPU(int)), this, SLOT(onBC_CurrentZPosInPU(int)));
    connect(pPrinterComm, SIGNAL(BC_HalfLife(int)), this, SLOT(onBC_HalfLife(int)));
    connect(pPrinterComm, SIGNAL(BC_NativeX(int)), this, SLOT(onBC_NativeX(int)));
    connect(pPrinterComm, SIGNAL(BC_NativeY(int)), this, SLOT(onBC_NativeY(int)));
    connect(pPrinterComm, SIGNAL(BC_XYPixelSize(int)), this, SLOT(onBC_XYPixelSize(int)));

//    connect(ui->lineEditCommand, SIGNAL(returnPressed()), Terminal, SLOT(sendCommand()));

    m_pVatTimer = new QTimer(this);
    connect(m_pVatTimer, SIGNAL(timeout()), this, SLOT(onMotionVatTimeout()));
    connect(pPrinterComm, SIGNAL(BC_CurrentVatPercentOpen(int)), this, SLOT(onBC_CurrentVatPercentOpen(int)));

    m_pPReleaseCycleTimer = new QTimer(this);
    connect(m_pPReleaseCycleTimer, SIGNAL(timeout()), this, SLOT(onReleaseCycleTimeout()));
    connect(pPrinterComm, SIGNAL(BC_PrintReleaseCycleFinished()), this, SLOT(onBC_PrintReleaseCycleFinished()));

}

Terminal::~Terminal()
{
    delete ui;
    delete pProjector;
    delete pPrinterComm;
    qDebug() << "Terminal End";
}

int Terminal::getEstBaseCycleTime(int iCur, int iTgt){
    int iDelta = abs(iTgt - iCur);
    int iLowerSpd,iOpnSpd,iSettle;

        iLowerSpd = pSettings->m_iLSpd1;
        iOpnSpd = pSettings->m_iOpenSpd1;
        iSettle = pSettings->m_dSettleOpen1*1000.0;

    // Time to move iDelta
    int iTimeReq = getZMoveTime(iDelta, iLowerSpd);
    // Plus time to open vat
    iTimeReq += getVatMoveTime(iOpnSpd);
    // Plus settle time;
    iTimeReq += iSettle;
    return iTimeReq;
}

int Terminal::getEstNextCycleTime(int iCur, int iTgt){
    int iDelta = abs(iTgt - iCur);
    int iRaiseSpd,iLowerSpd,iOpnSpd,iClsSpd,iGap,iBreathe,iSettle;

        iRaiseSpd = pSettings->m_iRSpd1;
        iLowerSpd = pSettings->m_iLSpd1;
        iOpnSpd = pSettings->m_iOpenSpd1;
        iClsSpd = pSettings->m_iCloseSpd1;
        iGap = (int)(pSettings->m_dOverLift1*100000.0/(double)pPrinterComm->getPU());
        iBreathe = pSettings->m_dBreatheClosed1*1000.0;
        iSettle = pSettings->m_dSettleOpen1*1000.0;

    // Time to move +iDelta + iGap, up and down
    int iTimeReq = getZMoveTime(iDelta+iGap, iRaiseSpd);
    iTimeReq += getZMoveTime(iDelta+iGap, iLowerSpd);
    // Plus time to close + open the vat
    iTimeReq += getVatMoveTime(iClsSpd)+getVatMoveTime(iOpnSpd);
    // Plus breathe & settle time;
    iTimeReq += iBreathe + iSettle;
    return iTimeReq;
}

int Terminal::getEstFinalCycleTime(int iCur, int iTgt){
    int iDelta = abs(iTgt - iCur);
    int iRaiseSpd,iClsSpd;

        iRaiseSpd = pSettings->m_iRSpd1 / 130;
        iClsSpd = pSettings->m_iCloseSpd1 / 130;

    // Time to move +iDelta up
    int iTimeReq = getZMoveTime(iDelta, iRaiseSpd);
    // time to close the vat
    iTimeReq += getVatMoveTime(iClsSpd);
    return iTimeReq;
}

QTime Terminal::getEstCompleteTime(int iCurLayer, int iTotLayers, double dLayerThicknessMM, int iExposeMS)
{
    return QTime::currentTime().addMSecs(getEstCompleteTimeMS(iCurLayer, iTotLayers, dLayerThicknessMM, iExposeMS));
}

int Terminal::getEstCompleteTimeMS(int iCurLayer, int iTotLayers, double dLayerThicknessMM, int iExposeMS)
{
    //return estimated completion time

      int  iLowerCount=iTotLayers;

        iLowerCount = iLowerCount-iCurLayer;

    int iTotalTimeMS = iExposeMS*iLowerCount;// + iExposeMS*iUpperCount;

    iTotalTimeMS = getLampAdjustedExposureTime(iTotalTimeMS);

    // Add Breathe and Settle
    iTotalTimeMS += iLowerCount*(pSettings->m_dBreatheClosed1 + pSettings->m_dSettleOpen1)*1000;

    // Z Travel Time
    int iGap1 = iLowerCount*(int)(pSettings->m_dOverLift1*100000.0/(double)pPrinterComm->getPU());

    int iZRaiseDistance1 = iGap1 + iLowerCount*(int)(dLayerThicknessMM*100000.0/(double)pPrinterComm->getPU());
    int iZLowerDistance1 = iGap1;

    //cyp 百分比速率改成转速
    iTotalTimeMS += getZMoveTime(iZRaiseDistance1,pSettings->m_iRSpd1);
    iTotalTimeMS += getZMoveTime(iZLowerDistance1,pSettings->m_iLSpd1);

    // Vat movement Time
    iTotalTimeMS += iLowerCount*getVatMoveTime(pSettings->m_iOpenSpd1) + iLowerCount*getVatMoveTime(pSettings->m_iCloseSpd1);
    return iTotalTimeMS;
}


int Terminal::getLampAdjustedExposureTime(int iBaseTimeMS)
{
    if(pPrinterComm==NULL||pPrinterComm->getLampHrs()<0||pPrinterComm->getHalfLife()<0)return iBaseTimeMS;

    //  dLife = 0.0 at zero lamp hours and 1.0 at or above halflife hours.
    //  We multiply the base time by dLife and return the original amount + the product.
    //  So at Halflife, we've doubled the standard exposure time.
    double dLife = (double)pPrinterComm->getLampHrs()/(double)pPrinterComm->getHalfLife();
    if(dLife > 1.0)dLife = 1.0; // Limit to 100% the amount of applied bulb degradation (reached at HalfLife)
    return iBaseTimeMS + (double)iBaseTimeMS*dLife;
}


int Terminal::getZMoveTime(int iDelta, int iSpd){
    // returns time to travel iDelta PUs distance in milliseconds
    // Accurate but assumes that 100% is 130rpm and 0% is 50rpm
    // Also assumes 200 PU (Steps) per revolution
    // returns milliseconds required to move iDelta PU's
    if(iDelta==0)return 0;
    double dPUms; // printer units per millisecond
//    dPUms = ((double)iSpd/100.0)*80.0 + 50.0;
    dPUms = iSpd;//((double)iSpd/100.0)*130.0;
    dPUms *= 200.0; // PU per minute
    dPUms /= 60; // PU per second
    dPUms /= 1000; // PU per millisecond
    return (int)(double(iDelta)/dPUms);
}

int Terminal::getVatMoveTime(int iSpeed){
    double dPercent = (double)iSpeed/100.0;
//    return 999 - dPercent*229.0; // based on speed tests of B9C1 on 11/14/2012
    return 1500 - dPercent*229.0; // updated based on timeouts on pre-production model tests 11/18/2012
}

void Terminal::onScreenCountChanged(int iCount){
    QString sVideo = "Disconnected or Primary Monitor";
    if(pProjector) {
        delete pProjector;
        pProjector = NULL;
        if(pPrinterComm->getProjectorStatus()==B9PrinterStatus::PS_ON)
            if(!isEnabled())emit signalAbortPrint("Print Aborted:  Connection to Projector Lost or Changed During Print Cycle");
    }
    pProjector = new B9Projector(true, 0,Qt::WindowStaysOnTopHint);
    makeProjectorConnections();
    int i=iCount;
    int screenCount = m_pDesktop->screenCount();
    QRect screenGeometry;

    if(m_bUsePrimaryMonitor)
    {
        screenGeometry = m_pDesktop->screenGeometry(0);
    }
    else{
        for(i=screenCount-1;i>= 0;i--) {
            screenGeometry = m_pDesktop->screenGeometry(i);
            if(screenGeometry.width() == pPrinterComm->getNativeX() && screenGeometry.height() == pPrinterComm->getNativeY()) {
                //Found the projector!
                sVideo = "Connected to Monitor: " + QString::number(i+1);
                m_bNeedsWarned = true;
                break;
            }
        }
    }
    if(i<=0||m_bUsePrimaryMonitor)m_bPrimaryScreen = true; else m_bPrimaryScreen = false;

    emit updateProjectorOutput(sVideo);

    pProjector->setShowGrid(true);
    pProjector->setCPJ(NULL);

    emit sendStatusMsg("WINSUN - Idle");//cyp B9Creator
    pProjector->setGeometry(1920,0,100,100);//screenGeometry
    if(!m_bPrimaryScreen){
        pProjector->showFullScreen(); // Only show it if it is a secondary monitor
        pProjector->hide();
        activateWindow(); // if not using primary monitor, take focus back to here.
    }
    else if(m_bPrintPreview||(pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_OFF &&
            pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_COOLING &&
            pPrinterComm->getProjectorStatus() != B9PrinterStatus::PS_UNKNOWN)) {
        // if the projector is not turned off, we better put up the blank screen now!
        pProjector->showFullScreen();
    }
    //else warnSingleMonitor();

}

void Terminal::onBC_ConnectionStatusDetailed(QString sText)
{
    setEnabledWarned();

    ui->commStatus->setText(sText);

    emit BC_ConnectionStatusDetailed(sText);
}

void Terminal::onBC_LostCOMM(){
    //Broadcast an alert
    if(!isEnabled())emit signalAbortPrint("ERROR: Lost Printer Connection.  Possible reasons: Power Loss, USB cord unplugged.");
    qDebug() << "BC_LostCOMM signal received.";
}

void Terminal::onUpdateRAWPrinterComm(QString sText)
{
    QString html = "<font color=\"Blue\">" + sText + "</font><br>";
    ui->textEditCommOut->insertHtml(html);
    html = ui->textEditCommOut->toHtml();
    ui->textEditCommOut->clear();
    ui->textEditCommOut->insertHtml(html.right(2000));
    ui->textEditCommOut->setAlignment(Qt::AlignBottom);
}

void Terminal::onUpdatePrinterComm(QString sText)
{
    QString html = "<font color=\"Black\">" + sText + "</font><br>";
    ui->textEditCommOut->insertHtml(html);
    html = ui->textEditCommOut->toHtml();
    ui->textEditCommOut->clear();
    ui->textEditCommOut->insertHtml(html.right(2000));
    ui->textEditCommOut->setAlignment(Qt::AlignBottom);
}

void Terminal::onBC_ModelInfo(QString sModel){
    m_sModelName = sModel;
    m_pCatalog->load(m_sModelName);
//    ui->lineEditModelInfo->setText(m_sModelName);
    resetLastSentCycleSettings();
}

void Terminal::onBC_FirmVersion(QString sVersion){
//    ui->lineEditFirmVersion->setText(sVersion);
}

void Terminal::onBC_ProjectorRemoteCapable(bool bCapable){
//    ui->groupBoxProjector->setEnabled(bCapable);
    ui->pushButtonProjPower->setEnabled(bCapable);
}
void Terminal::onBC_HasShutter(bool bHS){
//    ui->groupBoxVAT->setEnabled(bHS);
}

void Terminal::onBC_ProjStatusChanged()
{
    QString sText = "UNKNOWN";
    bool isTurningOn = false;
    switch (pPrinterComm->getProjectorStatus()){
    case B9PrinterStatus::PS_OFF:
        ui->pushButtonProjPower->setEnabled(true);
        sText = "OFF";
        break;
    case B9PrinterStatus::PS_TURNINGON:
        isTurningOn = true;
        ui->pushButtonProjPower->setEnabled(false);
        sText = "TURN ON";
        break;
    case B9PrinterStatus::PS_WARMING:
        ui->pushButtonProjPower->setEnabled(true);
        sText = "WARM UP";
        break;
    case B9PrinterStatus::PS_ON:
        ui->pushButtonProjPower->setEnabled(true);
        emit(ProjectorIsOn());
        sText = "ON";
        break;
    case B9PrinterStatus::PS_COOLING:
        ui->pushButtonProjPower->setEnabled(false);
        sText = "COOL DN";
        break;
    case B9PrinterStatus::PS_TIMEOUT:
        ui->pushButtonProjPower->setEnabled(false);
        sText = "TIMEOUT";
        if(!isEnabled())emit signalAbortPrint("Timed out while attempting to turn on projector.  Check Projector's Power Cord and RS-232 cable.");
        break;
    case B9PrinterStatus::PS_FAIL:
        ui->pushButtonProjPower->setEnabled(false);
        sText = "FAIL";
        if(!isEnabled())emit signalAbortPrint("Lost Communications with Projector.  Possible Causes:  Manually powered off, Power Failure, Cord Disconnected or Projector Lamp Failure");
        break;
    case B9PrinterStatus::PS_UNKNOWN:
        ui->pushButtonProjPower->setEnabled(true);
    default:
        sText = "UNKNOWN";
        break;
    }

    // Update the power button state
    if(isTurningOn){
        pProjector->hide(); //cyp
        activateWindow();
        ui->pushButtonProjPower->setChecked(true);
        ui->pushButtonProjPower->setText("OFF");
    }
    else if(pPrinterComm->isProjectorPowerCmdOn()){
        if(!isTurningOn)
            pProjector->show();//打开投影仪的时候不需要显示
        activateWindow();
        ui->pushButtonProjPower->setChecked(true);
        ui->pushButtonProjPower->setText("OFF");
    }
    else{
        pProjector->hide();
        activateWindow();
        ui->pushButtonProjPower->setChecked(false);
        ui->pushButtonProjPower->setText("ON");
    }

    if(!isEnabled())emit sendStatusMsg("WINSUN - 投影仪 状态: "+sText);
    if(!isEnabled())emit updateProjectorStatus(sText);
    if(!isEnabled())emit updateProjector(pPrinterComm->getProjectorStatus());

//    ui->lineEditProjState->setText(sText);
    sText = "UNKNOWN";
    int iLH = pPrinterComm->getLampHrs();
    if(iLH >= 0 && (pPrinterComm->getProjectorStatus()==B9PrinterStatus::PS_ON||
                    pPrinterComm->getProjectorStatus()==B9PrinterStatus::PS_WARMING||
                    pPrinterComm->getProjectorStatus()==B9PrinterStatus::PS_COOLING))sText = QString::number(iLH);
//    ui->lineEditLampHrs->setText(sText);
}

void Terminal::onBC_ProjStatusFAIL()
{
    onBC_ProjStatusChanged();
    on_pushButtonProjPower_toggled(pPrinterComm->isProjectorPowerCmdOn());
}

void Terminal::onBC_PU(int iPU){
//    double dPU = (double)iPU/100000.0;
//    ui->lineEditPUinMicrons->setText(QString::number(dPU,'g',8));
}

void Terminal::onBC_UpperZLimPU(int iUpZLimPU){
//    double dZUpLimMM = (iUpZLimPU * pPrinterComm->getPU())/100000.0;
//    ui->lineEditUpperZLimMM->setText(QString::number(dZUpLimMM,'g',8));
//    ui->lineEditUpperZLimInches->setText(QString::number(dZUpLimMM/25.4,'g',8));
//    ui->lineEditUpperZLimPU->setText(QString::number(iUpZLimPU,'g',8));
    m_UpperZLimPU = iUpZLimPU;
}

void Terminal::onMotionResetTimeout(){
    this->setEnabled(true);
    m_pResetTimer->stop();
    QMessageBox msg;
    msg.setText("ERROR: TIMEOUT attempting to locate home position.  Check connections.");
    if(isEnabled())msg.exec();
}

void Terminal::onMotionResetComplete()
{
//    ui->groupBoxMain->setEnabled(true);
    this->setEnabled(true);
//    if(pPrinterComm->getHomeStatus()==B9PrinterStatus::HS_FOUND) ui->lineEditNeedsInit->setText("No");
//    else if(pPrinterComm->getHomeStatus()==B9PrinterStatus::HS_UNKNOWN) ui->lineEditNeedsInit->setText("Yes");
//    else ui->lineEditNeedsInit->setText("Seeking");
//    ui->lineEditZDiff->setText(QString::number(pPrinterComm->getLastHomeDiff()).toAscii());
    m_pResetTimer->stop();

    // Check for post reset go to fill command
    if(m_iFillLevel>=0){
        pPrinterComm->SendCmd("G"+QString::number(m_iFillLevel));
        m_iFillLevel=-1;
    }
    emit HomeFound();
}

void Terminal::onBC_CurrentZPosInPU(int iCurZPU){
    double dZPosMM = (iCurZPU * pPrinterComm->getPU())/100000.0;
    ui->lineEditCurZPosInMM->setText(QString::number(dZPosMM,'g',8));
//    ui->lineEditCurZPosInInches->setText(QString::number(dZPosMM/25.4,'g',8));
    ui->lineEditCurZPosInPU->setText(QString::number(iCurZPU,'g',8));
    emit ZMotionComplete();
}

void Terminal::onBC_HalfLife(int iHL){
//    ui->lineEditHalfLife->setText(QString::number(iHL));
}

void Terminal::onBC_NativeX(int iNX){
//    ui->lineEditNativeX->setText(QString::number(iNX));
}

void Terminal::onBC_NativeY(int iNY){
//    ui->lineEditNativeY->setText(QString::number(iNY));
    if(pProjector == NULL)emit onScreenCountChanged();
}


void Terminal::onBC_XYPixelSize(int iPS){
//    int i=2;
//    if(iPS==50)i=0;
//    else if(iPS==75)i=1;
//    ui->comboBoxXPPixelSize->setCurrentIndex(i);
}


void Terminal::setTgtAltitudePU(int iTgtPU)
{
    double dTgtMM = (iTgtPU * pPrinterComm->getPU())/100000.0;
    ui->lineEditTgtZPU->setText(QString::number(iTgtPU));
    ui->lineEditTgtZMM->setText(QString::number(dTgtMM,'g',8));
//    ui->lineEditTgtZInches->setText(QString::number(dTgtMM/25.4,'g',8));
}

void Terminal::setTgtAltitudeMM(double dTgtMM){
    double dPU = (double)pPrinterComm->getPU()/100000.0;
    setTgtAltitudePU((int)(dTgtMM/dPU));
}

void Terminal::setTgtAltitudeIN(double dTgtIN){
    setTgtAltitudeMM(dTgtIN*25.4);
}

void Terminal::onMotionVatTimeout(){
    m_pVatTimer->stop();
    on_pushButtonStop_clicked(); // STOP!
    QMessageBox msg;
    msg.setText("Vat Timed out");
    if(isEnabled())msg.exec();
//    ui->groupBoxVAT->setEnabled(true);
}

void Terminal::onBC_CurrentVatPercentOpen(int iPO){
    m_pVatTimer->stop();
    int iVPO = iPO;
    if (iVPO>-3 && iVPO<4)iVPO=0;
    if (iVPO>97 && iVPO<104)iVPO=100;
//    ui->spinBoxVatPercentOpen->setValue(iVPO);
//    ui->groupBoxVAT->setEnabled(true);
}

void Terminal::onBC_PrintReleaseCycleFinished()
{
    m_pPReleaseCycleTimer->stop();
    ui->lineEditCycleStatus->setText("Cycle Complete.");
    ui->pushButtonPrintBase->setEnabled(true);
    ui->pushButtonPrintNext->setEnabled(true);
    ui->pushButtonPrintFinal->setEnabled(true);
    emit PrintReleaseCycleFinished();
}

void Terminal::onReleaseCycleTimeout()
{
    m_pPReleaseCycleTimer->stop();
    if(true){  // Set to true if we wish to abort due to the timeout.
        qDebug()<<"Release Cycle Timeout.  Possible reasons: Power Loss, Jammed Mechanism.";
        on_pushButtonStop_clicked(); // STOP!
        ui->lineEditCycleStatus->setText("ERROR: TimeOut");
        ui->pushButtonPrintBase->setEnabled(true);
        ui->pushButtonPrintNext->setEnabled(true);
        ui->pushButtonPrintFinal->setEnabled(true);
        if(!isEnabled())emit signalAbortPrint("ERROR: Cycle Timed Out.  Possible reasons: Power Loss, Jammed Mechanism.");
        return;
    }
    else {
        qDebug()<<"Release Cycle Timeout.  Possible reasons: Power Loss, Jammed Mechanism. IGNORED";
        qDebug()<<"Serial Port Last Error:  "<<pPrinterComm->errorString();
    }
}

void Terminal::SetCycleParameters(){//cyp
    int iD, iE, iJ, iK, iL, iW, iX;
//    if(pSettings->m_dBTClearInMM*100000/(double)pPrinterComm->getPU()>(double)ui->lineEditTgtZPU->text().toInt()){
        iD = (int)(pSettings->m_dBreatheClosed1*1000.0); // Breathe delay time
        iE = (int)(pSettings->m_dSettleOpen1*1000.0); // Settle delay time
        iJ = (int)(pSettings->m_dOverLift1*100000.0/(double)pPrinterComm->getPU()); // Overlift Raise Gap coverted to PU
        iK = pSettings->m_iRSpd1;  // Raise Speed
        iL = pSettings->m_iLSpd1;  // Lower Speed
        iW = pSettings->m_iOpenSpd1;  // Vat open speed
        iX = pSettings->m_iCloseSpd1; // Vat close speed
//    }
//    else{
//        iD = (int)(pSettings->m_dBreatheClosed2*1000.0); // Breathe delay time
//        iE = (int)(pSettings->m_dSettleOpen2*1000.0); // Settle delay time
//        iJ = (int)(pSettings->m_dOverLift2*100000.0/(double)pPrinterComm->getPU()); // Overlift Raise Gap coverted to PU
//        iK = pSettings->m_iRSpd2;  // Raise Speed
//        iL = pSettings->m_iLSpd2;  // Lower Speed
//        iW = pSettings->m_iOpenSpd2;  // Vat open speed
//        iX = pSettings->m_iCloseSpd2; // Vat close speed
//    }
    if(iD!=m_iD){pPrinterComm->SendCmd("D"+QString::number(iD)); m_iD = iD;}
    if(iE!=m_iE){pPrinterComm->SendCmd("E"+QString::number(iE)); m_iE = iE;}
    if(iJ!=m_iJ){pPrinterComm->SendCmd("J"+QString::number(iJ)); m_iJ = iJ;}
    if(iK!=m_iK){pPrinterComm->SendCmd("K"+QString::number(iK)); m_iK = iK;}
    if(iL!=m_iL){pPrinterComm->SendCmd("L"+QString::number(iL)); m_iL = iL;}
    if(iW!=m_iW){pPrinterComm->SendCmd("W"+QString::number(iW)); m_iW = iW;}
    if(iX!=m_iX){pPrinterComm->SendCmd("X"+QString::number(iX)); m_iX = iX;}
}

void Terminal::dlgEditMatCat()
{

}

void Terminal::dlgEditPrinterCycleSettings()
{
    CyclePrintSetting dlgPCycles(pSettings,0);
    dlgPCycles.exec();
}

void Terminal::rcBasePrint(double dBaseMM)
{
    pPrinterComm->m_bIsLog = true;//cyp
    setTgtAltitudeMM(dBaseMM);
    on_pushButtonPrintBase_clicked();
}

void Terminal::rcNextPrint(double dNextMM)
{
    setTgtAltitudeMM(dNextMM);
    on_pushButtonPrintNext_clicked();
}

void Terminal::rcFinishPrint(double dDeltaMM)
{
    // Calculates final position based on current + dDeltaMM
    int newPos = dDeltaMM*100000.0/(double)pPrinterComm->getPU();
    newPos += ui->lineEditCurZPosInPU->text().toInt();
    int curPos = ui->lineEditCurZPosInPU->text().toInt();
    int upperLim = m_UpperZLimPU;//ui->lineEditUpperZLimPU->text().toInt();

    if(curPos >= upperLim)
        newPos = curPos;
    else if(newPos > upperLim)
        newPos = upperLim;

    setTgtAltitudePU(newPos);
    on_pushButtonPrintFinal_clicked();
    pPrinterComm->m_bIsLog = false;//cyp
}


void Terminal::rcCloseVat()
{
//    ui->groupBoxVAT->setEnabled(false);
    m_pVatTimer->start(3000); //should never take that long, even at slow speed
    pPrinterComm->SendCmd("V0");
}

void Terminal::rcSetProjMessage(QString sMsg)
{
    if(pProjector==NULL)return;
    // Pass along the message for the projector screen
    pProjector->setStatusMsg("WINSUN  -  "+sMsg);
}

void Terminal::on_pushButtonCmdReset_clicked()
{
    int iTimeoutEstimate = 80000; // 80 seconds (should never take longer than 75 secs from upper limit)

    this->setEnabled(false);
//    ui->lineEditNeedsInit->setText("Seeking");
    // Remote activation of Reset (Find Home) Motion
    m_pResetTimer->start(iTimeoutEstimate);
    pPrinterComm->SendCmd("R");
    resetLastSentCycleSettings();

}

void Terminal::on_pushButtonProjPower_clicked()
{
    if(ui->pushButtonProjPower->text() == "ON"){
        rcSendCmd("P1");//open
        ui->pushButtonProjPower->setText("OFF");
    }
    else{
        rcSendCmd("P0");//close
        ui->pushButtonProjPower->setText("ON");
    }

}

void Terminal::on_pushButtonBoard_clicked()
{
    if(ui->pushButtonBoard->text() == "ON"){
        rcSendCmd("#0");//open
        ui->pushButtonBoard->setText("OFF");
    }
    else{
        rcSendCmd("#1");//close
        ui->pushButtonBoard->setText("ON");
    }
}

void Terminal::on_pushButtonCycleSettings_clicked()
{
    pSettings->updateValues();

}

void Terminal::on_pushButtonPrintBase_clicked()
{
    ui->lineEditCycleStatus->setText("Moving to Base...");
    ui->pushButtonPrintBase->setEnabled(false);
    ui->pushButtonPrintNext->setEnabled(false);
    ui->pushButtonPrintFinal->setEnabled(false);
    resetLastSentCycleSettings();
    SetCycleParameters();
    int iTimeout = getEstBaseCycleTime(ui->lineEditCurZPosInPU->text().toInt(), ui->lineEditTgtZPU->text().toInt());
    pPrinterComm->SendCmd("B"+ui->lineEditTgtZPU->text());
    m_pPReleaseCycleTimer->start(iTimeout * 2.0); // Timeout after 200% of estimated time required

}

void Terminal::on_pushButtonPrintNext_clicked()
{
    ui->lineEditCycleStatus->setText("Cycling to Next...");
    ui->pushButtonPrintBase->setEnabled(false);
    ui->pushButtonPrintNext->setEnabled(false);
    ui->pushButtonPrintFinal->setEnabled(false);

    SetCycleParameters();
    int iTimeout = getEstNextCycleTime(ui->lineEditCurZPosInPU->text().toInt(), ui->lineEditTgtZPU->text().toInt());
    pPrinterComm->SendCmd("N"+ui->lineEditTgtZPU->text());
    m_pPReleaseCycleTimer->start(iTimeout * 2.0); // Timeout after 200% of estimated time required

}

void Terminal::on_pushButtonPrintFinal_clicked()
{
    rcProjectorPwr(false);  // command projector OFF
    ui->lineEditCycleStatus->setText("Final Release...");
    ui->pushButtonPrintBase->setEnabled(false);
    ui->pushButtonPrintNext->setEnabled(false);
    ui->pushButtonPrintFinal->setEnabled(false);
    SetCycleParameters();
    int iTimeout = getEstFinalCycleTime(ui->lineEditCurZPosInPU->text().toInt(), ui->lineEditTgtZPU->text().toInt());
    pPrinterComm->SendCmd("F"+ui->lineEditTgtZPU->text());
    m_pPReleaseCycleTimer->start(iTimeout * 2.0); // Timeout after 200% of estimated time required

}

void Terminal::on_pushButtonStop_clicked()
{
    m_pPReleaseCycleTimer->stop();
    m_pVatTimer->stop();
    pPrinterComm->SendCmd("S");
    ui->lineEditCycleStatus->setText("停止");//Cycle Stopped.
    ui->pushButtonPrintBase->setEnabled(true);
    ui->pushButtonPrintNext->setEnabled(true);
    ui->pushButtonPrintFinal->setEnabled(true);
//    ui->groupBoxVAT->setEnabled(true);
    resetLastSentCycleSettings();

}

void Terminal::on_pushButtonProjPower_toggled(bool checked)
{
//    ui->pushButtonProjPower->setChecked(checked);
//    if(checked)
//        ui->pushButtonProjPower->setText("ON");
//    else
//        ui->pushButtonProjPower->setText("OFF");
//    pPrinterComm->cmdProjectorPowerOn(checked);
//    emit(setProjectorPowerCmd(checked));

//    // if m_bPrimaryScreen is true, we need to show it before turning on projector!
////    if(m_bPrimaryScreen) onScreenCountChanged();//cyp
//    emit sendStatusMsg("B9Creator - Projector status: TURN ON");

//    // We always close the vat when powering up
//    if(checked){
//        emit onBC_CurrentVatPercentOpen(0);
////        emit on_spinBoxVatPercentOpen_editingFinished();
//    }

}


void Terminal::makeProjectorConnections()
{
    // should be called any time we create a new projector object
    if(pProjector==NULL)return;
    connect(pProjector, SIGNAL(keyReleased(int)),this, SLOT(getKey(int)));
    connect(this, SIGNAL(sendStatusMsg(QString)),pProjector, SLOT(setStatusMsg(QString)));
    connect(this, SIGNAL(sendGrid(bool)),pProjector, SLOT(setShowGrid(bool)));
    connect(this, SIGNAL(sendCPJ(CrushedPrintJob*)),pProjector, SLOT(setCPJ(CrushedPrintJob*)));
    connect(this, SIGNAL(sendXoff(int)),pProjector, SLOT(setXoff(int)));
    connect(this, SIGNAL(sendYoff(int)),pProjector, SLOT(setYoff(int)));
}

void Terminal::getKey(int iKey)
{
    if(!m_bPrimaryScreen)return; // Ignore keystrokes from the print window unless we're using the primary monitor
    if(isVisible()&&isEnabled())
    {
        // We must be "calibrating"  If we get any keypress we should close the projector window
        if(pProjector!=NULL) pProjector->hide();
    }
    switch(iKey){
    case 112:		// 'p' Pause/Resume
        emit pausePrint();
        break;
    case 97://cyp 65:        // Capital 'A' to abort
        if(!isEnabled()){
            m_pPReleaseCycleTimer->stop();
            emit signalAbortPrint("User Directed Abort.");
        }
        break;
    default:
        break;
    }
}
void Terminal::on_lineEditCommand_returnPressed()
{
    pPrinterComm->SendCmd(ui->lineEditCommand->text());
    ui->lineEditCommand->clear();
}

void Terminal::on_lineEditCurZPosInPU_returnPressed()
{
    int iValue=ui->lineEditCurZPosInPU->text().toInt();
    if(QString::number(iValue)!=ui->lineEditCurZPosInPU->text()|| iValue<0 || iValue >32000){
        // Bad Value, just return
        ui->lineEditCurZPosInPU->setText("Bad Value");
        return;
    }
    pPrinterComm->SendCmd("G"+QString::number(iValue));
    ui->lineEditCurZPosInPU->setText("In Motion...");
    ui->lineEditCurZPosInMM->setText("In Motion...");
//    ui->lineEditCurZPosInInches->setText("In Motion...");
}

void Terminal::on_lineEditCurZPosInMM_returnPressed()
{
    double dPU = (double)pPrinterComm->getPU()/100000.0;
    double dValue=ui->lineEditCurZPosInMM->text().toDouble();
    if((dValue==0 && ui->lineEditCurZPosInMM->text().length()>1 )||dValue<0 || dValue >203.2){
        // Bad Value, just return
        ui->lineEditCurZPosInMM->setText("Bad Value");
        return;
    }

    pPrinterComm->SendCmd("G"+QString::number((int)(dValue/dPU)));
    ui->lineEditCurZPosInPU->setText("In Motion...");
    ui->lineEditCurZPosInMM->setText("In Motion...");
//    ui->lineEditCurZPosInInches->setText("In Motion...");
}

void Terminal::on_lineEditTgtZPU_editingFinished()
{
    int iValue=ui->lineEditTgtZPU->text().toInt();
    if(QString::number(iValue)!=ui->lineEditTgtZPU->text()||
            iValue<0 || iValue >31497){
        QMessageBox::information(this, tr("Target Level (Z steps) Out of Range"),
                                       tr("Please enter an integer value between 0-31497.\n"
                                          "This will be the altitude for the next layer.\n"),
                                       QMessageBox::Ok);
        iValue = 0;
        ui->lineEditTgtZPU->setText(QString::number(iValue));
        ui->lineEditTgtZPU->setFocus();
        ui->lineEditTgtZPU->selectAll();
    }
    setTgtAltitudePU(iValue);
}

void Terminal::on_lineEditTgtZMM_editingFinished()
{
    double dValue=ui->lineEditTgtZMM->text().toDouble();
    double dTest = QString::number(dValue).toDouble();
    if((dTest==0 && ui->lineEditTgtZMM->text().length()>2)|| dTest!=ui->lineEditTgtZMM->text().toDouble()||
            dValue<0 || dValue >200.00595){
        QMessageBox::information(this, tr("Target Level (Inches) Out of Range"),
                     tr("Please enter an integer value between 0-7.87425.\n"
                     "This will be the altitude for the next layer.\n"),QMessageBox::Ok);
        dValue = 0;
        ui->lineEditTgtZMM->setText(QString::number(dValue));
        ui->lineEditTgtZMM->setFocus();
        ui->lineEditTgtZMM->selectAll();
        return;
    }
    setTgtAltitudeMM(dValue);
}

void Terminal::warnSingleMonitor(){
    if(m_bPrimaryScreen && m_bNeedsWarned){
        m_bNeedsWarned = false;
        QMessageBox msg;
        msg.setWindowTitle("Projector Connection?");
        msg.setText("WARNING:  The printer's projector is not connected to a secondary video output.  Please check that all connections (VGA or HDMI) and system display settings are correct.  Disregard this message if your system has only one video output and will utilize a splitter to provide video output to both monitor and Projector.");
//        if(isEnabled())msg.exec();//cyp
    }
}

void Terminal::setEnabledWarned(){
    if(isHidden())return;
    if(!m_bWaiverPresented||m_bWaiverAccepted==false){
        // Present Waiver
        m_bWaiverPresented = true;
        m_bWaiverAccepted = false;
        if(!m_bWavierActive){
            m_bWavierActive = true;
//            int ret = QMessageBox::information(this, tr("Enable Terminal Control?"),
//                                           tr("Warning: Manual operation can damage the VAT coating.\n"
//                                              "If your VAT is installed and empty of resin care must be\n"
//                                              "taken to ensure it is not damaged.  Operation is only safe\n"
//                                              "with either the VAT removed, or the Sweeper and Build Table removed.\n"
//                                              "The purpose of this utility is to assist in troubleshooting.\n"
//                                              "Its use is not required during normal printing operations.\n"
//                                              "Do you want to enable manual control?"),
//                                           QMessageBox::Yes | QMessageBox::No
//                                           | QMessageBox::Cancel);

//            if(ret==QMessageBox::Cancel){m_bWavierActive = false;m_bWaiverPresented=false;hide();return;}
//            else if(ret==QMessageBox::Yes)m_bWaiverAccepted=true;

            m_bWaiverAccepted=true;//cyp
            warnSingleMonitor();
            m_bWavierActive = false;
        }
    }
//    ui->groupBoxMain->setEnabled(m_bWaiverAccepted&&pPrinterComm->isConnected()&&ui->lineEditNeedsInit->text()!="Seeking");
    this->setEnabled(m_bWaiverAccepted&&pPrinterComm->isConnected()/*&&ui->lineEditNeedsInit->text()!="Seeking"*/);
}

void Terminal::hideEvent(QHideEvent *event)
{
    emit eventHiding();
    event->accept();
}

//void Terminal::on_spinBoxVatPercentOpen_editingFinished()
//{
//    if(m_pVatTimer->isActive()) return;
//    m_pVatTimer->start(3000); //should never take that long, even at slow speed
//    int iValue = ui->spinBoxVatPercentOpen->value();
////    ui->groupBoxVAT->setEnabled(false);
//    pPrinterComm->SendCmd("V"+QString::number(iValue));
//}
