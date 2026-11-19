#include "formprogel.h"
#include "ui_formprogel.h"

FormProgEl::FormProgEl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormProgEl)
{
    ui->setupUi(this);
    loadSettings();

    currentId="";

    modelElDim=RelModels::instance()->getModel("els_dims");
    modelElDim->setHeaderData(1,Qt::Horizontal,tr("Марка, диаметр"),Qt::EditRole);

    ui->tableViewElDim->setModel(modelElDim);
    ui->tableViewElDim->setColumnHidden(0,true);
    ui->tableViewElDim->setColumnWidth(1,200);
}

FormProgEl::~FormProgEl()
{
    saveSettings();
    delete ui;
}

void FormProgEl::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->ui->splitter->restoreState(settings.value("el_prog_splitter_width").toByteArray());
}

void FormProgEl::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("el_prog_splitter_width",ui->splitter->saveState());
}

void FormProgEl::updFinished()
{
    if (currentId.isEmpty() && modelElDim->rowCount()){
        ui->tableViewElDim->selectRow(0);
    }
}