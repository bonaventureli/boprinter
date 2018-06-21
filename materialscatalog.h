#ifndef MATERIALSCATALOG_H
#define MATERIALSCATALOG_H

#include <QWidget>

namespace Ui {
class MaterialsCatalog;
}

class MaterialsCatalog : public QWidget
{
    Q_OBJECT
    
public:
    explicit MaterialsCatalog(QWidget *parent = 0);
    ~MaterialsCatalog();
    
private:
    Ui::MaterialsCatalog *ui;
};

#endif // MATERIALSCATALOG_H
