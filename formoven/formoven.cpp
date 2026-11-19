#include "formoven.h"
#include "ui_formoven.h"

FormOven::FormOven(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormOven)
{
    ui->setupUi(this);
    loadSettings();

    RelModels::instance()->getModel("owens_groups")->setEditable(true);

    modelOven = new RestTableModel("owens",this);

    ui->tableViewOven->setModel(modelOven);

    mapper = new RestMapper(ui->tableViewOven,this);
    mapper->addMapping(ui->lineEditId,"id");
    mapper->addMapping(ui->lineEditNam,"num");
    mapper->addMapping(ui->comboBoxGroup,"id_group");
    mapper->addMapping(ui->lineEditPhead,"pwr");
    mapper->addMapping(ui->lineEditPvent,"pwr_cool");
    mapper->addMapping(ui->lineEditType,"tip");
    mapper->addEmptyLock(ui->tableViewChannel);
    mapper->addEmptyLock(ui->tableViewSensor);

    ui->horizontalLayoutMap->insertWidget(0,mapper);

    modelChannel = new RestTableModel("owens_trm_channel",this);

    ui->tableViewChannel->setItemDelegateForColumn(5,new ColorDelegate(this));
    ui->tableViewChannel->setItemDelegateForColumn(6,new ColorDelegate(this));

    ui->tableViewChannel->setModel(modelChannel);


    modelSensor = new RestTableModel("owens_trm_channel_sensor",this);
    ui->tableViewSensor->setModel(modelSensor);

    connect(mapper,SIGNAL(currentIndexChanged(int)),this,SLOT(setCurrentOven(int)));
    connect(ui->tableViewChannel->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(setCurrentOvenSensor(QModelIndex)));
    connect(modelChannel,SIGNAL(sigRefresh()),this,SLOT(channelUpdFinished()));

    modelOven->select();
}

FormOven::~FormOven()
{
    saveSettings();
    delete ui;
}

void FormOven::loadSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    this->ui->splitter->restoreState(settings.value("oven_splitter_width").toByteArray());
}

void FormOven::saveSettings()
{
    QSettings settings("szsm", QApplication::applicationName());
    settings.setValue("oven_splitter_width",ui->splitter->saveState());
}

void FormOven::setCurrentOven(int index)
{
    if (index<0){
        modelChannel->setDefaultValue("id_owen",-1);
        modelChannel->clear();
        modelSensor->setDefaultValue("id_channel",-1);
        modelSensor->clear();
        return;
    }
    int id_oven=mapper->modelData(index,"id").toInt();
    modelChannel->setFilter(RestFilter::rule(modelChannel->tableName(),"id_owen",RestFilter::Op::Eq,id_oven));
    modelChannel->setDefaultValue("id_owen",id_oven);
    modelChannel->select();
}

void FormOven::channelUpdFinished()
{
    if (modelChannel->rowCount()){
        ui->tableViewChannel->selectRow(0);
    } else {
        modelSensor->setDefaultValue("id_channel",-1);
        modelSensor->clear();
    }
}

void FormOven::setCurrentOvenSensor(QModelIndex index)
{
    QVariant id = modelChannel->getModelData(index.row(),"id");
    bool ok = !(id.isNull() || index.row()<0);
    ui->tableViewSensor->setEnabled(ok);
    if (!ok){
        modelSensor->setDefaultValue("id_channel",-1);
        modelSensor->clear();
    } else {
        int id_channel=id.toInt();
        modelSensor->setFilter(RestFilter::rule(modelSensor->tableName(),"id_channel",RestFilter::Op::Eq,id_channel));
        modelSensor->setDefaultValue("id_channel",id_channel);
        modelSensor->select();
    }
}
