#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QtCore/QProcess>
#include <QtGui/QX11EmbedContainer>
#include <QtGui/QWidget>

class TerminalWidget : public QWidget
{
	Q_OBJECT;
	
public:
	TerminalWidget(const QString &workingDir = QString(), QWidget *parent = 0);
	bool isRunning();
	QString workingDirectory();
					
public slots:
	void start();
	void execute(const QString &command);
	bool close();
				
private slots:
	void processFinished();
	
signals:
	void terminalClosed(TerminalWidget *terminalWidget);
	
private:
	QX11EmbedContainer *container;
	QProcess *process;
	QStringList argument;
	QString workingDir;
};

#endif
