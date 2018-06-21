#include "materialscatalog.h"
#include "ui_materialscatalog.h"

MaterialsCatalog::MaterialsCatalog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MaterialsCatalog)
{
    ui->setupUi(this);
}

MaterialsCatalog::~MaterialsCatalog()
{
    delete ui;
}
