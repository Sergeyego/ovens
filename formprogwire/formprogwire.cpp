#include "formprogwire.h"
#include "ui_formprogwire.h"

FormProgWire::FormProgWire(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormProgWire)
{
    ui->setupUi(this);
    loadSettings();

    modelProg = new RestTableModel("owens_progs",this);
    ui->tableViewProg->setModel(modelProg);

    modelProgData = new RestTableModel("owens_progs_data",this);
    ui->tableViewProgData->setModel(modelProgData);

    mapper = new RestMapper(ui->tableViewProg,this);
    ui->horizontalLayoutMap->insertWidget(0,mapper);

    mapper->addMapping(ui->lineEditProgName,"nam");
    mapper->addLock(ui->pushButtonUpd);

    connect(mapper,SIGNAL(currentIndexChanged(int)),this,SLOT(currentIndexChanged(int)));
    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),modelProg,SLOT(select()));
    connect(modelProgData,SIGNAL(sigAboutToBeInsert()),this,SLOT(setCurrentStep()));

    modelProg->select();
}

FormProgWire::~FormProgWire()
{
    saveSettings();
    delete ui;
}

void FormProgWire::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->ui->splitter->restoreState(settings.value("wire_prog_splitter_width").toByteArray());
}

void FormProgWire::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("wire_prog_splitter_width",ui->splitter->saveState());
}

void FormProgWire::currentIndexChanged(int index)
{
    int id_prog=mapper->modelData(index,"id").isNull() ? -1 : mapper->modelData(index,"id").toInt();
    modelProgData->setFilter(RestFilter::rule(modelProgData->tableName(),"id_prog",RestFilter::Op::Eq,id_prog));
    modelProgData->setDefaultValue("id_prog",id_prog);
    modelProgData->setDefaultValue("step",1);
    modelProgData->select();
}

void FormProgWire::setCurrentStep()
{
    int old_step = modelProgData->getModelData(modelProgData->rowCount()-1,"step").toInt();
    modelProgData->setDefaultValue("step",old_step+1);
}
