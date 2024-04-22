#ifndef COMPLETEROADSWIDGET_HPP
#define COMPLETEROADSWIDGET_HPP

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QComboBox>
#include "cehuidatainfo.hpp"

class CompleteRoadsWidget : public QWidget {
    Q_OBJECT
public:
    CompleteRoadsWidget(QWidget *parent = nullptr);

signals:
    void itemCheckBoxChanged_signal(const QString& id, int status);

    void exportCompleteRoads_signal(const QString& groupid);

public slots:
    void updateCheckedItems(const std::vector<cehuidataInfo>& cehuidataInfoList);
    void OnItemChanged(QTreeWidgetItem* item,int column);
    void updateGroupidComboBox(const QStringList &listItems);

private slots:
    void submitCheckedItems();

private:
    QTreeWidget* m_treeWidget;
    int m_idIndexInTreeWidget;
    int m_nameIndexInTreeWidget;
    int m_checkedIndexInTreeWidget;

    QComboBox *m_groupidComboBox;
};

#endif //COMPLETEROADSWIDGET_HPP