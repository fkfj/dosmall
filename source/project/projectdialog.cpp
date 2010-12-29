#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QDialogButtonBox>

#include <QtCore/QDebug>
#include "projectdialog.h"


DeleteDialog::DeleteDialog(const QString &title, const QString &sureText,
						   QWidget *parent)
	: QDialog(parent)
{
	QLabel *sureLabel = 0;
	sureLabel = new QLabel(sureText);
	checkStateVar = 0;

	QDialogButtonBox *buttonBox =
		new QDialogButtonBox(QDialogButtonBox::Ok
							 | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QHBoxLayout *hbox = new QHBoxLayout();
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addStretch(1);
	hbox->addWidget(buttonBox);

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(sureLabel);
	vbox->addLayout(hbox);
	resize(400, 150);
	setWindowTitle(title);
}

DeleteDialog::DeleteDialog(const QString &title, const QString &sureText,
						   const QString &checkText, QWidget *parent)
	: QDialog(parent)
{
	QLabel *sureLabel = 0;
	sureLabel = new QLabel(sureText);

	QCheckBox *checkBox = 0;
	checkStateVar = 0;
	checkBox = new QCheckBox(checkText, this);
	connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));

	QDialogButtonBox *buttonBox =
		new QDialogButtonBox(QDialogButtonBox::Ok
							 | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QHBoxLayout *hbox = new QHBoxLayout();
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addStretch(1);
	hbox->addWidget(buttonBox);

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(sureLabel);
	vbox->addWidget(checkBox);
	vbox->addLayout(hbox);
	resize(400, 200);
	setWindowTitle(title);
}

int DeleteDialog::checkState()
{
	return checkStateVar;
}

void DeleteDialog::stateChanged(int state)
{
	checkStateVar = state;
}

RenameDialog::RenameDialog(const QString &title, const QStringList &alreadyExistsName,
						   const QString &oldName, QWidget *parent)
	: QDialog(parent), oldName(oldName)
{
	this->alreadyExistsName = alreadyExistsName;
	this->alreadyExistsName.removeAll(oldName);
	checkStateVar = 0;
	
	lineEdit = new QLineEdit(oldName, this);
	warningLabel = new QLabel(this);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
									 QDialogButtonBox::Cancel);

	connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(checkName(const QString &)));
	connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(ok()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(ok()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addStretch(1);
	hbox->addWidget(buttonBox);
	
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(lineEdit);
	vbox->addWidget(warningLabel);
	vbox->addLayout(hbox);
	resize(400, 150);
	setWindowTitle(title);
}

RenameDialog::RenameDialog(const QString &title, const QStringList &alreadyExistsName,
						   const QString &oldName, const QString &checkText, QWidget *parent)
	: QDialog(parent), oldName(oldName)
{
	this->alreadyExistsName = alreadyExistsName;
	this->alreadyExistsName.removeAll(oldName);
	checkStateVar = 0;
	
	lineEdit = new QLineEdit(oldName, this);
	warningLabel = new QLabel(this);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
									 QDialogButtonBox::Cancel);

	connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(checkName(const QString &)));
	connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(ok()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(ok()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QCheckBox *checkBox = 0;
	if (!checkText.isEmpty()) {
		checkBox = new QCheckBox(checkText, this);
		connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
	}
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addStretch(1);
	hbox->addWidget(buttonBox);
	
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(lineEdit);
	vbox->addWidget(warningLabel);
	if (checkBox)
		vbox->addWidget(checkBox);
	vbox->addLayout(hbox);
	resize(400, 150);
	setWindowTitle(title);
}

QString RenameDialog::newName()
{
	return newNameVar;
}

int RenameDialog::checkState()
{
	return checkStateVar;
}

void RenameDialog::ok()
{
	QString text = lineEdit->text();
	if (checkName(text)) {
		text.remove(QRegExp("^\\s*"));
		text.remove(QRegExp("\\s*$"));
		if (text == oldName) {
			reject();
		} else {
			newNameVar = text;
			accept();
		}
	}
}

bool RenameDialog::checkName(const QString &text)
{
	QString str = text;
	str.remove(QRegExp("^\\s*"));
	lineEdit->setText(str);
	str.remove(QRegExp("\\s*$"));
	if (str.isEmpty()) {
		warningLabel->setText("名称不能为空.");
		return false;
	} else if (alreadyExistsName.contains(str)) {
		warningLabel->setText("\"" + str + "\"已经存在.");
		// buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
		return false;
	} else if (str.contains("/") || str.contains("\\") ||
			   str.contains("<") || str.contains(">") ||
			   str.contains("*")){
		warningLabel->setText("名称不能包含\\/<>*");
		// buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
		return false;
	}
	warningLabel->clear();
	return true;
}

void RenameDialog::stateChanged(int state)
{
	checkStateVar = state;
}
