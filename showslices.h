#ifndef SHOWSLICES_H
#define SHOWSLICES_H

#include <QWidget>
#include "b9edit/DrawingContext.h"
#include "crushbitmap.h"

class DrawingContext;
namespace Ui {
class ShowSlices;
}

class ShowSlices : public QWidget
{
    Q_OBJECT
    
public:
    explicit ShowSlices(/*CrushedPrintJob *cPJ,*/QWidget *parent = 0);
    ~ShowSlices();
    void UpdateWidgets();
    DrawingContext* pDrawingContext; // a pointer to the drawing widget

signals:
    void eventHiding();
    void selectedDirChanged(QString s);
    void setVersion(QString s);
    void setName(QString s);
    void setDescription(QString s);
    void setXYPixel(QString s);
    void setZLayer(QString s);
    void setSliceIndicator(QString s);

protected:
    void keyPressEvent(QKeyEvent * pEvent);		// Handle key Press events
    void keyReleaseEvent(QKeyEvent * pEvent);		// Handle key Release events
    void mouseReleaseEvent(QMouseEvent * pEvent);	// Handle mouse button released events


public:
    void hideEvent(QHideEvent *event){}
    Ui::ShowSlices *ui;
    QImage topImg;
    QImage botImg;

    QTimer greenTimer;

    QList<QImage> imgBackup;//list of images for undo, redo
    int backupIndx;

//    ShowSlices* pEditView;

    //pointer to the aboutbox
//    aboutbox* pAboutBox;

public:
    bool modified;
    bool supportMode;
    int currSlice;

    QString sVersion;
    QString sName;
    QString sDescription;
    QString XYPixel;
    QString ZLayer;
    QString sDirPath;

    bool dirtied;
    bool continueLoading;
    QString currJobFile;

    CrushedPrintJob* pCPJ;
    bool bGrid;				// if true, grid is to be drawn
    int m_xOffset, m_yOffset;

public slots:
    void setCurNumOfLayers(int);
    void GoToSlice(int slicenumber);
    void DeCompressIntoContext();
    void RefreshContext(bool alreadywhite);
    void AddSupport(QPoint pos, int size, SupportType type, int fastmode){}
    void SaveToUndoBuffer(){}
    void ReCompress(){}
    void RefreshWithGreen(){}

private slots:
    void on_spinBoxCurNumOfLayers_valueChanged(int arg1);
};

#endif // SHOWSLICES_H

