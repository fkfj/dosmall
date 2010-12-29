#ifndef OUTPUTWIDGET_H
#define OUTPUTWIDGET_H

#include <QtGui/QTabWidget>

class OutputWidget : public QTabWidget
{
	Q_OBJECT;
	
public:
	OutputWidget(QWidget *parent = 0);

private slots:
	void closePage(int index);
};

#endif
