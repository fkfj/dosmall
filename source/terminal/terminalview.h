#ifndef TERMINALVIEW_H
#define TERMINALVIEW_H

#include <QtGui/QTabWidget>
#include <QtGui/QIcon>

#include "terminalwidget.h"

class TerminalView : public QTabWidget
{
	Q_OBJECT;

public:
	TerminalView(QWidget *parent = 0);

public slots:
	void closePage(int index);
	void closePage(TerminalWidget *terminalWidget);
	void newTerm(const QString &workingDir = QString());
	void newTerm(const QString &workingDir, const QString &command);

private:
	int termId;
	QIcon icon;
};

#endif
