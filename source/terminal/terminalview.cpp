#include <QtCore/QDir>
#include <QtCore/QVariant>

#include "terminalview.h"

TerminalView::TerminalView(QWidget *parent)
	: QTabWidget(parent)
{
	termId = 1;
	icon.addFile(QString::fromUtf8(":/images/terminal.png"),
							QSize(), QIcon::Normal, QIcon::On);
	setMovable(true);
	setTabsClosable(true);
	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closePage(int)));
}

void TerminalView::closePage(int index)
{
	if (index < 0 || index >= count())
		return;

	TerminalWidget *term = qobject_cast<TerminalWidget *>(widget(index));
	if (term) {
		term->close();
		disconnect(term, 0, this, 0);
	}
	removeTab(index);
}

void TerminalView::closePage(TerminalWidget *terminalWidget)
{
	int index = indexOf(terminalWidget);
	if (index < 0 || index >= count())
		return;
	terminalWidget->close();
	disconnect(terminalWidget, 0, this, 0);
	removeTab(index);
}

void TerminalView::newTerm(const QString &workingDir)
{
	QString dir;
	if (workingDir.isEmpty()) {
		TerminalWidget *term = qobject_cast<TerminalWidget *>(currentWidget());
		if (term)
			dir = term->workingDirectory();
		else
			dir = QDir::homePath();
	} else {
		dir = workingDir;
	}
	
	TerminalWidget *term = new TerminalWidget(dir);
	connect(term, SIGNAL(terminalClosed(TerminalWidget *)), this, SLOT(closePage(TerminalWidget *)));
	addTab(term, icon, "term<" + QVariant(termId).toString() + ">");
	term->start();
	++termId;
}

void TerminalView::newTerm(const QString &workingDir, const QString &command)
{
	TerminalWidget *term = new TerminalWidget(workingDir);
	connect(term, SIGNAL(terminalClosed(TerminalWidget *)), this, SLOT(closePage(TerminalWidget *)));
	addTab(term, icon, "term<" + QVariant(termId).toString() + ">");
	term->execute(command);
	++termId;
}

