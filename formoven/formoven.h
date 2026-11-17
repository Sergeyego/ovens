#ifndef FORMOVEN_H
#define FORMOVEN_H

#include <QWidget>
#include "rest/resttablemodel.h"
#include "rest/restmapper.h"
#include "colordelegate.h"

namespace Ui {
class FormOven;
}

class FormOven : public QWidget
{
    Q_OBJECT

public:
    explicit FormOven(QWidget *parent = nullptr);
    ~FormOven();

private:
    Ui::FormOven *ui;
    RestTableModel *modelOven;
    RestTableModel *modelChannel;
    RestTableModel *modelSensor;
    RestMapper *mapper;
    void loadSettings();
    void saveSettings();

private slots:
    void setCurrentOven(int index);
    void setCurrentOvenSensor(QModelIndex index);

};

#endif // FORMOVEN_H
