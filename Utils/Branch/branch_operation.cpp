#include "branch_operation.h"

#include "ui_branch_operation.h"

BranchOperation::BranchOperation(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::BranchOperation)
{
	ui->setupUi(this);

}

BranchOperation::~BranchOperation()
{

}