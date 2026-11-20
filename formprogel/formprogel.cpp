#include "formprogel.h"
#include "ui_formprogel.h"

FormProgEl::FormProgEl(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormProgEl)
{
    ui->setupUi(this);
    loadSettings();

    copyId=-1;
    copyGrp=-1;

    ui->comboBoxGroup->setModel(RelModels::instance()->getModel("owens_groups"));
    colVal defGroup;
    defGroup.val=0;
    ui->comboBoxGroup->setCurrentData(defGroup);
    ui->comboBoxGroup->setEditable(false);

    ui->comboBoxOven->setModel(RelModels::instance()->getModel("owens"));
    colVal defOven;
    defOven.val=1;
    ui->comboBoxOven->setCurrentData(defOven);
    ui->comboBoxOven->setEditable(false);

    modelElDim=RelModels::instance()->getModel("els_dims");
    modelElDim->setHeaderData(1,Qt::Horizontal,tr("Марка, диаметр"),Qt::EditRole);

    ui->tableViewElDim->setModel(modelElDim);
    ui->tableViewElDim->setColumnHidden(0,true);
    ui->tableViewElDim->setColumnWidth(1,200);

    modelProg = new RestTableModel("dry_reg",this);
    ui->tableViewProg->setModel(modelProg);

    modelProgData = new RestTableModel("dry_prog",this);
    ui->tableViewProgData->setModel(modelProgData);

    modelProc = new ProgDataModel(this);
    ui->tableViewProc->setModel(modelProc);

    connect(ui->tableViewElDim->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(selectElDim(QModelIndex)));
    connect(ui->tableViewProg->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(selectProg()));
    connect(ui->comboBoxGroup,SIGNAL(currentIndexChanged(int)),this,SLOT(selectProg()));
    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),this,SLOT(upd()));
    connect(modelElDim,SIGNAL(refreshFinished(QString)),this,SLOT(updFinished()));
    connect(modelProg,SIGNAL(sigRefresh()),this,SLOT(updProgFinished()));
    connect(modelProgData,SIGNAL(sigAboutToBeInsert()),this,SLOT(setCurrentStep()));
    connect(ui->pushButtonCopy,SIGNAL(clicked(bool)),this,SLOT(copy()));
    connect(ui->pushButtonPaste,SIGNAL(clicked(bool)),this,SLOT(paste()));
    connect(ui->comboBoxOven,SIGNAL(currentIndexChanged(int)),this,SLOT(updProc()));
    connect(ui->lineEditT0,SIGNAL(textChanged(QString)),this,SLOT(updProc()));
    connect(ui->lineEditW0,SIGNAL(textChanged(QString)),this,SLOT(updProc()));
    connect(modelProgData,SIGNAL(sigUpd()),this,SLOT(updProc()));
    if (modelElDim->rowCount()){
        updFinished();
    }
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

void FormProgEl::upd()
{
    QByteArray data;
    RestConnection::instance()->sendSyncRequest("api/proc/ovens/updeldim","POST","{}",data);
    modelElDim->refresh();
    RelModels::instance()->getModel("owens_groups")->refresh();
    RelModels::instance()->getModel("owens")->refresh();
}

void FormProgEl::updFinished()
{
    if (modelElDim->rowCount()){
        if (currentId.isNull()){
            ui->tableViewElDim->selectRow(0);
        } else {
            int pos = modelElDim->findRowByKey(currentId);
            if (pos>0){
                ui->tableViewElDim->selectRow(pos);
            } else {
                ui->tableViewElDim->selectRow(0);
            }
        }
    }
}

void FormProgEl::selectElDim(QModelIndex index)
{
    if (index.row()<0){
        modelProg->clear();
        modelProgData->clear();
        modelProc->clear();
        selectProg();
        return;
    }
    currentId=modelElDim->getModelData(index.row()).val;
    modelProg->setFilter(RestFilter::rule(modelProg->tableName(),"ide",RestFilter::Op::Eq,currentId.toString()));
    modelProg->setDefaultValue("ide",currentId);
    modelProg->select();
}

void FormProgEl::updProgFinished()
{
    if(modelProg->rowCount()){
        ui->tableViewProg->setCurrentIndex(modelProg->index(0,modelProg->columnIndex("nam")));
    }
}

void FormProgEl::selectProg()
{
    int index=ui->tableViewProg->currentIndex().row();
    QVariant id = modelProg->getModelData(index,"id");
    bool ok = (index>=0) && !id.isNull();
    ui->groupBoxProg->setEnabled(ok);
    modelProgData->setDefaultValue("step",1);
    if (!ok){
        modelProgData->clear();
        modelProc->clear();
        ui->groupBoxProg->setTitle(tr("Программа не выбрана"));
        return;
    }
    ui->groupBoxProg->setTitle(modelProg->getModelData(index,"nam").toString());
    int id_prog = id.toInt();
    int id_grp = ui->comboBoxGroup->getCurrentData().val.toInt();
    RestFilter filter(RestFilter::Group::And);
    filter.addRule(modelProgData->tableName(),"id",RestFilter::Op::Eq,id_prog);
    filter.addRule(modelProgData->tableName(),"id_grp",RestFilter::Op::Eq,id_grp);
    modelProgData->setFilter(filter);
    modelProgData->setDefaultValue("id",id_prog);
    modelProgData->setDefaultValue("id_grp",id_grp);
    modelProgData->select();

    updProc();
}

void FormProgEl::setCurrentStep()
{
    int old_step = modelProgData->getModelData(modelProgData->rowCount()-1,"step").toInt();
    modelProgData->setDefaultValue("step",old_step+1);
}

void FormProgEl::copy()
{
    QVariant id_prg=modelProg->getModelData(ui->tableViewProg->currentIndex().row(),"id");
    if (!id_prg.isNull()){
        copyId=id_prg.toInt();
        copyGrp=ui->comboBoxGroup->getCurrentData().val.toInt();
        ui->pushButtonPaste->setEnabled(true);
    }
}

void FormProgEl::paste()
{
    QVariant id_prg=modelProg->getModelData(ui->tableViewProg->currentIndex().row(),"id");
    if (copyId>=0 && copyGrp>=0 && !id_prg.isNull()){
        QVariant id_grp=ui->comboBoxGroup->getCurrentData().val;
        QJsonObject obj;
        obj.insert("id_src",copyId);
        obj.insert("id_grp_src",copyGrp);
        QJsonDocument doc;
        doc.setObject(obj);
        QByteArray data;
        bool ok = RestConnection::instance()->sendSyncRequest("api/proc/ovens/prog/paste/"+id_prg.toString()+"/"+id_grp.toString(),"POST",doc.toJson(),data);
        if (ok){
            modelProgData->select();
            updProc();
        }
    }
}

void FormProgEl::updProc()
{
    int index=ui->tableViewProg->currentIndex().row();
    QVariant id = modelProg->getModelData(index,"id");
    bool ok = (index>=0) && !id.isNull();
    if (!ok){
        modelProc->clear();
        return;
    }
    int id_prog = id.toInt();
    double v0=ui->lineEditW0->text().toDouble();
    double t0=ui->lineEditT0->text().toDouble();
    int id_oven=ui->comboBoxOven->getCurrentData().val.toInt();
    modelProc->load(id_prog,v0,t0,id_oven);
}