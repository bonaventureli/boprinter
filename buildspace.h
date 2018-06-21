#ifndef BUILDSPACE_H
#define BUILDSPACE_H

#include <QWidget>

namespace Ui {
class BuildSpace;
}

class BuildSpace : public QWidget
{
    Q_OBJECT
    
public:
    explicit BuildSpace(QWidget *parent = 0);
    ~BuildSpace();
    
private:
    Ui::BuildSpace *ui;
};

#endif // BUILDSPACE_H
