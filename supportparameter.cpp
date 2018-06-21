#include "supportparameter.h"
#include "b9layout/b9modelinstance.h"
#include "ui_supportparameter.h"

SupportParameter::SupportParameter(QWidget *parent, MainWindow *main) :
    QWidget(parent),
    ui(new Ui::SupportParameter)
{
    ui->setupUi(this);
    pMain = main;
    m_pSettings = pMain->pSettings;
//    m_pSettings->loadSettings(); // load from system
//    updateDialog();
    for(size_t i = 0; i < B9SupportStructure::AttachmentDataList.size(); i++)
    {
        ui->Support_Top_AttachType_comboBox->addItem(B9SupportStructure::AttachmentDataList[i].GetName());
        ui->Support_Mid_AttachType_comboBox->addItem(B9SupportStructure::AttachmentDataList[i].GetName());
        ui->Support_Bottom_AttachType_comboBox->addItem(B9SupportStructure::AttachmentDataList[i].GetName());
        ui->Support_Base_AttachType_comboBox->addItem(B9SupportStructure::AttachmentDataList[i].GetName());
    }

    ui->pushButtonAutoSupport->hide();
}

SupportParameter::~SupportParameter()
{
    delete ui;
}

void SupportParameter::updateDialog()
{//Support_Base_Radius_doubleSpinBox
    ui->Support_Base_Length_doubleSpinBox->setValue(m_pSettings->m_dBaseLength);
    ui->Support_Base_Radius_doubleSpinBox->setValue(m_pSettings->m_dBaseRadius);

    ui->Support_Top_Radius_doubleSpinBox->setValue(m_pSettings->m_dTopRadius);
    ui->Support_Top_Length_doubleSpinBox->setValue(m_pSettings->m_dTopLength);
    ui->Support_Top_Penetration_horizontalSlider->setValue(m_pSettings->m_dTopPenetration*100);
    ui->Support_Top_AngleFactor_horizontalSlider->setValue(m_pSettings->m_dTopAngleFactor*100);

    ui->Support_Mid_Radius_doubleSpinBox->setValue(m_pSettings->m_dMidRadius);

    ui->Support_Bottom_Radius_doubleSpinBox->setValue(m_pSettings->m_dBottomRadius);
    ui->Support_Bottom_Length_doubleSpinBox->setValue(m_pSettings->m_dBottomLength);
    ui->Support_Bottom_Penetration_horizontalSlider->setValue(m_pSettings->m_dBottomPenetration*100);
    ui->Support_Bottom_AngleFactor_horizontalSlider->setValue(m_pSettings->m_dBottomAngleFactor*100);


    int indx;
    indx = ui->Support_Base_AttachType_comboBox->findText(m_pSettings->m_baseAttachShape);
    ui->Support_Base_AttachType_comboBox->blockSignals(true);
        ui->Support_Base_AttachType_comboBox->setCurrentIndex(indx);
    ui->Support_Base_AttachType_comboBox->blockSignals(false);

    indx = ui->Support_Top_AttachType_comboBox->findText(m_pSettings->m_topAttachShape);
    ui->Support_Top_AttachType_comboBox->blockSignals(true);
     ui->Support_Top_AttachType_comboBox->setCurrentIndex(indx);
    ui->Support_Top_AttachType_comboBox->blockSignals(false);

    indx = ui->Support_Mid_AttachType_comboBox->findText(m_pSettings->m_midAttachShape);
    ui->Support_Mid_AttachType_comboBox->blockSignals(true);
     ui->Support_Mid_AttachType_comboBox->setCurrentIndex(indx);
    ui->Support_Mid_AttachType_comboBox->blockSignals(false);

    indx = ui->Support_Bottom_AttachType_comboBox->findText(m_pSettings->m_bottomAttachShape);
    ui->Support_Bottom_AttachType_comboBox->blockSignals(true);
     ui->Support_Bottom_AttachType_comboBox->setCurrentIndex(indx);
    ui->Support_Bottom_AttachType_comboBox->blockSignals(false);

    ui->radioButtonAdd->setChecked(true);

    B9SupportStructure* basePlate = pMain->SupportModeInst()->GetBasePlate();
//    int indx;

    if(basePlate == NULL)
    {
        ui->checkBoxBase->setChecked(false);
    }
    else
    {
        ui->checkBoxBase->setChecked(true);
        indx = ui->Support_Base_AttachType_comboBox->findText(basePlate->GetBottomAttachShape());
        ui->Support_Base_AttachType_comboBox->setCurrentIndex(indx);
        ui->Support_Base_Radius_doubleSpinBox->setValue(basePlate->GetBottomRadius());
        ui->Support_Base_Length_doubleSpinBox->setValue(basePlate->GetBottomLength());
    }
}

void SupportParameter::stuffSettings()
{
    m_pSettings->m_baseAttachShape = ui->Support_Base_AttachType_comboBox->currentText();
    m_pSettings->m_dBaseLength = ui->Support_Base_Length_doubleSpinBox->value();
    m_pSettings->m_dBaseRadius = ui->Support_Base_Radius_doubleSpinBox->value();

    m_pSettings->m_topAttachShape = ui->Support_Top_AttachType_comboBox->currentText();
    m_pSettings->m_dTopRadius = ui->Support_Top_Radius_doubleSpinBox->value();
    m_pSettings->m_dTopLength = ui->Support_Top_Length_doubleSpinBox->value();
    m_pSettings->m_dTopPenetration = ui->Support_Top_Penetration_horizontalSlider->value()*0.01;
    m_pSettings->m_dTopAngleFactor = ui->Support_Top_AngleFactor_horizontalSlider->value()*0.01;

    m_pSettings->m_midAttachShape = ui->Support_Mid_AttachType_comboBox->currentText();
    m_pSettings->m_dMidRadius = ui->Support_Mid_Radius_doubleSpinBox->value();

    m_pSettings->m_bottomAttachShape = ui->Support_Bottom_AttachType_comboBox->currentText();
    m_pSettings->m_dBottomRadius  = ui->Support_Bottom_Radius_doubleSpinBox->value();
    m_pSettings->m_dBottomLength = ui->Support_Bottom_Length_doubleSpinBox->value();
    m_pSettings->m_dBottomPenetration = ui->Support_Bottom_Penetration_horizontalSlider->value()*0.01;
    m_pSettings->m_dBottomAngleFactor = ui->Support_Bottom_AngleFactor_horizontalSlider->value()*0.01;

}

void SupportParameter::on_radioButtonAdd_toggled(bool checked)
{
    if(checked){
        pMain->SetToolSupportAdd();
    }
}

void SupportParameter::on_radioButtonDelete_toggled(bool checked)
{
    if(checked){
        pMain->SetToolSupportDelete();
    }
}

void SupportParameter::on_pushButtonCleanAll_clicked()
{
    pMain->RemoveAllSupports();
}

void SupportParameter::on_pushButtonRestoreDefaults_clicked()
{
    m_pSettings->setFactorySettings();
    updateDialog();
}

void SupportParameter::on_checkBoxBase_toggled(bool checked)
{
    pMain->OnBasePlatePropertiesChanged();
}

bool SupportParameter::getBaseIsChecked()
{
    return ui->checkBoxBase->isChecked();
}

void SupportParameter::on_Support_Base_AttachType_comboBox_currentIndexChanged(const QString &arg1)
{
    m_pSettings->m_baseAttachShape = arg1;
    if(ui->checkBoxBase->isChecked()){
//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("BaseAttachShape",arg1);//cyp
        pMain->OnBasePlatePropertiesChanged();
        appSettings.endGroup();
    }
}

void SupportParameter::on_Support_Base_Radius_doubleSpinBox_valueChanged(double arg1)
{
    m_pSettings->m_dBaseRadius = arg1;
//    QSettings appSettings("supportsettings");
    QSettings appSettings;
    appSettings.beginGroup("USERSUPPORTPARAMS");
    appSettings.setValue("BaseRadius",arg1);//cyp
    appSettings.endGroup();
    if(ui->checkBoxBase->isChecked()){
        pMain->OnBasePlatePropertiesChanged();
    }
}

void SupportParameter::on_Support_Base_Length_doubleSpinBox_valueChanged(double arg1)
{
    m_pSettings->m_dBaseLength = arg1;
//    QSettings appSettings("supportsettings");
    QSettings appSettings;
    appSettings.beginGroup("USERSUPPORTPARAMS");
    appSettings.setValue("BaseLength",arg1);//cyp
    appSettings.endGroup();
    if(ui->checkBoxBase->isChecked()){
        pMain->OnBasePlatePropertiesChanged();
    }
}

void SupportParameter::on_Support_Top_AttachType_comboBox_currentIndexChanged(const QString &arg1)
{
    pMain->OnSupport_Top_AttachType_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){

//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("TopAttachShape",arg1);
        appSettings.endGroup();
        m_pSettings->m_topAttachShape = arg1;
    }
}

void SupportParameter::on_Support_Top_Length_doubleSpinBox_valueChanged(double arg1)
{
    pMain->OnSupport_Top_Length_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){

//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("TopLength",arg1);
        appSettings.endGroup();
        m_pSettings->m_dTopLength = arg1;
    }
}

void SupportParameter::on_Support_Top_Radius_doubleSpinBox_valueChanged(double arg1)
{
    pMain->OnSupport_Top_Radius_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){

//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("TopRadius",arg1);
        double r = appSettings.value("TopRadius").toDouble();//
        m_pSettings->m_dTopRadius = arg1;
        appSettings.endGroup();
    }
}

void SupportParameter::on_Support_Top_Penetration_horizontalSlider_valueChanged(int value)
{
    ui->Support_Top_Penetration_label->setText(QString::number(ui->Support_Top_Penetration_horizontalSlider->value()*0.01));

    pMain->OnSupport_Top_Penetration_Changed();

    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){
        m_pSettings->m_dTopPenetration = value*0.01;
//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("TopPenetration",value*0.01);//cyp
        appSettings.endGroup();
    }
}

void SupportParameter::on_Support_Top_AngleFactor_horizontalSlider_valueChanged(int value)
{
    ui->Support_Top_AngleFactor_label->setText(QString::number(ui->Support_Top_AngleFactor_horizontalSlider->value()*0.01));

    pMain->OnSupport_Top_AngleFactor_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){
        m_pSettings->m_dTopAngleFactor = value*0.01;
//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("TopAngleFactor",value*0.01);//cyp
        appSettings.endGroup();

    }
}

void SupportParameter::on_Support_Mid_AttachType_comboBox_currentIndexChanged(const QString &arg1)
{
    pMain->OnSupport_Mid_AttachType_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){

//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("MidAttachShape",arg1);
        appSettings.endGroup();
        m_pSettings->m_midAttachShape = arg1;
    }
}

void SupportParameter::on_Support_Mid_Radius_doubleSpinBox_valueChanged(double arg1)
{
    pMain->OnSupport_Mid_Radius_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){

//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("MidRadius",arg1);
        appSettings.endGroup();
        m_pSettings->m_dMidRadius = arg1;
    }
}

void SupportParameter::on_Support_Bottom_AttachType_comboBox_currentIndexChanged(const QString &arg1)
{
    pMain->OnSupport_Bottom_AttachType_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD")
    {
//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("BottomAttachShape",arg1);
        appSettings.endGroup();
        m_pSettings->m_bottomAttachShape = arg1;
    }
}

void SupportParameter::on_Support_Bottom_Length_doubleSpinBox_valueChanged(double arg1)
{
    pMain->OnSupport_Bottom_Length_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){

//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("BottomLength",arg1);
        appSettings.endGroup();
        m_pSettings->m_dBottomLength = arg1;
    }
}

void SupportParameter::on_Support_Bottom_Radius_doubleSpinBox_valueChanged(double arg1)
{
    pMain->OnSupport_Bottom_Radius_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){

//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("BottomRadius",arg1);
        appSettings.endGroup();
        m_pSettings->m_dBottomRadius = arg1;
    }
}

void SupportParameter::on_Support_Bottom_Penetration_horizontalSlider_valueChanged(int value)
{
    ui->Support_Bottom_Penetration_label->setText(QString::number(ui->Support_Bottom_Penetration_horizontalSlider->value()*0.01));

    pMain->OnSupport_Bottom_Penetration_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){
        m_pSettings->m_dBottomPenetration = value*0.01;
//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("BottomPenetration",value*0.01);//cyp
        appSettings.endGroup();
    }
}

void SupportParameter::on_Support_Bottom_AngleFactor_horizontalSlider_valueChanged(int value)
{
    ui->Support_Bottom_AngleFactor_label->setText(QString::number(ui->Support_Bottom_AngleFactor_horizontalSlider->value()*0.01));

    pMain->OnSupport_Bottom_AngleFactor_Changed();
    if(pMain->pWorldView->GetTool() == "SUPPORTADD"){
        m_pSettings->m_dBottomAngleFactor = value*0.01;
//        QSettings appSettings("supportsettings");
        QSettings appSettings;
        appSettings.beginGroup("USERSUPPORTPARAMS");
        appSettings.setValue("BottomAngleFactor",value*0.01);//cyp
        appSettings.endGroup();
    }
}


void SupportParameter::on_pushButtonAutoSupport_clicked()
{
    pMain->pWorldView->OnToolAutoAddSupport();
}
