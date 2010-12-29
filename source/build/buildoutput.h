#ifndef BUILDOUTPUT_H
#define BUILDOUTPUT_H

#include <QtGui/QTabWidget>

class BuildOutput : public QTabWidget
{
	Q_OBJECT;
	
public:
	BuildOutput(QWidget *parent = 0);
	bool quit();

private slots:
	void closePage(int index);
};

#endif
