#ifndef CPROJECTCONFIGDIALOG_H
#define CPROJECTCONFIGDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QStackedWidget>

#include "cprojectconfigpages.h"
#include "cproject.h"

class CProjectConfigDialog : public QDialog
{
    Q_OBJECT;

public:
    CProjectConfigDialog(CProject *project, QWidget *parent = 0);

public slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
	void ok();

private:
	void setUi();

	CProject *cproject;
	BasicPage *basicPage;
	IncludePage *includePage;
	LibraryPage *libraryPage;
	MainSourceDependPage *mainSourceDependPage;
	QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
};

#endif
