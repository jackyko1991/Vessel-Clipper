#ifndef __BRANCHOPERATION_H__
#define __BRANCHOPERATION_H__

// qt
#include <QWidget>

namespace Ui {
	class BranchOperation;
}

class BranchOperation : public QWidget
{
	Q_OBJECT

public:
	BranchOperation(QWidget* parent = nullptr);
	~BranchOperation();

public slots:

private slots :

private:
	Ui::BranchOperation *ui;
};

#endif