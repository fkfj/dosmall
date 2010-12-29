#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include "processaction.h"

ProcessAction::ProcessAction(const QString &workingDir, const QString &execFile, const QString &args,
							 const QStringList &ldPathList, QObject *parent)
	:QAction(parent)
{

	this->workingDirectroy = workingDir;
	this->executeFile = execFile;
	this->executeFile.remove(QRegExp("^./"));
	this->args = args;
	if (!ldPathList.isEmpty()) {
		ldPaths = QString();
	} else {
		ldPaths = ldPathList.join(":") + ":$LD_LIBRARY_PATH";
	}
}

void ProcessAction::startProcess()
{
	process = new QProcess;
	connect(process, SIGNAL(started()), this, SLOT(processStarted()));
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished()));
	// connect(this, SIGNAL(triggered()), process, SLOT(kill()));
	connect(this, SIGNAL(triggered()), this, SLOT(exitProcess()));
	process->setWorkingDirectory(workingDirectroy);
	QStringList termArgv;
	QString command = QString(); // 执行程序的命令
	if (!ldPaths.isEmpty())
		command = "export LD_LIBRARY_PATH=" + ldPaths + " && ";
	// 结束时提示输入Ctrl-C退出， 并循环读取数据, 之所以用循环读取数据而不是只读一行， 是因为用户在执行程序的时候，
	// 可能多输入了数据，这些数据留给了最后的read，当read读取了这些数据之后xterm就退出了，用户就看不到程序的输出。
	// 而用循环读取数据的方式就会将用户输入的多余数据读完并继续读取输入， 直到输入Ctrl-C后xterm才退出。
	command += "./" + executeFile + " " + args + " && echo \"程序执行结束, 按Ctrl-C 退出\" && while true; do read; done";
	termArgv << "-fn" << "-misc-fixed-medium-r-normal--18-120-100-100-c-90-iso10646-1" // xterm设置字体
			 << "-bg" << "black" << "-fg" << "white" // 设置xterm背景色为黑色， 字符颜色为白色
			 << "-title" << executeFile + " " + args // 设置xterm标题栏为[程序名称 程序参数] 
			 << "-e" << command;
	process->start("xterm", termArgv);
}

void ProcessAction::exitProcess()
{
	process->kill();
}

void ProcessAction::processStarted()
{
	emit started(this);
}

void ProcessAction::processFinished()
{
	emit finished(this);
}
