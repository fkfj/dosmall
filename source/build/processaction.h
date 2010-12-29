#ifndef PROCESSACTION_H
#define PROCESSACTION_H

#include <QtGui/QAction>
#include <QtCore/QProcess>

class ProcessAction : public QAction
{
	Q_OBJECT;
	
public:
	ProcessAction(const QString &workingDir, const QString &execFile, const QString &args = QString(),
				  const QStringList &ldPathList = QStringList() , QObject *parent = 0);

public slots:
	void startProcess();
	void exitProcess();

private slots:
	void processStarted();
	void processFinished();
	
signals:
	void started(ProcessAction *processAction);
	void finished(ProcessAction *processAction);

private:
	QString workingDirectroy;
	QString executeFile;
	QString args;
	QString ldPaths;
	QProcess *process;
};

#endif
