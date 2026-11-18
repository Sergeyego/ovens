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

    connect(ui->pushButtonUpd,SIGNAL(clicked(bool)),this,SLOT(upd()));
}

FormProfile::~FormProfile()
{
    delete ui;
}

void FormProfile::upd()
{
    modelProfile->setPath("api/proc/ovens/profile/"+ui->dateEditBeg->date().toString("yyyy-MM-dd")+"/"+ui->dateEditEnd->date().toString("yyyy-MM-dd"));
    modelProfile->select();
}
