#ifndef FORMPROFILE_H
#define FORMPROFILE_H

#include <QWidget>
#include "rest/restrotablemodel.h"

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

private slots:
    void upd();
};

#endif // FORMPROFILE_H
