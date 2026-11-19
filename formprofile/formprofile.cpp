#include "formprofile.h"
#include "ui_formprofile.h"

FormProfile::FormProfile(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormProfile)
{
    ui->setupUi(this);

    ui->dateEditBeg->setDate(QDate::currentDate().addDays(-QDate::currentDate().day() + 1));
    ui->dateEditEnd->setDate(QDate::currentDate().addDays(1));

    modelProfile = new RestRoTableModel(this);
    ui->tableView->setModel(modelProfile);

    prd = new ProgressReportDialog(this);

    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),this,SLOT(upd()));
    connect(ui->pushButtonViewXlsx,SIGNAL(clicked(bool)),ui->tableView,SLOT(viewExcel()));
    connect(ui->pushButtonSaveXlsx,SIGNAL(clicked(bool)),ui->tableView,SLOT(saveXlsx()));
    connect(modelProfile,SIGNAL(sigRefresh()),prd,SLOT(hide()));
}

FormProfile::~FormProfile()
{
    delete ui;
}

void FormProfile::upd()
{
    modelProfile->setPath("api/proc/ovens/profile/"+ui->dateEditBeg->date().toString("yyyy-MM-dd")+"/"+ui->dateEditEnd->date().toString("yyyy-MM-dd"));
    modelProfile->select();
    prd->show();
}
