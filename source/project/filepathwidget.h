#ifndef FILEPATHWIDGET_H
#define FILEPATHWIDGET_H

#include <QtCore/QStringList>
#include <QtGui/QAction>
#include <QtGui/QFileDialog>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTableView>

class FilePathDialog;

class FilePathWidget : public QWidget
{
  Q_OBJECT;

public:
	FilePathWidget(const QString &labelText, const QString &prefix, bool selectDir, QWidget *parent = 0);
	void add(const QStringList &list);
	QStringList getList();
	bool isModified();
	void setModification(bool b);
			  
private slots:
	void add();
	void del();
	void edit();
	void up();
	void down();
	
protected:
	QString prefix;
	bool selectDir;
	bool modified;

	QTableView *tableView;
	QStandardItemModel *model;
	QLabel *headerLabel;
	QAction *action_Add;
	QAction *action_Del;
	QAction *action_Edit;
	QAction *action_Up;
	QAction *action_Down;
	QStringList filePathList;
	FilePathDialog *filePathDialog;
};

class FilePathDialog : public QDialog
{
  Q_OBJECT;

public:
	FilePathDialog(const QString &title = "FilePathDialog", const QString &text = "",
				   QWidget *parent = 0);
	
	QStringList getDir(const QString &title, const QString &text);
	QStringList getFiles(const QString &title, const QString &text);

private slots:
	void fileSystemSlot();
	void okSlot();
	void cancelSlot();
	
protected:
	QStringList filePathList;
	QLineEdit *lineEdit;
	QPushButton *fileSystem;
	QPushButton *ok;
	QPushButton *cancel;
	bool selectDir;
};

#endif
