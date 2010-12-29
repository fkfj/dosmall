#include <QtCore/QDebug>
#include <QtGui/QMessageBox>
#include "buildoutput.h"
#include "builderwidget.h"

BuildOutput::BuildOutput(QWidget *parent)
	: QTabWidget(parent)
{
	setTabsClosable(true);
	setMovable(true);

	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closePage(int)));
}

void BuildOutput::closePage(int index)
{
	BuilderWidget *builderWidget = qobject_cast<BuilderWidget*>(widget(index));
	if (!builderWidget) {
		removeTab(index);
	} else if (builderWidget->close()) {
		removeTab(index);
		delete builderWidget;
	}
}


bool BuildOutput::quit()
{
	int tabCount = count();
	if (tabCount < 0)
		return true;
	
	BuilderWidget *builderWidget = 0;
	for (int i = 0; i < tabCount; ++i) {
		builderWidget = qobject_cast<BuilderWidget*>(widget(i));
		if (builderWidget) {
			if (builderWidget->isBuilding() || builderWidget->hasRunningProcess()) {
				int ret = QMessageBox::warning(this, "有程序正在编译或运行", "真的要关闭吗?",
											   QMessageBox::Ok | QMessageBox::Cancel,
											   QMessageBox::Cancel);
				if (ret == QMessageBox::Ok)
					return true;
				else
					return false;
			}
		}
	}
	return true;
}


