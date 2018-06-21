#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "b9layout/b9layoutprojectdata.h"
#include "crushbitmap.h"
#include "b9layout/slicecontext.h"
#include "b9layout/sliceset.h"
#include "b9layout/slice.h"
#include "loadingbar.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVector3D>
#include <QGLWidget>
#include <QDebug>
#include "b9layout/SlcExporter.h"
#include "b9layout/modeldata.h"
#include "b9layout/b9modelinstance.h"
#include "b9supportstructure.h"
#include "OS_Wrapper_Functions.h"
#include "b9modelwriter.h"

#define BASERADIUS 6.0
#define TOPRADIUS 0.5
#define MIDRADIUS 0.5
#define BOTTOMRADIUS 3.0
#define BASELENGTH 1.0
#define TOPLENGTH 1.0
//#define MIDLENGTH
#define BOTTOMLENGTH 0.5
#define BASESHAP "Cylinder"
#define TOPSHAPE "Cone 25%"
#define MIDHAPE "Cylinder"
#define BOTTOMSHAPE "Cylinder"
#define TOPPENETRATION 0
#define BOTTOMPENETRATION 0
#define TOPANGLE 0.2
#define BOTTOMANGLE 0.2

void PSupportSettings::updateValues()
{
    SupportSetting dlg(this);
    dlg.exec();
}

void PSupportSettings::loadSettings()
{

//    QSettings settings("supportsettings");
    QSettings appSettings;
    appSettings.beginGroup("USERSUPPORTPARAMS");
//    appSettings.beginGroup("SUPPORT_BASE");
    m_dBaseRadius = appSettings.value("BaseRadius",BASERADIUS).toDouble();
    m_dBaseLength = appSettings.value("BaseLength",BASELENGTH).toDouble();
    m_baseAttachShape = appSettings.value("BaseAttachShape",BASESHAP).toString();
//    appappSettings.endGroup();
//    appSettings.beginGroup("SUPPORT_TOP");
    m_topAttachShape = appSettings.value("TopAttachShape",TOPSHAPE).toString();
    m_dTopRadius = appSettings.value("TopRadius",TOPRADIUS).toDouble();
    m_dTopLength = appSettings.value("TopLength",TOPLENGTH).toDouble();
    m_dTopPenetration = appSettings.value("TopPenetration",TOPPENETRATION).toDouble();
    m_dTopAngleFactor = appSettings.value("TopAngleFactor",TOPANGLE).toDouble();
//    appSettings.endGroup();
//    appSettings.beginGroup("SUPPORT_MID");
    m_midAttachShape = appSettings.value("MidAttachShape",MIDHAPE).toString();
    m_dMidRadius = appSettings.value("MidRadius",MIDRADIUS).toDouble();
//    m_dMidLength = appSettings.value("MidLength",MIDLENGTH).toDouble();
//    appSettings.endGroup();
//    appSettings.beginGroup("SUPPORT_BOTTOM");
    m_bottomAttachShape = appSettings.value("BottomAttachShape",BOTTOMSHAPE).toString();
    m_dBottomLength = appSettings.value("BottomLength",BOTTOMLENGTH).toDouble();
    m_dBottomPenetration = appSettings.value("BottomPenetration",BOTTOMPENETRATION).toDouble();
    m_dBottomAngleFactor = appSettings.value("BottomAngleFactor",BOTTOMANGLE).toDouble();
    m_dBottomRadius = appSettings.value("BottomRadius",BOTTOMRADIUS).toDouble();

    appSettings.endGroup();
//    m_baseAttachShape = "Cylinder";
//    m_topAttachShape = "Cone 25%";
//    m_midAttachShape = "Cylinder";
//    m_bottomAttachShape = "Cylinder";


}

void PSupportSettings::saveSettings()
{
//    QSettings settings("supportsettings");

    QSettings appSettings;
    appSettings.beginGroup("USERSUPPORTPARAMS");
            appSettings.setValue("BaseRadius",m_dBaseRadius);
            appSettings.setValue("BaseLength",m_dBaseLength);

        appSettings.setValue("TopLength",m_dTopLength);
        appSettings.setValue("TopRadius",m_dTopRadius);
        appSettings.setValue("TopPenetration",m_dTopPenetration);
        appSettings.setValue("TopAngleFactor", m_dTopAngleFactor);

        appSettings.setValue("MidRadius",m_dMidRadius);

        appSettings.setValue("BottomLength",m_dBottomLength);
        appSettings.setValue("BottomRadius",m_dBottomRadius);
        appSettings.setValue("BottomPenetration",m_dBottomPenetration);
        appSettings.setValue("BottomAngleFactor", m_dBottomAngleFactor);


        appSettings.setValue("BaseAttachShape",m_baseAttachShape);
        appSettings.setValue("TopAttachShape",m_topAttachShape);
        appSettings.setValue("MidAttachShape",m_midAttachShape);
        appSettings.setValue("BottomAttachShape",m_bottomAttachShape);

        appSettings.endGroup();
}

void PSupportSettings::setFactorySettings()
{
    m_baseAttachShape = "Cylinder";
    m_dBaseRadius = BASERADIUS;
    m_dBaseLength = BASELENGTH;

    m_topAttachShape = "Cone 25%";
    m_dTopLength = TOPLENGTH;
    m_dTopRadius = TOPRADIUS;
    m_dTopPenetration = TOPPENETRATION;
    m_dTopAngleFactor = TOPANGLE;

    m_midAttachShape = "Cylinder";
    m_dMidRadius = MIDRADIUS;

    m_bottomAttachShape = "Cylinder";
    m_dBottomRadius = BOTTOMRADIUS;
    m_dBottomLength = BOTTOMLENGTH;
    m_dBottomPenetration = BOTTOMPENETRATION;
    m_dBottomAngleFactor = BOTTOMANGLE;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)/*,
    ui(new Ui::MainWindow)*/
{
    ui.setupUi(this);

    pixmap = NULL;

    pSettings = new PSupportSettings;
    //Initialize project data
    project = new B9LayoutProjectData();
    project->pMain = this;

    //子窗口
    bs = new BuildSpace();
    t = new Terminal();
    pw = NULL;
    pPrint = new Print(t);
    pSliceView=NULL;
    pw = new PrintWindow(this,t);
    connect (pw, SIGNAL(accepted()),this,SLOT(doPrint()));


    //Create the worldview and attach it to the window    pixmap = new QPixmap(100,100);

    pWorldView = new WorldView(NULL,this);
    ui.WorldViewContext->addWidget(pWorldView);
    pModelList = new ModelList(this,pWorldView);
    pModelList->setGeometry(5,5,337,335);
    pWorldView->show();
    SetToolModelSelect();//start off with pointer tool

    //support editing
    currInstanceInSupportMode = NULL;
    useContourAid = true;
    hideSupports = true;

    //slicing
    cancelslicing = false;

    //import premade stls for support structures
    B9SupportStructure::ImportAttachmentDataFromStls();
//        B9SupportStructure::FillRegistryDefaults();//if needed

    //build the interface initially - different than update interface..

    m_pCPJ = new CrushedPrintJob;

    this->geometry();

//    pixmap->load(":/B9Edit/icons/scale3.png");
//    icon = new QIcon(*pixmap);
//    pButtonScale = new QPushButton(pWorldView);
//    pButtonScale->setGeometry(230,500,100,100);
//    pButtonScale->setFixedSize(100,100);
//    pButtonScale->setStyleSheet("background-color: rgba(70, 130, 180, 100%);");
//    pButtonScale->setIcon(*icon);
////    pButtonScale->setText("Scale");
//    pButtonScale->setIconSize(QSize(100,100));
//    pButtonScale->setFlat(true);
//    pButtonScale->setFocusPolicy(Qt::NoFocus);
//    pButtonScale->setToolTip("缩放");
//    connect(pButtonScale,SIGNAL(clicked()),this,SLOT(SetToolModelScale()));


    InitInterface();

    New();



}

MainWindow::~MainWindow()
{
    if(bs)
        delete bs;
    if(t)
        delete t;
    if(pw)
        delete pw;
    if(pPrint)
        delete pPrint;
    if(pSliceView)
        delete pSliceView;
    if(pSupport)
        delete pSupport;
    if(pModelList)
        delete pModelList;
    if(pixmap)
        delete pixmap;
    if(icon)
        delete icon;
    if(pButtonRotate)
        delete pButtonRotate;
    if(pButtonMove)
        delete pButtonMove;
    if(pButtonScale)
        delete pButtonScale;
    if(pWorldView)
        delete pWorldView;
    if(project)
        delete project;
    if(m_pCPJ)
        delete m_pCPJ;
    if(pSettings)
        delete pSettings;
    if(currInstanceInSupportMode)
        delete currInstanceInSupportMode;

    if(pActionSupport)
        delete pActionSupport;
}

void MainWindow::InitInterface()
{
    pActionSupport = new QAction(ui.SupportToolBar);
    pActionSupport->setCheckable(true);
    pActionSupport->setChecked(false);
    pActionSupport->setText("Support");
    pActionSupport->setToolTip("支撑");
    connect(pActionSupport,SIGNAL(toggled(bool)),this,SLOT(SetSupportMode(bool)));

    //toolbar items
    ui.fileToolBar->addAction(ui.actionNew_Project);
    ui.fileToolBar->addAction(ui.actionOpen_Project);
    ui.fileToolBar->addAction(ui.actionSave);
    ui.fileToolBar->addAction(pActionSupport);
    ui.fileToolBar->addAction(ui.slice);
    ui.fileToolBar->addAction(ui.actionPrint);

//    ui.editToolBar->addAction(ui.add_model);
//    ui.editToolBar->addAction(ui.delete_model);
    ui.editToolBar->addAction(ui.actionRotate);
    ui.editToolBar->addAction(ui.actionMove);
    ui.editToolBar->addAction(ui.actionScale);
    ui.editToolBar->addAction(ui.actionDrop_To_Floor);

//    ui.viewToolBar->addAction(ui.center);
    ui.viewToolBar->addAction(ui.actionTop_View);
    ui.viewToolBar->addAction(ui.actionBottom_View);
    ui.viewToolBar->addAction(ui.actionLeft_View);
    ui.viewToolBar->addAction(ui.actionRight_View);
    ui.viewToolBar->addAction(ui.actionFront_View);
    ui.viewToolBar->addAction(ui.actionBack_View);

    pButtonRotate = new QPushButton(pWorldView);
    pButtonRotate->setGeometry(10,500,100,100);
    pButtonRotate->setStyleSheet("border-image: url(:/B9Edit/icons/rotate3.png);background-color: rgba(70, 130, 180, 100%);");
    pButtonRotate->setFlat(true);
    pButtonRotate->setFocusPolicy(Qt::NoFocus);
    pButtonRotate->setToolTip("旋转");
    connect(pButtonRotate,SIGNAL(clicked()),this,SLOT(SetToolModelOrientate()));

    pButtonMove = new QPushButton(pWorldView);
    pButtonMove->setGeometry(120,500,100,100);
    pButtonMove->setStyleSheet("border-image: url(:/B9Edit/icons/crosshair.png);background-color: rgba(70, 130, 180, 100%);");
    pButtonMove->setFlat(true);
    pButtonMove->setFocusPolicy(Qt::NoFocus);
    pButtonMove->setToolTip("平移");
    connect(pButtonMove,SIGNAL(clicked()),this,SLOT(SetToolModelMove()));

    pixmap = new QPixmap;//(100,100);
    pixmap->load(":/B9Edit/icons/scale3.png");
    icon = new QIcon(*pixmap);

    pButtonScale = new QPushButton(pWorldView);
    pButtonScale->setFlat(true);
    pButtonScale->setGeometry(230,500,100,100);
//    pButtonScale->setFixedSize(100,100);
    pButtonScale->setStyleSheet("background-color: rgba(70, 130, 180, 100%);");
    pButtonScale->setIcon(*icon);
//    pButtonScale->setText("Scale");
    pButtonScale->setIconSize(QSize(100,100));
//    pButtonScale->setFocusPolicy(Qt::NoFocus);
    pButtonScale->setToolTip("缩放");
    connect(pButtonScale,SIGNAL(clicked()),this,SLOT(SetToolModelScale()));


    pButtonRotate->hide();
    pButtonMove->hide();
    pButtonScale->hide();

    pSupport = new SupportParameter(pWorldView,this);
    pSupport->setGeometry(5,345,337,240);
    pSupport->hide();

    //信号和槽
    //file
    connect(ui.actionNew_Project,SIGNAL(triggered()),this,SLOT(New()));
    connect(ui.actionOpen_Project,SIGNAL(triggered()),this,SLOT(Open()));
    connect(ui.actionSave,SIGNAL(triggered()),this,SLOT(Save()));
    connect(ui.actionSave_As,SIGNAL(triggered()),this,SLOT(SaveAs()));
    connect(ui.actionAdd_Model,SIGNAL(triggered()),this,SLOT(AddModel()));
    connect(ui.actionDelete,SIGNAL(triggered()),this,SLOT(DeleteSelection()));
    connect(ui.actionDuplicate,SIGNAL(triggered()),this,SLOT(DuplicateSelection()));
    connect(ui.actionPrint,SIGNAL(triggered()),this,SLOT(showPrintWindow()));

    //edit
    connect(ui.actionMove,SIGNAL(triggered()),this,SLOT(SetToolModelMove()));
    connect(ui.actionRotate,SIGNAL(triggered()),this,SLOT(SetToolModelOrientate()));
    connect(ui.actionScale,SIGNAL(triggered()),this,SLOT(SetToolModelScale()));
    connect(ui.actionDrop_To_Floor,SIGNAL(triggered()),this,SLOT(DropSelectionToFloor()));

    //append
    connect(ui.slice,SIGNAL(triggered()),this,SLOT(doSlice()));
    connect(ui.showSlices,SIGNAL(triggered()),this,SLOT(showSliceWindow()));
    connect(ui.materialCatalog,SIGNAL(triggered()),this,SLOT(showCatalog()));
    connect(ui.buildSpace,SIGNAL(triggered()),bs,SLOT(show()));

    //view
//    QObject::connect(ui.actionCenter_View,SIGNAL(activated()),pWorldView,SLOT(CenterView()));
    QObject::connect(ui.actionTop_View,SIGNAL(activated()),pWorldView,SLOT(TopView()));
    QObject::connect(ui.actionRight_View,SIGNAL(activated()),pWorldView,SLOT(RightView()));
    QObject::connect(ui.actionFront_View,SIGNAL(activated()),pWorldView,SLOT(FrontView()));
    QObject::connect(ui.actionBack_View,SIGNAL(activated()),pWorldView,SLOT(BackView()));
    QObject::connect(ui.actionLeft_View,SIGNAL(activated()),pWorldView,SLOT(LeftView()));
    QObject::connect(ui.actionBottom_View,SIGNAL(activated()),pWorldView,SLOT(BottomView()));

}

void MainWindow::RemoveAllSupports()
{
    currInstanceInSupportMode->RemoveAllSupports();
}


void MainWindow::doSlice()
{
    project->SetPixelThickness(pModelList->getThickness());
    SliceWorld();

}



void MainWindow::showSliceWindow()
{
    QFileDialog dialog(0);
    QSettings settings("Dir");
    QString openFile = dialog.getOpenFileName(this,"Select WINSUN Slice File", settings.value("WorkingDir").toString(), tr("WINSUN File (*.ws)"));//cyp .b9j
    if(openFile.isEmpty()) return;
    settings.setValue("WorkingDir", QFileInfo(openFile).absolutePath());


    cPJ.clearAll();

    QFile file(openFile);
    if(!cPJ.loadCPJ(&file)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("WINSUN");
        msgBox.setText("Error Loading File.  Unknown Version?");//
        msgBox.setToolTip("文件加载错误。未知版本？");
        msgBox.exec();
        return;
    }
    pSliceView = new ShowSlices(/*&cPJ*/);
    pSliceView->pCPJ = &cPJ;
    pSliceView->GoToSlice(0);
    pSliceView->UpdateWidgets();

    pSliceView->show();
}

void MainWindow::showPrintWindow()
{
    QFileDialog dialog(0);
    QSettings settings("Dir");
    QString openFile = dialog.getOpenFileName(this,"Select WINSUN File", settings.value("WorkingDir").toString(), tr("WINSUN File (*.ws)"));//cyp .b9j
    if(openFile.isEmpty()) return;
    settings.setValue("WorkingDir", QFileInfo(openFile).absolutePath());


    m_pCPJ->clearAll();

//    CrushedPrintJob *newPCJ = new CrushedPrintJob;
    QFile file(openFile);
    if(!m_pCPJ->loadCPJ(&file)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle("WINSUN");
        msgBox.setText("Error Loading File.  Unknown Version?");//"");
        msgBox.setToolTip("文件加载错误。未知版本？");
        msgBox.exec();
        return;
    }
    m_pCPJ->showSupports(true);
    m_pCPJ->setName(openFile);//cyp

    pw->setCPJ(m_pCPJ);
//    pw = new PrintWindow(m_pCPJ,t);
//    connect (pw, SIGNAL(accepted()),this,SLOT(doPrint()));
    pw->show();
}

//returns a list of the currently selected instances
std::vector<B9ModelInstance*> MainWindow::GetSelectedInstances()
{
    return pModelList->GetSelectedInstances();
}
std::vector<B9ModelInstance*> MainWindow::GetAllInstances()
{
    unsigned int d, i;
    std::vector<B9ModelInstance*> allInsts;

    for(d = 0; d < ModelDataList.size(); d++)
    {
        for(i = 0; i < ModelDataList[d]->instList.size(); i++)
        {
            allInsts.push_back(ModelDataList[d]->instList[i]);
        }
    }

    return allInsts;


}




//////////////////////////////////////////////////////
//Public Slots
//////////////////////////////////////////////////////

//Base Plate creation and destruction is handled here in addition to other base plate properties.
void MainWindow::OnBasePlatePropertiesChanged()
{
    B9SupportStructure* basePlate;

    if(SupportModeInst() == NULL)
        return;


    basePlate = SupportModeInst()->GetBasePlate();

    //cyp checkBoxSupportBase
    //Creation/Destruction
    if(/*ui.Foundation_action->isChecked()*/pSupport->getBaseIsChecked() && (basePlate == NULL))
    {
        SupportModeInst()->EnableBasePlate();
    }

    if(/*!ui.Foundation_action->isChecked()*/!pSupport->getBaseIsChecked() && (basePlate != NULL))
    {
        SupportModeInst()->DisableBasePlate();
    }

    //check again
    basePlate = SupportModeInst()->GetBasePlate();

    if(basePlate != NULL)
    {
        basePlate->SetBottomAttachShape(pSettings->m_baseAttachShape);//cyp"Cylinder"

        basePlate->SetBottomRadius(pSettings->m_dBaseRadius);//actualRad

        basePlate->SetBottomLength(pSettings->m_dBaseLength/*0.5ui.Support_Base_Length_lineEdit->text().toDouble()*/);
    }

    UpdateInterface();
}


//called when selection changes primarily
void MainWindow::PushSupportProperties()
{
//   B9SupportStructure* selSup;
//   int indx;


//   //do minimal gui updating in add mode
//   if(!currSelectedSupports.size())
//        return;

//   selSup = currSelectedSupports[0];//TODO determine common properties and use those to fill
//                                    //gui, instead of just the first support

//   ui.Support_Top_Radius_lineEdit->setText(QString::number(selSup->GetTopRadius()));
//   ui.Support_Mid_Radius_lineEdit->setText(QString::number(selSup->GetMidRadius()));
//   ui.Support_Bottom_Radius_lineEdit->setText(QString::number(selSup->GetBottomRadius()));

//   ui.Support_Top_Length_lineEdit->setText(QString::number(selSup->GetTopLength()));
//   ui.Support_Mid_Length_lineEdit->setText(QString::number(selSup->GetMidLength()));
//   ui.Support_Bottom_Length_lineEdit->setText(QString::number(selSup->GetBottomLength()));

//   ui.Support_Top_AngleFactor_label->setText(QString::number(selSup->GetTopAngleFactor()*100.0).append("%"));
//   ui.Support_Bottom_AngleFactor_label->setText(QString::number(selSup->GetBottomAngleFactor()*100.0).append("%"));

//   ui.Support_Top_Penetration_label->setText(QString::number(selSup->GetTopPenetration()*100.0).append("um"));
//   ui.Support_Bottom_Penetration_label->setText(QString::number(selSup->GetBottomPenetration()*100.0).append("um"));

//   indx = ui.Support_Top_AttachType_comboBox->findText(selSup->GetTopAttachShape());
//   ui.Support_Top_AttachType_comboBox->blockSignals(true);
//    ui.Support_Top_AttachType_comboBox->setCurrentIndex(indx);
//   ui.Support_Top_AttachType_comboBox->blockSignals(false);

//   indx = ui.Support_Mid_AttachType_comboBox->findText(selSup->GetMidAttachShape());
//   ui.Support_Mid_AttachType_comboBox->blockSignals(true);
//    ui.Support_Mid_AttachType_comboBox->setCurrentIndex(indx);
//   ui.Support_Mid_AttachType_comboBox->blockSignals(false);

//   indx = ui.Support_Bottom_AttachType_comboBox->findText(selSup->GetBottomAttachShape());
//   ui.Support_Bottom_AttachType_comboBox->blockSignals(true);
//    ui.Support_Bottom_AttachType_comboBox->setCurrentIndex(indx);
//   ui.Support_Bottom_AttachType_comboBox->blockSignals(false);

//   //Angle Factor GUIs
//   ui.Support_Top_AngleFactor_horizontalSlider->blockSignals(true);
//    ui.Support_Top_AngleFactor_horizontalSlider->setValue(selSup->GetTopAngleFactor()*100.0);
//   ui.Support_Top_AngleFactor_horizontalSlider->blockSignals(false);
//   ui.Support_Bottom_AngleFactor_horizontalSlider->blockSignals(true);
//    ui.Support_Bottom_AngleFactor_horizontalSlider->setValue(selSup->GetBottomAngleFactor()*100.0);
//   ui.Support_Bottom_AngleFactor_horizontalSlider->blockSignals(false);

//   //Penetration GUIs
//   ui.Support_Top_Penetration_horizontalSlider->blockSignals(true);
//    ui.Support_Top_Penetration_horizontalSlider->setValue(selSup->GetTopPenetration()*100.0);
//   ui.Support_Top_Penetration_horizontalSlider->blockSignals(false);
//   ui.Support_Bottom_Penetration_horizontalSlider->blockSignals(true);
//    ui.Support_Bottom_Penetration_horizontalSlider->setValue(selSup->GetBottomPenetration()*100.0);
//   ui.Support_Bottom_Penetration_horizontalSlider->blockSignals(false);

}

void MainWindow::PushBasePlateProperties()
{
    B9SupportStructure* basePlate = SupportModeInst()->GetBasePlate();
//    int indx;

    if(basePlate == NULL)
    {
//        ui.Support_Base_GroupBox->setChecked(false);
//        ui.Support_Base_Frame->hide();
    }
    else
    {
//        ui.Support_Base_GroupBox->setChecked(true);
//        ui.Support_Base_Frame->show();

//        indx = ui.Support_Base_AttachType_comboBox->findText(basePlate->GetBottomAttachShape());
//        ui.Support_Base_Coverage_label->setText(QString::number(ui.Support_Base_Coverage_horizontalSlider->value()) + QString("%"));
//        ui.Support_Base_AttachType_comboBox->setCurrentIndex(indx);
//        ui.Support_Base_Length_lineEdit->setText(QString::number(basePlate->GetBottomLength()));
    }
}


void MainWindow::ResetSupportLight()//connected to push button will always use hardcoded values!
{
    B9SupportStructure::FillRegistryDefaults(true,"LIGHT");
    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        FillSupportParamsWithDefaults();

        ui.Support_Reset_Heavy_button->setChecked(false);
        ui.Support_Reset_Medium_button->setChecked(false);
    }
    UpdateInterface();
}
void MainWindow::ResetSupportMedium()//connected to push button will always use hardcoded values!
{
    B9SupportStructure::FillRegistryDefaults(true,"MEDIUM");
    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        FillSupportParamsWithDefaults();

        ui.Support_Reset_Heavy_button->setChecked(false);
        ui.Support_Reset_Light_button->setChecked(false);
    }
    UpdateInterface();
}
void MainWindow::ResetSupportHeavy()//connected to push button will always use hardcoded values!
{
    B9SupportStructure::FillRegistryDefaults(true,"HEAVY");
    if(pWorldView->GetTool() == "SUPPORTADD")
    {
        FillSupportParamsWithDefaults();

        ui.Support_Reset_Light_button->setChecked(false);
        ui.Support_Reset_Medium_button->setChecked(false);
    }
    UpdateInterface();
}

//fill the support parameter box will default params.
void MainWindow::FillSupportParamsWithDefaults()
{
//    int indx;

    QSettings appSettings;
    appSettings.beginGroup("USERSUPPORTPARAMS");
//    appSettings.beginGroup("SUPPORT_TOP");
//        indx = ui.Support_Top_AttachType_comboBox->findText(appappSettings.value("ATTACHSHAPE").toString());
//        ui.Support_Top_AttachType_comboBox->blockSignals(true);
//            ui.Support_Top_AttachType_comboBox->setCurrentIndex(indx);
//        ui.Support_Top_AttachType_comboBox->blockSignals(false);
//        ui.Support_Top_AngleFactor_horizontalSlider->blockSignals(true);
//            ui.Support_Top_AngleFactor_horizontalSlider->setValue(appappSettings.value("ANGLEFACTOR").toDouble()*100);
//        ui.Support_Top_AngleFactor_horizontalSlider->blockSignals(false);
//        ui.Support_Top_AngleFactor_label->setText(appappSettings.value("ANGLEFACTOR").toString());
//        ui.Support_Top_Length_lineEdit->setText(appappSettings.value("LENGTH").toString());
//        ui.Support_Top_Penetration_horizontalSlider->blockSignals(true);
//            ui.Support_Top_Penetration_horizontalSlider->setValue(appappSettings.value("PENETRATION").toDouble()*100);
//        ui.Support_Top_Penetration_horizontalSlider->blockSignals(false);
//        ui.Support_Top_Penetration_label->setText(appappSettings.value("PENETRATION").toString());
//        ui.Support_Top_Radius_lineEdit->setText(appappSettings.value("RADIUS").toString());
        pSettings->m_dTopRadius = appSettings.value("TopRadius").toDouble();
        pSettings->m_dTopLength = appSettings.value("TopLength").toDouble();
        pSettings->m_dTopPenetration = appSettings.value("TopPenetration").toDouble();
        pSettings->m_dTopAngleFactor = appSettings.value("TopAngleFactor").toDouble();
    appSettings.endGroup();

//    appappSettings.beginGroup("SUPPORT_MID");
//        indx = ui.Support_Mid_AttachType_comboBox->findText(appappSettings.value("ATTACHSHAPE").toString());
//        ui.Support_Mid_AttachType_comboBox->blockSignals(true);
//            ui.Support_Mid_AttachType_comboBox->setCurrentIndex(indx);
//        ui.Support_Mid_AttachType_comboBox->blockSignals(false);
//        ui.Support_Mid_Radius_lineEdit->setText(appappSettings.value("RADIUS").toString());
        pSettings->m_dMidRadius = appSettings.value("MidRadius").toDouble();
//    appappSettings.endGroup();

//    appappSettings.beginGroup("SUPPORT_BOTTOM_GROUNDED");
        pSettings->m_dBottomRadius = appSettings.value("BottomRadius").toDouble();
        pSettings->m_dBottomLength = appSettings.value("BottomLength").toDouble();
        pSettings->m_dBottomPenetration = appSettings.value("BottomPenetration").toDouble();
        pSettings->m_dBottomAngleFactor = appSettings.value("BottomAngleFactor").toDouble();
//        indx = ui.Support_Bottom_AttachType_comboBox->findText(appappSettings.value("ATTACHSHAPE").toString());
//        ui.Support_Bottom_AttachType_comboBox->blockSignals(true);
//            ui.Support_Bottom_AttachType_comboBox->setCurrentIndex(indx);
//        ui.Support_Bottom_AttachType_comboBox->blockSignals(false);
//        ui.Support_Bottom_AngleFactor_horizontalSlider->blockSignals(true);
//            ui.Support_Bottom_AngleFactor_horizontalSlider->setValue(appappSettings.value("ANGLEFACTOR").toDouble()*100);
//        ui.Support_Bottom_AngleFactor_horizontalSlider->blockSignals(false);
//        ui.Support_Bottom_AngleFactor_label->setText(appappSettings.value("ANGLEFACTOR").toString());
//        ui.Support_Bottom_Length_lineEdit->setText(appappSettings.value("LENGTH").toString());
//        ui.Support_Bottom_Penetration_horizontalSlider->blockSignals(true);
//            ui.Support_Bottom_Penetration_horizontalSlider->setValue(appappSettings.value("PENETRATION").toDouble()*100);
//        ui.Support_Bottom_Penetration_horizontalSlider->blockSignals(false);
//        ui.Support_Bottom_Penetration_label->setText(appappSettings.value("PENETRATION").toString());
//        ui.Support_Bottom_Radius_lineEdit->setText(appappSettings.value("RADIUS").toString());
//    appappSettings.endGroup();
    appSettings.endGroup();
}


//debug interface
void MainWindow::OpenDebugWindow()
{


}

//file
void MainWindow::New()
{
    SetSupportMode(false);
    RemoveAllInstances(); //remove instances.
    project->New();
    UpdateBuildSpaceUI();
    project->SetDirtied(false);//because UpdatingBuildSpaceUI dirties things in a round about way.
    pWorldView->CenterView();
    pActionSupport->setChecked(false);
    ui.slice->setEnabled(false);
    ui.actionSave->setEnabled(false);
    ui.actionSave_As->setEnabled(false);
//    ui.Foundation_action->setEnabled(false);
    UpdateInterface();
}
QString MainWindow::Open(bool withoutVisuals)
{
    bool success;

    QSettings settings("Dir");


    //first check if the user needs to save what hes working on first.
    if(project->IsDirtied())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("WINSUN");
        msgBox.setText("The current layout has been modified.");//
         msgBox.setInformativeText("Do you want to save your changes before opening?");//
         msgBox.setToolTip("当前布局已被修改\n打开之前要保存吗？");
         msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//         msgBox.setButtonText(QMessageBox::Save, "保存");
//         msgBox.setButtonText(QMessageBox::Discard, "不保存");
//         msgBox.setButtonText(QMessageBox::Cancel, "取消");
         msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret)
        {
          case QMessageBox::Save:
                SaveAs();
              break;
          case QMessageBox::Discard:
                //do nothing
              break;
          case QMessageBox::Cancel:
            return "";
              break;
          default:
              break;
        }
    }


//    QString filename = QFileDialog::getOpenFileName(this,
//             tr("选择布局文件"), appSettings.value("WorkingDir").toString(), tr("WINSUN 布局文件 (*.wstl)"));
    QString filename = QFileDialog::getOpenFileName(this,
             tr("Select .wstl"), settings.value("WorkingDir").toString(), tr("WINSUN stl File (*.wstl)"));

    if(filename.isEmpty())
    {return "";}


    QApplication::setOverrideCursor(Qt::WaitCursor);

    SetSupportMode(false);
    RemoveAllInstances(); //remove instances.
    success = project->Open(filename,withoutVisuals);

    //lets update some of the UI stuff to match what we just loaded.
    UpdateBuildSpaceUI();
    pWorldView->UpdatePlasmaFence();

    QApplication::restoreOverrideCursor();
    if(!success)
    {
        QMessageBox::warning(this, tr("MainWindow"), tr("Open Failed"),QMessageBox::Ok);//打开失败

        return "";
    }

    CleanModelData();// now delete unneeded model data

    //set recent directory.
    settings.setValue("WorkingDir", QFileInfo(filename).absolutePath());

    ui.slice->setEnabled(true);
    ui.actionSave->setEnabled(true);
    ui.actionSave_As->setEnabled(true);
    pActionSupport->setChecked(false);

    return filename;

}
void MainWindow::Save()
{
    if(project->GetFileName() == "untitled")
    {
        SaveAs();
    }
    else
    {
        project->Save(project->GetFileName());
    }
}
void MainWindow::SaveAs()
{
    bool success;

    QSettings settings("Dir");

    QString filename = CROSS_OS_GetSaveFileName(this, tr("Save"),
                    settings.value("WorkingDir").toString(),
                                                tr("WINSUN stl (*.wstl)"),QStringList("wstl"));
    if(filename.isEmpty())
    {
        return;
    }

    success = project->Save(filename);
    if(!success)
    {
        QMessageBox::warning(this, tr("MainWindow"), tr("保存失败"),QMessageBox::Ok);
        return;
    }

    settings.setValue("WorkingDir",QFileInfo(filename).absolutePath());
}

//interface
void MainWindow::OnChangeTab(int idx)
{
//    if(idx == 1)
//    {
//        SetSupportMode(true);
//        ui.actionSupportMode->blockSignals(true);
//        ui.actionSupportMode->setChecked(true);
//        ui.actionSupportMode->blockSignals(false);
//    }
//    if(idx == 0)
//    {
//        ui.actionSupportMode->blockSignals(true);
//        ui.actionSupportMode->setChecked(false);
//        ui.actionSupportMode->blockSignals(false);
//        SetSupportMode(false);
//    }
}

void MainWindow::SetXYPixelSizePreset(QString size)
{
    project->SetPixelSize(size.toDouble());
    project->CalculateBuildArea();
    UpdateExtentsUI();
}

void MainWindow::SetZLayerThickness(QString thick)
{
    project->SetPixelThickness(thick.toDouble());
    project->CalculateBuildArea();
    UpdateExtentsUI();
}

void MainWindow::SetProjectorX(QString x)
{
    project->SetResolution(QVector2D(x.toInt(),project->GetResolution().y()));
    project->CalculateBuildArea();
}
void MainWindow::SetProjectorY(QString y)
{
    project->SetResolution(QVector2D(project->GetResolution().x(),y.toInt()));
    project->CalculateBuildArea();
}
void MainWindow::SetProjectorPreset(int index)
{
    switch(index)
    {
        case 0:
            SetProjectorX(QString().number(1024));
            SetProjectorY(QString().number(768));

            break;
        case 1:
            SetProjectorX(QString().number(1280));
            SetProjectorY(QString().number(768));

            break;
        case 2:
            SetProjectorX(QString().number(1920));
            SetProjectorY(QString().number(1080));
            break;
        case 3:
            SetProjectorX(QString().number(1920));
            SetProjectorY(QString().number(1200));
            break;
        case 4:
            SetProjectorX(QString().number(2048));
            SetProjectorY(QString().number(1536));

            break;
        default:


            break;

    }
    UpdateExtentsUI();

}

void MainWindow::SetZHeight(QString z)
{
    project->SetBuildSpaceSize(QVector3D(project->GetBuildSpace().x(),project->GetBuildSpace().x(), z.toDouble()));

}

void MainWindow::SetAttachmentSurfaceThickness(QString num)
{
    project->SetAttachmentSurfaceThickness(num.toDouble());
}

void MainWindow::UpdateBuildSpaceUI()
{
    int pixi;
    int proi;

    //pixel sizes
    if(project->GetPixelSize() == 50)
        pixi=0;
    else if(project->GetPixelSize() == 75)
        pixi=1;
    else if(project->GetPixelSize() == 100)
        pixi=2;

    //projector resolutions
    if(project->GetResolution() == QVector2D(1024,768))
        proi=0;
    else if(project->GetResolution() == QVector2D(1280,768))
        proi=1;
    else if(project->GetResolution() == QVector2D(1920,1080))
        proi=2;
    else if(project->GetResolution() == QVector2D(1920,1200))
        proi=3;
    else if(project->GetResolution() == QVector2D(2048,1536))
        proi=4;



//    ui.pixelsizecombo->setCurrentIndex(pixi);
//    ui.projectorcombo->setCurrentIndex(proi);

}


void MainWindow::UpdateExtentsUI()
{

//    ui.Print_Extents_X->setText(QString::number(ProjectData()->GetBuildSpace().x(),'g',4));
//    ui.Print_Extents_Y->setText(QString::number(ProjectData()->GetBuildSpace().y(),'g',4));
//    ui.Print_Extents_Z->setText(QString::number(ProjectData()->GetBuildSpace().z(),'g',4));

}
void MainWindow::UpdateInterface()
{
    pModelList->UpdateInterface();
    if(pModelList->GetSelectedItem().size() <= 0 )//no items selected.
    {
        ui.actionDuplicate->setEnabled(false);
        ui.actionDelete->setEnabled(false);
    }
    else//1 or more items selected
    {
        SupportModeInst();
        ui.actionDuplicate->setEnabled(true);
        ui.actionDelete->setEnabled(true);
    }
    if(pModelList->GetSelectedItem().size() <= 0 || pModelList->GetSelectedItem().size() > 1)
        //no or multiple items selected.
    {
        pActionSupport->setEnabled(false);
//        ui.menuSupport->setEnabled(false);

    }
    else //exactly ONE object selected
    {
        pActionSupport->setEnabled(true);
    }

    //refresh support structure tab as well.
    if(SupportModeInst() != NULL)//in support mode
    {
        if(pWorldView->GetTool() == "SUPPORTADD")
        {
            //start off with the right button checked
            QSettings appSettings;
            appSettings.beginGroup("USERSUPPORTPARAMS");
            QString weight = appSettings.value("ADDPRESETWEIGHT","LIGHT").toString();
            if(weight == "LIGHT") ui.Support_Reset_Light_button->setChecked(true);
            if(weight == "MEDIUM") ui.Support_Reset_Medium_button->setChecked(true);
            if(weight == "HEAVY") ui.Support_Reset_Heavy_button->setChecked(true);
        }
        //fill the base plate gui region
        PushBasePlateProperties();
    }
    else//out of support mode
    {
        UpdateExtentsUI();

    }

}

void MainWindow::PushTranslations()
{

        UpdateInterface();

}

//when the spin slider changes value by the user.
void MainWindow::OnModelSpinSliderChanged(int val)
{
//    SetSelectionRot(QVector3D(ui.rotx->text().toDouble(),
//                              ui.roty->text().toDouble(),
//                              val));

//    ui.rotz->setText(QString::number(val));
}
void MainWindow::OnModelSpinSliderReleased()
{
    for(unsigned int i = 0; i < GetSelectedInstances().size(); i++)
    {
        GetSelectedInstances()[i]->UpdateBounds();
    }
    UpdateInterface();
}

void MainWindow::LockScale(bool lock)
{
//    if(lock)
//    {
//        SetSelectionScale(ui.scalex->text().toDouble(),0,0,1);
//        SetSelectionScale(0,ui.scalex->text().toDouble(),0,2);
//        SetSelectionScale(0,0,ui.scalex->text().toDouble(),3);
//        UpdateInterface();
//    }
}


//tools interface
void MainWindow::SetTool(QString toolname)
{
    if(toolname == "MODELSELECT")
        SetToolModelSelect();
    else if(toolname == "MODELMOVE")
        SetToolModelMove();
    else if(toolname == "MODELSPIN")
        SetToolModelSpin();
    else if(toolname == "MODELORIENTATE")
        SetToolModelOrientate();
    else if(toolname == "MODELSCALE")
        SetToolModelScale();
    else if(toolname == "SUPPORTADD")
        SetToolSupportAdd();
    else if(toolname == "SUPPORTDELETE")
        SetToolSupportDelete();
}

void MainWindow::SetToolModelSelect()
{
    pWorldView->SetTool("MODELSELECT");
    ui.actionMove->setChecked(false);
    ui.actionRotate->setChecked(false);
    ui.actionScale->setChecked(false);
}
void MainWindow::SetToolModelMove()
{
    pWorldView->SetTool("MODELMOVE");
    ui.actionMove->setChecked(true);
    ui.actionRotate->setChecked(false);
    ui.actionScale->setChecked(false);
}
void MainWindow::SetToolModelSpin()
{
    pWorldView->SetTool("MODELSPIN");
//    ui.actionOrientate->setChecked(false);
    ui.actionRotate->setChecked(true);
    ui.actionMove->setChecked(false);
//    ui.actionSelection->setChecked(false);
    ui.actionScale->setChecked(false);
}
void MainWindow::SetToolModelOrientate()
{
    pWorldView->SetTool("MODELORIENTATE");
//    ui.actionOrientate->setChecked(true);
    ui.actionRotate->setChecked(true);
    ui.actionMove->setChecked(false);
//    ui.actionSelection->setChecked(false);
    ui.actionScale->setChecked(false);
}

void MainWindow::SetToolModelScale()
{
    pWorldView->SetTool("MODELSCALE");
    ui.actionScale->setChecked(true);
    ui.actionMove->setChecked(false);
//    ui.actionSelection->setChecked(false);
//    ui.actionOrientate->setChecked(false);
    ui.actionRotate->setChecked(false);
}




void MainWindow::ExitToolAction()
{
    pWorldView->ExitToolAction();
}



//contour aid
void MainWindow::OnToggleContourAid(bool tog)
{
    useContourAid = tog;
}

bool MainWindow::ContourAidEnabled()
{
    return useContourAid;
}

//xray vision
void MainWindow::OnToggleXRay(bool tog)
{

}

void MainWindow::OnXRayChange(int val)
{

}

bool MainWindow::XRayEnabled()
{
    return false;

}

//support hiding
bool MainWindow::HidingSupports()
{
    return hideSupports;
}

void MainWindow::OnToggleSupportHiding(bool tog)
{
    hideSupports = tog;
}





//model
B9ModelInstance* MainWindow::AddModel(QString filepath, bool bypassVisuals)
{
    QSettings settings("Dir");

    if(filepath.isEmpty())
    {
        filepath = QFileDialog::getOpenFileName(this,
            tr("Open Model"), settings.value("WorkingDir").toString(), tr("Models (*.stl)"));

        //cancel button
        if(filepath.isEmpty())
            return NULL;
    }
    //by this point we should have a valid file path, if we dont - abort.
    if(!QFileInfo(filepath).exists())
    {
        return NULL;
    }

    //if the file has already been opened and is in the project, we dont want to load in another! instead we want to make a new instance
    for(unsigned int i = 0; i < ModelDataList.size(); i++)
    {
        if(ModelDataList[i]->GetFilePath() == filepath)
        {
            return ModelDataList[i]->AddInstance();//make a new instance
        }
    }

    ModelData* pNewModel = new ModelData(this, bypassVisuals);

    bool success = pNewModel->LoadIn(filepath);
    if(!success)
    {
        delete pNewModel;
        return NULL;
    }

    //update registry
    settings.setValue("WorkingDir",QFileInfo(filepath).absolutePath());

    //add to the list
    ModelDataList.push_back(pNewModel);

    //make an Instance of the model!
    B9ModelInstance* pNewInst = pNewModel->AddInstance();
    project->UpdateZSpace();

    //select it too
    SelectOnly(pNewInst);

    ui.slice->setEnabled(true);
    ui.actionSave->setEnabled(true);
    ui.actionSave_As->setEnabled(true);

    return pNewInst;
}
void MainWindow::RemoveAllInstances()
{
    unsigned int m;
    unsigned int i;

    std::vector<B9ModelInstance*> allinstlist;
    for(m=0;m<this->ModelDataList.size();m++)
    {
        ModelDataList[m]->loadedcount = 0;//also reset the index counter for instances!
        for(i=0;i<ModelDataList[m]->instList.size();i++)
        {
            allinstlist.push_back(ModelDataList[m]->instList[i]);
        }
    }
    for(i=0;i<allinstlist.size();i++)
    {
        delete allinstlist[i];
    }

    CleanModelData();

}
void MainWindow::CleanModelData()
{
    unsigned int m;
    std::vector<ModelData*> templist;
    for(m=0;m<ModelDataList.size();m++)
    {
        if(ModelDataList[m]->instList.size() > 0)
        {
            templist.push_back(ModelDataList[m]);
        }
        else
        {
            delete ModelDataList[m];
        }
    }
    ModelDataList.clear();
    ModelDataList = templist;
    if(ModelDataList.size() <= 0){
        ui.slice->setEnabled(false);
        ui.actionSave->setEnabled(false);
        ui.actionSave_As->setEnabled(false);
    }
}

void MainWindow::AddTagToModelList(QListWidgetItem* item)
{
    //    ui.ModelList->addItem(item);
        pModelList->AddTagToModelList(item);
}

B9ModelInstance* MainWindow::FindInstance(QListWidgetItem* item)
{
    unsigned int d;
    unsigned int i;
    for(d=0;d<ModelDataList.size();d++)
    {
        for(i=0;i<ModelDataList[d]->instList.size();i++)
        {
            if(ModelDataList[d]->instList[i]->listItem == item)
            {
                return ModelDataList[d]->instList[i];
            }
        }
    }
    return NULL;
}


//selection
void MainWindow::RefreshSelectionsFromList()
{
//    int i;
//    for(i=0;i<ui.ModelList->count();i++)
//    {
//        B9ModelInstance* inst = FindInstance(ui.ModelList->item(i));
//        if(inst == NULL)
//            return;

//        if(!ui.ModelList->item(i)->isSelected())
//        {
//            DeSelect(FindInstance(ui.ModelList->item(i)));
//        }
//        else if(ui.ModelList->item(i)->isSelected())
//        {
//            Select(FindInstance(ui.ModelList->item(i)));
//        }
//    }
}
void MainWindow::Select(B9ModelInstance* inst)
{
    //qDebug() << inst << "added to selection";
    inst->SetSelected(true);
    UpdateInterface();
}
void MainWindow::DeSelect(B9ModelInstance* inst)
{
    //qDebug() << inst << "removed from selection";
    inst->SetSelected(false);
    UpdateInterface();
}
void MainWindow::SelectOnly(B9ModelInstance* inst)
{
    DeSelectAll();
    Select(inst);
}
void MainWindow::SelectOnly(B9SupportStructure* sup)
{
    DeSelectAllSupports();
    SelectSupport(sup);
}
void MainWindow::SelectSupport(B9SupportStructure* sup)
{
    //first see if the support is already selected, we dont want duplicates.
    unsigned int i;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {
        if(sup == currSelectedSupports[i])
            return;
    }

    //qDebug() << "Support: "<< sup << " added to selection";
    sup->SetSelected(true);
    currSelectedSupports.push_back(sup);

    UpdateInterface();


}

void MainWindow::DeSelectAll()
{
    unsigned int m;
    unsigned int i;
    for(m=0;m<ModelDataList.size();m++)
    {
        for(i=0;i<ModelDataList[m]->instList.size();i++)
        {
            DeSelect(ModelDataList[m]->instList[i]);
        }
    }
}

void MainWindow::SetSelectionPos(double x, double y, double z, int axis)
{
    int i;
    for(i=0;i<pModelList->GetSelectedItem().size();i++)
    {
        B9ModelInstance* inst = FindInstance(pModelList->GetSelectedItem()[i]);
        if(axis==0)
            inst->SetPos(QVector3D(x,y,z));
        else if(axis==1)
            inst->SetPos(QVector3D(x,inst->GetPos().y(),inst->GetPos().z()));
        else if(axis==2)
            inst->SetPos(QVector3D(inst->GetPos().x(),y,inst->GetPos().z()));
        else if(axis==3)
            inst->SetPos(QVector3D(inst->GetPos().x(),inst->GetPos().y(),z + inst->GetPos().z() - inst->GetMinBound().z()));
    }
}
void MainWindow::SetSelectionRot(QVector3D newRot)
{
    int i;
    for(i=0;i<pModelList->GetSelectedItem().size();i++)
    {
        B9ModelInstance* inst = FindInstance(pModelList->GetSelectedItem()[i]);

        inst->SetRot(newRot);
    }
}
void MainWindow::SetSelectionScale(double x, double y, double z, int axis)
{
    int i;
    for(i=0;i<pModelList->GetSelectedItem().size();i++)
    {
        B9ModelInstance* inst = FindInstance(pModelList->GetSelectedItem()[i]);
        if(axis==0)
            inst->SetScale(QVector3D(x,y,z));
        else if(axis==1)
            inst->SetScale(QVector3D(x,inst->GetScale().y(),inst->GetScale().z()));
        else if(axis==2)
            inst->SetScale(QVector3D(inst->GetScale().x(),y,inst->GetScale().z()));
        else if(axis==3)
            inst->SetScale(QVector3D(inst->GetScale().x(),inst->GetScale().y(),z));
    }
}
void MainWindow::SetSelectionFlipped(bool flipped)
{
    unsigned int i;
    std::vector<B9ModelInstance*> instList = GetSelectedInstances();

    for(i=0; i < instList.size(); i++)
    {
        instList[i]->SetFlipped(flipped);
    }

}


void MainWindow::DropSelectionToFloor()
{
    unsigned int i;
    for(i = 0; i < GetSelectedInstances().size(); i++)
    {
        GetSelectedInstances()[i]->RestOnBuildSpace();
    }
    UpdateInterface();
}
void MainWindow::ResetSelectionRotation()
{
    unsigned int i;
    for(i = 0; i < GetSelectedInstances().size(); i++)
    {
        GetSelectedInstances()[i]->SetRot(QVector3D(0,0,0));
        GetSelectedInstances()[i]->UpdateBounds();
    }
    UpdateInterface();
}


void MainWindow::DuplicateSelection()
{
    unsigned int i;
    B9ModelInstance* inst;
    B9ModelInstance* newinst;
    B9ModelInstance* compareinst;
    B9SupportStructure* newSup;
    bool good = true;
    double x;
    double y;
    double xkeep;
    double ykeep;
    std::vector<B9ModelInstance*> sellist = GetSelectedInstances();
    for(i=0;i<sellist.size();i++)
    {
        inst = sellist[i];

        double xsize = inst->GetMaxBound().x() - inst->GetMinBound().x();
        double ysize = inst->GetMaxBound().y() - inst->GetMinBound().y();

        xkeep = 0;
        ykeep = 0;
        for(x = -project->GetBuildSpace().x()*0.5 + xsize/2; x <= project->GetBuildSpace().x()*0.5 - xsize/2; x += xsize + 1)
        {
            for(y = -project->GetBuildSpace().y()*0.5 + ysize/2; y <= project->GetBuildSpace().y()*0.5 - ysize/2; y += ysize + 1)
            {
                good = true;
                for(unsigned int d=0;d<ModelDataList.size();d++)
                {
                    for(unsigned int n=0;n<ModelDataList[d]->instList.size();n++)
                    {
                        compareinst = ModelDataList[d]->instList[n];

                        if(((x - xsize/2) < compareinst->GetMaxBound().x()) && ((x + xsize/2) > compareinst->GetMinBound().x()) && ((y - ysize/2) < compareinst->GetMaxBound().y()) && ((y + ysize/2) > compareinst->GetMinBound().y()))
                        {
                            good = false;
                        }
                    }
                }
                if(good)
                {
                    xkeep = x;
                    ykeep = y;
                }

            }
        }

        newinst = inst->pData->AddInstance();
        newinst->SetPos(QVector3D(xkeep,ykeep,inst->GetPos().z()));
        newinst->SetRot(inst->GetRot());
        newinst->SetScale(inst->GetScale());
        newinst->SetFlipped(inst->GetFlipped());
        newinst->SetBounds(inst->GetMaxBound() + (newinst->GetPos() - inst->GetPos()),
                           inst->GetMinBound() + (newinst->GetPos() - inst->GetPos()));
        //dup supports
        for(i = 0; i < inst->GetSupports().size(); i++)
        {
            newSup = newinst->AddSupport();
            newSup->CopyFrom(inst->GetSupports()[i]);
            newSup->SetInstanceParent(newinst);

        }//and base plate
        if(inst->GetBasePlate())
        {
            newinst->EnableBasePlate();
            newinst->GetBasePlate()->CopyFrom(inst->GetBasePlate());
            newinst->GetBasePlate()->SetInstanceParent(newinst);
        }

        SelectOnly(newinst);
    }
}

//Upper level del action branching
void MainWindow::DeleteSelection()//delete whatever is selected - support or instance..
{
    if(SupportModeInst())
    {
        DeleteSelectedSupports();
    }
    else
    {
        DeleteSelectedInstances();
    }
}

void MainWindow::DeleteSelectedInstances()
{
    unsigned int i;
    std::vector<B9ModelInstance*> list = GetSelectedInstances();

    if(SupportModeInst())
        return;

    for(i=0;i < list.size();i++)
    {
        delete list[i];
    }
    //cleanup any unnessecary modeldata
    CleanModelData();
    UpdateInterface();
    project->UpdateZSpace();
}



//Support Mode/////////////////////////////////
void MainWindow::SetSupportMode(bool tog)
{
    pModelList->setEnabled(!tog);
    ui.slice->setEnabled(!tog);
    ui.actionSave->setEnabled(!tog);
    ui.actionSave_As->setEnabled(!tog);
    ui.menuEdit->setEnabled(!tog);
    ui.editToolBar->setEnabled(!tog);

    if(tog && (currInstanceInSupportMode == NULL))//go into support mode
    {

        qDebug() << "Entering Support Mode";
//        //make sure weve selected somthing
        if(!GetSelectedInstances().size())
            return;

        //we can assume weve selected something...
        currInstanceInSupportMode = GetSelectedInstances()[0];

       // if the instance is on the ground, raise it so we dont get crunched supports.
        if(currInstanceInSupportMode->GetMinBound().z() < 0.01 && !currInstanceInSupportMode->GetSupports().size())
        {
            oldModelConstricted = true;
            currInstanceInSupportMode->Move(QVector3D(0,0,5));
        }else oldModelConstricted = false;

        currInstanceInSupportMode->SetInSupportMode(true);
        //bake the instance in a manner similar to slice preparation
        //but without support baking!
        currInstanceInSupportMode->BakeGeometry();
        currInstanceInSupportMode->SortBakedTriangles();
        currInstanceInSupportMode->AllocateTriContainers(0.1);
        currInstanceInSupportMode->FillTriContainers();
        currInstanceInSupportMode->FormTriPickDispLists();

        oldZoom = pWorldView->GetZoom();
        oldPan = pWorldView->GetPan();
        oldRot = pWorldView->GetRotation();
        oldTool = pWorldView->GetTool();

        pWorldView->SetRevolvePoint(currInstanceInSupportMode->GetPos());
        pWorldView->SetPan(QVector3D(0,0,0));
        pWorldView->SetZoom(100.0);



        //set tool to support addition
        SetToolSupportAdd();

        FillSupportList();//refresh the list gui.

        pSettings->loadSettings();
        pSupport->updateDialog();
        pSupport->show();
    }
    else if(!tog && (currInstanceInSupportMode != NULL))
    {
        qDebug() << "Exiting Support Mode";

        if(currInstanceInSupportMode != NULL)
        {
            currInstanceInSupportMode->SetInSupportMode(false);
            currInstanceInSupportMode->FreeTriPickDispLists();
            currInstanceInSupportMode->UnBakeGeometry();
            currInstanceInSupportMode->FreeTriContainers();

            if(oldModelConstricted && !currInstanceInSupportMode->GetSupports().size())
                currInstanceInSupportMode->Move(QVector3D(0,0,-5));
            else
                currInstanceInSupportMode->SetPos(currInstanceInSupportMode->GetPos());//nudge to fix supports

            currInstanceInSupportMode = NULL;
        }
        pWorldView->SetRevolvePoint(QVector3D(0,0,0));
        pWorldView->SetPan(oldPan);
        pWorldView->SetZoom(oldZoom);
        SetTool(oldTool);

        pSupport->hide();
        //pWorldView->setXRotation(oldRot.x());
        //pWorldView->setYRotation(oldRot.y());
        //pWorldView->setZRotation(oldRot.z());

    }
    //if entering or leaving we always deselect all supports
    DeSelectAllSupports();

    UpdateInterface();
//    if(tog){
//        pSettings->loadSettings();
//        pSupport->updateDialog();
//        pSupport->show();
//    }
//    else pSupport->hide();
}

//SupportTools interface
void MainWindow::SetToolSupportModify()
{

}

void MainWindow::SetToolSupportAdd()
{
    pWorldView->SetTool("SUPPORTADD");
//    ui.Support_Add_Supports_action->blockSignals(true);
//        ui.Support_Add_Supports_action->setChecked(true);
//    ui.Support_Add_Supports_action->blockSignals(false);

//    ui.Support_Delete_Supports_action->blockSignals(true);
//        ui.Support_Delete_Supports_action->setChecked(false);
//    ui.Support_Delete_Supports_action->blockSignals(false);

    //when were adding nothing should be selected
    DeSelectAllSupports();

    UpdateInterface();

    FillSupportParamsWithDefaults();


}

void MainWindow::SetToolSupportDelete()
{
    pWorldView->SetTool("SUPPORTDELETE");
//    ui.Support_Add_Supports_action->blockSignals(true);
//        ui.Support_Add_Supports_action->setChecked(false);
//    ui.Support_Add_Supports_action->blockSignals(false);

//    ui.Support_Delete_Supports_action->blockSignals(true);
//        ui.Support_Delete_Supports_action->setChecked(true);
//    ui.Support_Delete_Supports_action->blockSignals(false);

    //when were deleteing nothing should be selected
    DeSelectAllSupports();

    UpdateInterface();
}


void MainWindow::FillSupportList()
{

}

void MainWindow::UpdateSupportList()
{

}



B9SupportStructure* MainWindow::FindSupportByName(QString name)
{
    unsigned int s;
    unsigned int searchNum = name.remove("Support ").toInt();
    std::vector<B9SupportStructure*> supList;

    if(!currInstanceInSupportMode)
    {
        qDebug() << "WARNING: finding support out of support mode";
         return NULL;
    }

    supList = currInstanceInSupportMode->GetSupports();

    for(s = 0; s < supList.size(); s++)
    {
        if(searchNum == supList[s]->SupportNumber)
        {
            return supList[s];
        }
    }
    return NULL;
}


void MainWindow::RefreshSupportSelectionsFromList()
{

}



B9ModelInstance* MainWindow::SupportModeInst()
{
    return currInstanceInSupportMode;
}


std::vector<B9SupportStructure*>* MainWindow::GetSelectedSupports()
{
    return &currSelectedSupports;
}


bool MainWindow::IsSupportSelected(B9SupportStructure* sup)
{
    unsigned int i;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {
        if(currSelectedSupports[i] == sup)
            return true;
    }

    return false;
}

void MainWindow::DeSelectSupport(B9SupportStructure* sup)
{
    unsigned int s;
    std::vector<B9SupportStructure*> keepers;

    for(s = 0; s < currSelectedSupports.size(); s++)
    {
        if(currSelectedSupports[s] == sup)
        {
            //qDebug() << "Support: " << sup << " removed from selection";
            sup->SetSelected(false);
        }
        else
            keepers.push_back(currSelectedSupports[s]);
    }
    currSelectedSupports.clear();
    currSelectedSupports = keepers;

    UpdateInterface();

}


void MainWindow::DeSelectAllSupports()
{
    //qDebug() << "De-Selecting All Supports, selection list size: " << currSelectedSupports.size();
    while(currSelectedSupports.size())
    {
        DeSelectSupport(currSelectedSupports[0]);
    }
    UpdateSupportList();

}

void MainWindow::DeleteSelectedSupports()//called from remove button.
{
    unsigned int s;
    if(!SupportModeInst())
        return;

    for(s = 0; s < currSelectedSupports.size(); s++)
    {
        SupportModeInst()->RemoveSupport(currSelectedSupports[s]);
    }
    currSelectedSupports.clear();

    FillSupportList();

    UpdateInterface();
}

void MainWindow::DeleteSupport(B9SupportStructure* pSup)
{
    if(SupportModeInst() != NULL)
    {
        if(IsSupportSelected(pSup))
            DeSelectSupport(pSup);

        SupportModeInst()->RemoveSupport(pSup);
    }

    FillSupportList();

    UpdateInterface();
}

void MainWindow::MakeSelectedSupportsVertical()
{
    unsigned int i;
    B9SupportStructure* pSup;

    if(SupportModeInst() == NULL)
        return;

    for(i = 0; i < currSelectedSupports.size(); i++)
    {
        pSup = currSelectedSupports[i];
        pSup->SetBottomPoint(QVector3D(pSup->GetTopPivot().x(),
                                       pSup->GetTopPivot().y(),
                                       pSup->GetBottomPoint().z()));
    }
}

void MainWindow::MakeSelectedSupportsStraight()
{
    unsigned int i;
    B9SupportStructure* pSup;
    QVector3D lenVec;
    QVector3D topNorm, bottomNorm;

    if(SupportModeInst() == NULL)
        return;


    for(i = 0; i < currSelectedSupports.size(); i++)
    {
        pSup = currSelectedSupports[i];

        lenVec = pSup->GetTopPoint() - pSup->GetBottomPoint();
        lenVec.normalize();

        topNorm = pSup->GetTopNormal();
        bottomNorm = pSup->GetBottomNormal();

        topNorm = lenVec;
        topNorm.normalize();

        bottomNorm = -lenVec;
        bottomNorm.normalize();


        pSup->SetTopNormal(topNorm);
        if(!pSup->GetIsGrounded())
            pSup->SetBottomNormal(bottomNorm);

        pSup->SetTopAngleFactor(1.0);
        if(!pSup->GetIsGrounded())
            pSup->SetBottomAngleFactor(1.0);

    }

    UpdateInterface();
}

//Support Properties changes
void MainWindow::OnSupport_Top_AttachType_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetTopAttachShape(pSettings->m_topAttachShape);//cyp
    }

    if(updateInterface)
    UpdateInterface();

}
void MainWindow::OnSupport_Top_Radius_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetTopRadius(pSettings->m_dTopRadius);//cyp
    }

    if(updateInterface)
    UpdateInterface();

}
void MainWindow::OnSupport_Top_Length_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetTopLength(pSettings->m_dTopLength);//cyp
    }

    if(updateInterface)
    UpdateInterface();

}
void MainWindow::OnSupport_Top_Penetration_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetTopPenetration(pSettings->m_dTopPenetration*0.01);//cyp
    }

    if(updateInterface)
    UpdateInterface();

}
void MainWindow::OnSupport_Top_AngleFactor_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetTopAngleFactor(pSettings->m_dTopAngleFactor*0.01);//cyp
    }

    if(updateInterface)
    UpdateInterface();

}
void MainWindow::OnSupport_Mid_AttachType_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetMidAttachShape(pSettings->m_midAttachShape);//cyp
    }

    if(updateInterface)
    UpdateInterface();
}
void MainWindow::OnSupport_Mid_Radius_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {  selSup = currSelectedSupports[i];
       selSup->SetMidRadius(pSettings->m_dMidRadius);//cyp
    }

    if(updateInterface)
    UpdateInterface();
}
void MainWindow::OnSupport_Bottom_AttachType_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetBottomAttachShape(pSettings->m_bottomAttachShape);
    }

    if(updateInterface)
    UpdateInterface();

}
void MainWindow::OnSupport_Bottom_Radius_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetBottomRadius(pSettings->m_dBottomRadius);
    }

    if(updateInterface)
    UpdateInterface();

}
void MainWindow::OnSupport_Bottom_Length_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetBottomLength(pSettings->m_dBottomLength);
    }

    if(updateInterface)
    UpdateInterface();

}
void MainWindow::OnSupport_Bottom_Penetration_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetBottomPenetration(pSettings->m_dBottomPenetration*0.01);
    }

    if(updateInterface)
    UpdateInterface();

}
void MainWindow::OnSupport_Bottom_AngleFactor_Changed(bool updateInterface)
{
    unsigned int i;
    B9SupportStructure* selSup = NULL;
    for(i = 0; i < currSelectedSupports.size(); i++)
    {   selSup = currSelectedSupports[i];
        selSup->SetBottomAngleFactor(pSettings->m_dBottomAngleFactor*0.01);
    }

    if(updateInterface)
    UpdateInterface();

}



//overall slicing routine, returns success.
bool MainWindow::SliceWorld()
{//cyp
    QSettings settings("Dir");

    QString filename = CROSS_OS_GetSaveFileName(this, tr("Export Slices"),/*导出切片*/
                                                settings.value("WorkingDir").toString() + "/" + ProjectData()->GetJobName(),
                                                tr("WINSUN Slice File (*.ws);;SLC (*.slc)"));//cyp .b9j

    if(filename.isEmpty())//cancell button
    {
        return false;
    }

    QString Format = QFileInfo(filename).completeSuffix();
    settings.setValue("WorkingDir",QFileInfo(filename).absolutePath());

    if(Format.toLower() == "ws")//b9j
    {
        if(SliceWorldToJob(filename))
        {
//             QMessageBox::information(0,"完成","切片完成\n\n所有切片数据已保存.");
             QMessageBox::information(0,"Finished","Slicing Completed\n\nAll layers sliced and job file saved.");
             return true;
        }
        else
        {
//            QMessageBox::information(0,"取消","切片已取消\n\n保存失败.");
            QMessageBox::information(0,"Canceled","Slicing Canceled\n\nnothing was saved.");
            return false;
        }
    }
    else if(Format.toLower() == "slc")
    {
        if(SliceWorldToSlc(filename))
        {
//            QMessageBox::information(0,"完成","切片完成\n\n所有切片数据已保存.");
            QMessageBox::information(0,"Finished","Slicing Completed\n\nAll layers sliced and slc file saved.");
            return true;

        }
        else
        {
//            QMessageBox::information(0,"取消","切片已取消\n\n保存失败.");
            QMessageBox::information(0,"Cancelled","Slicing Cancelled\n\nnothing was saved.");
            return false;
        }
    }

    return false;
}

//slicing to a job file!
bool MainWindow::SliceWorldToJob(QString filename)
{
    unsigned int m,i,l;
    unsigned int totalSliceOps = 0;
    unsigned int globalLayers;
    int nummodels = 0;
    B9ModelInstance* pInst;
    double zhieght = project->GetBuildSpace().z();
    double thickness = project->GetPixelThickness()*0.001;
    QString jobname = project->GetJobName();
    QString jobdesc = project->GetJobDescription();
    CrushedPrintJob* pMasterJob = NULL;
    Slice* pSlice;
    bool moreSlicesToCome;

    //calculate how many layers we need
    globalLayers = qCeil(zhieght/thickness);//CYP

    //calculate how many models there are
    for(m=0;m<ModelDataList.size();m++)
    {
        for(i=0;i<ModelDataList[m]->instList.size();i++)
        {
            pInst = ModelDataList[m]->instList[i];
            totalSliceOps += qCeil((pInst->GetMaxBound().z() - pInst->GetMinBound().z())/thickness);
        }
    }

    //make a loading bar
    LoadingBar progressbar(0, totalSliceOps);
    QObject::connect(&progressbar,SIGNAL(rejected()),this,SLOT(CancelSlicing()));
    progressbar.setDescription("Slicing...");//切片中
    progressbar.setValue(0);
    QApplication::processEvents();


    //make a master job file for use later
    pMasterJob = new CrushedPrintJob();
    pMasterJob->setName(jobname);
    pMasterJob->setDescription(jobdesc);
    pMasterJob->setXYPixel(QString().number(project->GetPixelSize()/1000));
    pMasterJob->setZLayer(QString().number(project->GetPixelThickness()/1000));
    pMasterJob->clearAll(globalLayers);//fills the master job with the needed layers


    //FOR Each Model Instance
    for(m=0;m<ModelDataList.size();m++)
    {
        for(i=0;i<ModelDataList[m]->instList.size();i++)
        {
            B9ModelInstance* inst = ModelDataList[m]->instList[i];
            inst->PrepareForSlicing(thickness);

            //slice all layers and add to instance's job file
            for(l = 0; l < globalLayers; l++)
            {
                //if we are in the model's z - bounds
                if((double)l*thickness <= inst->GetMaxBound().z() && (double)l*thickness >= inst->GetMinBound().z()-0.5*thickness)
                {
                    inst->pSliceSet->QueNewSlice(l*thickness + thickness*0.5,l);
                }
            }
            if(nummodels == 1)
                inst->pSliceSet->SetSingleModelCompressHint(true);
            else
                inst->pSliceSet->SetSingleModelCompressHint(false);

            do
            {
                pSlice = inst->pSliceSet->ParallelCreateSlices(moreSlicesToCome,pMasterJob);
                if(pSlice != NULL)
                {
                    delete pSlice;
                    progressbar.setValue(progressbar.GetValue() + 1);
                    QApplication::processEvents();
                }

                if(cancelslicing)
                {
                    cancelslicing = false;
                    inst->FreeFromSlicing();
                    return false;
                }

            }while(moreSlicesToCome);



            inst->FreeFromSlicing();
        }
    }

    QFile* pf = new QFile(filename);

    pMasterJob->saveCPJ(pf);
    delete pf;
    delete pMasterJob;


    pWorldView->makeCurrent();

    cancelslicing = false;

    return true;
}

//slicing to a slc file!
bool MainWindow::SliceWorldToSlc(QString filename)
{
    unsigned int m;
    unsigned int i;
    int l;
    int numlayers;
    int nummodels = 0;

    double zhieght = project->GetBuildSpace().z();
    double thickness = project->GetPixelThickness()*0.001;

    Slice* currSlice = NULL;
    bool moreSlicesToCome;

    //calculate how many layers we need
    numlayers = qCeil(zhieght/thickness);
    //calculate how many models there are
    for(m=0;m<ModelDataList.size();m++)
    {
        for(i=0;i<ModelDataList[m]->instList.size();i++)
        {
            nummodels++;
        }
    }

    //make a loading bar
        LoadingBar progressbar(0, numlayers*nummodels);
        QObject::connect(&progressbar,SIGNAL(rejected()),this,SLOT(CancelSlicing()));
        progressbar.setDescription("Exporting SLC..");
        progressbar.setValue(0);
        QApplication::processEvents();


    //create an slc exporter
    SlcExporter slc(filename.toStdString());
    if(!slc.SuccessOpen())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("WINSUN");
        msgBox.setText("Unable To Open Slc File!");//
        msgBox.setToolTip(".slc文件打不开");
        msgBox.exec();
    }
    //write the header
    slc.WriteHeader("heeeeelllllloooooo");
    slc.WriteReservedSpace();
    slc.WriteSampleTableSize(1);
    slc.WriteSampleTable(0.0,float(thickness),0.0f);



    //for each modelinstance
    for(m=0;m<ModelDataList.size();m++)
    {
        for(i=0;i<ModelDataList[m]->instList.size();i++)
        {
            B9ModelInstance* inst = ModelDataList[m]->instList[i];
            inst->PrepareForSlicing(thickness);

            //slice all layers and add to instance's job file
            for(l = 0; l < numlayers; l++)
            {
                //make sure we are in the model's z - bounds
                if(l*thickness <= inst->GetMaxBound().z() && l*thickness >= inst->GetMinBound().z())
                {
                    inst->pSliceSet->QueNewSlice(l*thickness + thickness*0.5,l);
                }
            }

            do
            {
                currSlice = inst->pSliceSet->ParallelCreateSlices(moreSlicesToCome,0);
                if(currSlice != NULL)
                {
                    progressbar.setValue(progressbar.GetValue() + 1);
                    QApplication::processEvents();
                    slc.WriteNewSlice(currSlice->realAltitude, currSlice->loopList.size());
                    currSlice->WriteToSlc(&slc);
                    delete currSlice;
                }


                if(cancelslicing)
                {
                    cancelslicing = false;
                    inst->FreeFromSlicing();
                    return false;
                }

            }while(moreSlicesToCome);



            inst->FreeFromSlicing();
        }
    }

    slc.WriteNewSlice(0.0,0xFFFFFFFF);
    return true;
}


void MainWindow::CancelSlicing()
{
    cancelslicing = true;
}


//exporting
void MainWindow::PromptExportToSTL()
{
    QString filename;
    QSettings settings("Dir");
    filename = CROSS_OS_GetSaveFileName(this, tr("Export To STL"),
                                        settings.value("WorkingDir").toString() + "/" + ProjectData()->GetFileName(),
                                        tr("stl (*.stl)"));

    if(filename.isEmpty())
        return;

    if(ExportToSTL(filename))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("WINSUN");
        msgBox.setText("Stl Export Complete");//
        msgBox.setToolTip("stl导出完成");
        msgBox.exec();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("WINSUN");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Unable To Export stl File");//stl导出失败
        msgBox.setToolTip("stl导出失败");
        msgBox.exec();
    }
}

 bool MainWindow::ExportToSTL(QString filename)
 {
     unsigned int i;
     unsigned long int t;
     B9ModelInstance* pInst = NULL;
     Triangle3D* pTri = NULL;
     bool fileOpened;

     B9ModelWriter exporter(filename, fileOpened);


     if(!fileOpened)
         return false;

     SetSupportMode(false);

     for(i = 0; i < GetAllInstances().size(); i++)
     {
         pInst = GetAllInstances().at(i);
         pInst->BakeGeometry(true);

         for(t = 0; t < pInst->triList.size(); t++)
         {
             pTri = pInst->triList[t];
             exporter.WriteNextTri(pTri);
         }

         pInst->UnBakeGeometry();
     }

     exporter.Finalize();



     return true;
 }








//////////////////////////////////////////////////////
//Private
//////////////////////////////////////////////////////


///////////////////////////////////////////////////
//Events
///////////////////////////////////////////////////
void MainWindow::keyPressEvent(QKeyEvent * event )
{
    if(event->key() == Qt::Key_Escape)
    {
        SetSupportMode(false);
    }

    pWorldView->keyPressEvent(event);

}
void MainWindow::keyReleaseEvent(QKeyEvent * event )
{
    pWorldView->keyReleaseEvent(event);

}
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    //pWorldView->mousePressEvent(event);
    event->accept();
}
void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    //pWorldView->mouseReleaseEvent(event);
    event->accept();
}

void MainWindow::hideEvent(QHideEvent *event)
{
    emit eventHiding();

    pWorldView->pDrawTimer->stop();


    event->accept();
}
void MainWindow::showEvent(QShowEvent *event)
{

    pWorldView->pDrawTimer->start();
    showMaximized();
    event->accept();
}

void MainWindow::closeEvent ( QCloseEvent * event )
{

    //if the layout is dirty - ask the user if they want to save.
    if(project->IsDirtied())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("WINSUN");
        msgBox.setText("The layout has been modified.");//布局已被修改
         msgBox.setInformativeText("Do you want to save your changes?");//保存修改吗？
         msgBox.setToolTip("布局已被修改\n要保存吗？");
         msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//         msgBox.setButtonText(QMessageBox::Save,"保存");
//         msgBox.setButtonText(QMessageBox::Discard,"不保存");
//         msgBox.setButtonText(QMessageBox::Cancel,"取消");
         msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();


        switch (ret)
        {
          case QMessageBox::Save:
                Save();
              break;
          case QMessageBox::Discard:
                //nothing
              break;
          case QMessageBox::Cancel:
                event->ignore();
                return;
              break;
          default:
              break;
        }
    }

    //when we close the window - we want to make a new project.
    //because we might open the window again and we want a fresh start.
    New();
    event->accept();

}
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    /*
        QMenu menu(this);
        menu.addAction(ui.actionDelete);
        menu.addAction(ui.actionDrop_To_Floor);
        menu.addSeparator();
        menu.addAction(ui.actionMove);
        menu.addAction(ui.actionScale);

        menu.exec(event->globalPos());
    */
}

void MainWindow::doPrint()
{

    // print using variables set by wizard...
    pPrint->show();
//    pLogManager->setPrinting(false); //cyp set to true to Stop logfile entries when printing
    t->setIsPrinting(true);
    CROSS_OS_DisableSleeps(true);//disable things like screen saver - and power options.
    pPrint->print3D(m_pCPJ, 0, 0, pw->m_iTbaseMS, pw->m_iToverMS, pw->m_iTattachMS, pw->m_iNumAttach, pw->m_iLastLayer, pw->m_bDryRun, pw->m_bDryRun, pw->m_bMirrored);

    return;
}

void MainWindow::showCatalog()
{
    t->dlgEditMatCat();
}

void MainWindow::on_thicknesscombo_currentIndexChanged(const QString &arg1)
{
//    project->SetPixelThickness(ui.thicknesscombo->currentText().toDouble());

}


void MainWindow::on_ApplyButton_clicked()
{

}

void MainWindow::on_doubleSpinBoxBaseRadius_valueChanged(double arg1)
{
//    pSettings->m_dBaseRadius = arg1;

//    QSettings settings("supportsettings");
//    appSettings.setValue("BaseRadius",arg1);
//    if(ui.checkBoxSupportBase->isChecked())
//        OnBasePlatePropertiesChanged();
}

void MainWindow::on_pushButtonSupportSettings_clicked()
{
//    ui.doubleSpinBoxBaseRadius->setValue(pSettings->m_dBaseRadius);

    pSettings->updateValues();
    if(pActionSupport->isChecked()){
//        pBaseRadiusSlider->setValue(pSettings->m_dBaseRadius * 100 / 18);
        OnBasePlatePropertiesChanged();
    }
    else{
//        pSettings->m_dBaseRadius = pBaseRadiusSlider->value()* 18 / 100;
//        QSettings settings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("BaseRadius",pSettings->m_dBaseRadius);
        appSettings.endGroup();
    }
}

void MainWindow::OnBaseRadiusChange(int iValue)
{
//    pSettings->m_dBaseRadius = (double)iValue * 18 / 100;
//    QSettings settings("supportsettings");
//    appSettings.setValue("BaseRadius",pSettings->m_dBaseRadius);
//    if(ui.Foundation_action->isChecked())
//        OnBasePlatePropertiesChanged();
}
