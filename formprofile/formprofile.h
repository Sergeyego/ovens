#ifndef FORMPROFILE_H
#define FORMPROFILE_H

#include <QWidget>
#include "rest/restrotablemodel.h"
#include "progressreportdialog/progressreportdialog.h"

namespace Ui {
class FormProfile;
}

class FormProfile : public QWidget
{
    Q_OBJECT

public:
    explicit FormProfile(QWidget *parent = nullptr);
    ~FormProfile();

private:
    Ui::FormProfile *ui;
    RestRoTableModel *modelProfile;
    ProgressReportDialog *prd;

private slots:
    void upd();
};

#endif // FORMPROFILE_H
