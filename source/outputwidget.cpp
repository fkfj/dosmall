#include <QtCore/QDebug>
#include "outputwidget.h"
#include "build/builderwidget.h"

OutputWidget::OutputWidget(QWidget *parent)
	: QTabWidget(parent)
{
	setTabsClosable(true);
	setMovable(true);

	connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closePage(int)));
}
	
void OutputWidget::closePage(int index)
{
	qDebug() << "index = " << index;
	// BuilderWidget *builderWidget = qobject_cast<BuilderWidget*>(widget(index));
	// if (builderWidget->close()) {
		// qDebug() << "remove";
		// removeTab(index);
	// }

	if (widget(index)->close())
		qDebug() << "remove";
}


