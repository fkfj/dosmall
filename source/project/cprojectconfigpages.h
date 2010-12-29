#ifndef CPROJECTCONFIGPAGES_H
#define CPROJECTCONFIGPAGES_H

#include <QtCore/QStringList>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidgetItem>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QStackedWidget>
#include <QtGui/QToolBar>

#include "filepathwidget.h"
#include "cproject.h"

class BasicPage : public QWidget
{
	Q_OBJECT;
	
public:
	BasicPage(CProject *project, QWidget *parent = 0);
	void save();

private slots:
	void loadCCFilePath();
	void loadCCCFilePath();
	void loadMakeFilePath();
	
private:
	void loadSettings();

	CProject *cproject;
	QLineEdit *ccCompilerLineEdit;
	QLineEdit *cccCompilerLineEdit;
	QLineEdit *makeLineEdit;
	QPushButton *ccButton;
	QPushButton *cccButton;
	QPushButton *makeButton;

	QCheckBox *wallCheckBox;
	QCheckBox *wCheckBox;
	QCheckBox *werrorCheckBox;
	
	QComboBox *typeCombo;
	QComboBox *developModeCombo;
	QComboBox *platformCombo;
	QComboBox *stdCombo;

	QLineEdit *otherFlagLineEdit;
	
	QStringList typeList;
	QStringList developModeList;
	QStringList platformList;
	QStringList stdList;
};

class IncludePage : public QWidget
{
	Q_OBJECT;

public:
	IncludePage(CProject *project, QWidget *parent = 0);
	void save();
	
private:
	void loadSettings();
	CProject *cproject;
	FilePathWidget *includePath;
	FilePathWidget *includeFile;
};

class LibraryPage : public QWidget
{
	Q_OBJECT;

public:
	LibraryPage(CProject *project, QWidget *parent = 0);
	void save();
	
private:
	void loadSettings();
	CProject *cproject;
	FilePathWidget *libraryPath;
	FilePathWidget *libraryFile;
};

class MainSourceDependPage : public QWidget
{
	Q_OBJECT;

public:
	MainSourceDependPage(CProject *project, QWidget *parent = 0);
	void save();

private slots:
	void changePage(QListWidgetItem *current, QListWidgetItem *previous);
	void addMainSource();
	void outMainSource();
	void showInfo();
	
private:
	void loadSettings();
	CProject *cproject;
	QStringList infoList;
	QPushButton *infoButton;
	QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
	QAction *action_AddMainSource;
	QAction *action_OutMainSource;
};

class DependSourceWidget : public QWidget
{
	Q_OBJECT;
public:
	enum DependStatus { All, Part, Only };
	
	DependSourceWidget(const QString &rootPath, const QString &labelText, QWidget *parent = 0);
	void addSource(const QStringList &list);
	int dependStatus();
	QStringList getList();
	bool isDependAll();
	bool isDependPart();
	bool isDependOnly();

public slots:
	void dependAll();
	void dependPart();
	void dependOnly();
										   
private slots:
	void addSource();
	void outSource();

private:
	int status;
	QString path;
	QAction *action_AddSource;
	QAction *action_OutSource;
	QButtonGroup *buttonGroup;
	QRadioButton *dependAllButton;
	QRadioButton *dependPartButton;
	QRadioButton *dependOnlyButton;
	QToolBar *toolBar;
	QTableView *tableView;
	QStandardItemModel *model;
};

#endif
