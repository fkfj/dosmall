#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QDialogButtonBox>

class DeleteDialog : public QDialog
{
	Q_OBJECT;
	
public:
	DeleteDialog(const QString &title, const QString &sureText,
				 QWidget *parent = 0);

	DeleteDialog(const QString &title, const QString &sureText,
				 const QString &checkText, QWidget *parent = 0);
	
	int checkState();

private slots:
	void stateChanged(int state);

private:
	int checkStateVar;
};

class RenameDialog : public QDialog
{
	Q_OBJECT;
	
public:
	RenameDialog(const QString &title, const QStringList &alreadyExistsName,
				 const QString &oldName, QWidget *parent = 0);
	
	RenameDialog(const QString &title, const QStringList &alreadyExistsName,
				 const QString &oldName, const QString &checkText,
				 QWidget *parent = 0);
	
	QString newName();
	int checkState();
					
private slots:
	void ok();
	bool checkName(const QString &text);
	void stateChanged(int state);

private:
	QStringList alreadyExistsName;
	QString oldName;
	QString newNameVar;
	int checkStateVar;
	QLineEdit *lineEdit;
	QLabel *warningLabel;
	QDialogButtonBox *buttonBox;
};

#endif
