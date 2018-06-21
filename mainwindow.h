#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <supportsetting.h>
#include <showslices.h>
#include <materialscatalog.h>
#include <cycleprintsetting.h>
#include <buildspace.h>
#include <terminal.h>
#include <printwindow.h>
#include <print.h>

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QSplashScreen>
#include "logfilemanager.h"
#include "helpsystem.h"

#include "b9layout/worldview.h"


#include "modellist.h"
#include "supportsetting.h"
#include "supportparameter.h"

class ShowSlices;
class MainWindow;
class b9PrinterModelManager;
class B9UpdateManager;
class B9LayoutProjectData;
class WorldView;
class ModelData;
class B9ModelInstance;
class SliceDebugger;
class B9SupportStructure;
class SupportParameter;
class ModelList;
class PrintWindow;

//namespace Ui {
//class MainWindow;
//}

class PSupportSettings {
public:
    PSupportSettings(){loadSettings();}
    ~PSupportSettings(){}
    void updateValues(); // Opens a dialog and allows user to change settings

    void loadSettings();
    void saveSettings();
    void setFactorySettings();

    double m_dBaseRadius, m_dTopRadius, m_dMidRadius, m_dBottomRadius;
    double m_dBaseLength, m_dTopLength, m_dMidLength, m_dBottomLength;
    QString m_baseAttachShape, m_topAttachShape, m_midAttachShape, m_bottomAttachShape;
    double m_dTopPenetration, m_dBottomPenetration;
    double m_dTopAngleFactor, m_dBottomAngleFactor;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void InitInterface();
    std::vector<B9ModelInstance*> GetAllInstances();
    std::vector<B9ModelInstance*> GetSelectedInstances();
    std::vector<ModelData*> GetAllModelData(){return ModelDataList;}
    B9LayoutProjectData* ProjectData(){return project;}

signals:
    void eventHiding();
    void selectedDirChanged(QString s);
    void setVersion(QString s);
    void setName(QString s);
    void setDescription(QString s);
    void setXYPixel(QString s);
    void setZLayer(QString s);
    void setSliceIndicator(QString s);



public slots:

    //debug interface
    void OpenDebugWindow();

    //file
    void New();
    QString Open(bool withoutVisuals = false);
    void Save();
    void SaveAs();

    //interface
    void OnChangeTab(int idx);
    void SetXYPixelSizePreset(QString size);
    void SetZLayerThickness(QString thick);
    void SetProjectorX(QString);
    void SetProjectorY(QString);
    void SetProjectorPreset(int index);
    void SetZHeight(QString z);
    void SetAttachmentSurfaceThickness(QString num);
    void UpdateBuildSpaceUI();
    void UpdateExtentsUI();

    void BuildInterface(){}

    void UpdateInterface();//sets the translation interface fields acourding to what instance/instances are selected.
    void PushTranslations();
    void OnModelSpinSliderChanged(int val);//when the spin slider changes value by the user.
    void OnModelSpinSliderReleased();
    void LockScale(bool lock);

    //TOOLS
    void SetTool(QString toolname);//calls the functions below

    //ModelTools interface
    void SetToolModelSelect();
    void SetToolModelMove();
    void SetToolModelSpin();
    void SetToolModelOrientate();
    void SetToolModelScale();

    //SupportTools interface
    void SetToolSupportModify();
    void SetToolSupportAdd();
    void SetToolSupportDelete();

    void ExitToolAction();//use to panic out of a mouse down tool action.

    //contour aid
    void OnToggleContourAid(bool tog);
    bool ContourAidEnabled();

    //xray vision
    void OnToggleXRay(bool tog);
    void OnXRayChange(int val);
    bool XRayEnabled();

    //support hiding
    bool HidingSupports();
    void OnToggleSupportHiding(bool tog);

    //model
    B9ModelInstance* AddModel(QString filepath = "", bool bypassVisuals = false);
    void RemoveAllInstances();
    void CleanModelData();//cleans andy modeldata that does not have a instance!
    void AddTagToModelList(QListWidgetItem* item);

    B9ModelInstance* FindInstance(QListWidgetItem* item);//given a item you can find the connected instance

    //selection
    void RefreshSelectionsFromList();//searches through all active listitems and selects their corresponding instance;
    void Select(B9ModelInstance* inst);//selects the given instance
    void DeSelect(B9ModelInstance* inst);//de-selects the given instance
    void SelectOnly(B9ModelInstance* inst);//deselects eveything and selects only the instance
    void DeSelectAll();//de-selects all instances
    void SetSelectionPos(double x, double y, double z, int axis = 0);
    void SetSelectionRot(QVector3D newRot);
    void SetSelectionScale(double x, double y, double z, int axis = 0);
    void SetSelectionFlipped(bool flipped);
    void DropSelectionToFloor();
    void ResetSelectionRotation();
    void DuplicateSelection();
    void DeleteSelection();//delete whatever is selected - support or instance..
    void DeleteSelectedInstances();

    //Support Mode
    void SetSupportMode(bool tog);//Sets everything up for editing supports for the selected instance.
                            //when either the tab is clicked or the menu item - a selected instance
                            //can be assumed
    void FillSupportList();//adds the elements to the list
    void UpdateSupportList();//selects the correct elements in the list based off of real selection
    B9SupportStructure* FindSupportByName(QString name);
    void RefreshSupportSelectionsFromList();//Called when the user selects stuff in the support list

    void SelectSupport(B9SupportStructure* sup);
    void SelectOnly(B9SupportStructure* sup);//deselects eveything and selects only the instance
    std::vector<B9SupportStructure*>* GetSelectedSupports();
    bool IsSupportSelected(B9SupportStructure* sup);
    void DeSelectSupport(B9SupportStructure* sup);
    void DeSelectAllSupports();
    void DeleteSelectedSupports();//called from remove button.
    void DeleteSupport(B9SupportStructure* pSup);
    void MakeSelectedSupportsVertical();
    void MakeSelectedSupportsStraight();

    //Support Interface
    void OnSupport_Top_AttachType_Changed(bool updateInterface = true);
    void OnSupport_Top_Radius_Changed(bool updateInterface = true);
    void OnSupport_Top_Length_Changed(bool updateInterface = true);
    void OnSupport_Top_Penetration_Changed(bool updateInterface = true);
    void OnSupport_Top_AngleFactor_Changed(bool updateInterface = true);
    void OnSupport_Mid_AttachType_Changed(bool updateInterface = true);
    void OnSupport_Mid_Radius_Changed(bool updateInterface = true);
    void OnSupport_Bottom_AttachType_Changed(bool updateInterface = true);
    void OnSupport_Bottom_Radius_Changed(bool updateInterface = true);
    void OnSupport_Bottom_Length_Changed(bool updateInterface = true);
    void OnSupport_Bottom_Penetration_Changed(bool updateInterface = true);
    void OnSupport_Bottom_AngleFactor_Changed(bool updateInterface = true);

    //Foundation (BasePlate)
    void OnBasePlatePropertiesChanged();


    void PushSupportProperties();//fills the support properties with relevent data.
    void PushBasePlateProperties();
    void ResetSupportLight();//connected to push button
    void ResetSupportMedium();//connected to push button
    void ResetSupportHeavy();//connected to push button
    void FillSupportParamsWithDefaults();

    //returns a valid instance if we are editing it in support mode.
    B9ModelInstance* SupportModeInst();

    //slicing
    bool SliceWorld();//prompts the user to slice to world to different formats.
    bool SliceWorldToJob(QString filename);//slices to whole world to a job file
    bool SliceWorldToSlc(QString filename);//slices to whole world to a slc file
    void CancelSlicing(); //connected to the progress bar to stop slicing.

    //exporting
    void PromptExportToSTL();//export the whole layout to a stl file.
        bool ExportToSTL(QString filename);



    //events
    void keyPressEvent(QKeyEvent * event );
    void keyReleaseEvent(QKeyEvent * event );
    void mouseReleaseEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);

    void doPrint();

    void doSlice();
    void RemoveAllSupports();
    void showPrintWindow();
    void showCatalog();
    void showSliceWindow();

public:
    Ui::MainWindow ui;
    BuildSpace *bs;
    Terminal *t;
    PrintWindow *pw;
    Print *pPrint;
    ShowSlices *pSliceView;
    WorldView *pWorldView;

//    QSlider* pXRaySlider;
    B9LayoutProjectData* project;

    CrushedPrintJob cPJ;

    CrushedPrintJob *m_pCPJ;
    PSupportSettings *pSettings;

    std::vector<ModelData*> ModelDataList;

    bool cancelslicing;

    //support mode
    B9ModelInstance* currInstanceInSupportMode;
    bool oldModelConstricted;//for raising models that are too close to the ground in support mode.
    QVector3D oldPan;
    QVector3D oldRot;
    float oldZoom;
    QString oldTool;
    bool useContourAid;
    bool useXRayVision;
    bool hideSupports;
    std::vector<B9SupportStructure*> currSelectedSupports;//what supports are currently in selection.

    QPushButton *pButtonRotate;
    QPushButton *pButtonMove;
    QPushButton *pButtonScale;
    QAction *pActionSupport;
//    QSlider* pBaseRadiusSlider;

    QPixmap *pixmap;
    QIcon *icon;

    ModelList *pModelList;
    SupportParameter *pSupport;

    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent * event );
    void contextMenuEvent(QContextMenuEvent *event);


private slots:

private slots:
    void on_thicknesscombo_currentIndexChanged(const QString &arg1);

    void on_doubleSpinBoxBaseRadius_valueChanged(double arg1);
    void on_ApplyButton_clicked();
    void on_pushButtonSupportSettings_clicked();
    void OnBaseRadiusChange(int iValue);
};

#endif // MAINWINDOW_H
