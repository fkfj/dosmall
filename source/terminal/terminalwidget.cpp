#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtGui/QHBoxLayout>

#include "terminalwidget.h"

TerminalWidget::TerminalWidget(const QString &workingDir, QWidget *parent)
	:QWidget(parent)
{
	container = new QX11EmbedContainer(this);
	if (workingDir.isEmpty())
		this->workingDir = QDir::homePath();
	else
		this->workingDir = workingDir;

	process = new QProcess(this);
	process->setWorkingDirectory(this->workingDir);
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished()));

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(container);
}

bool TerminalWidget::close()
{
	if (process->pid())
		process->kill();
	return QWidget::close();
}

bool TerminalWidget::isRunning()
{
	if (process->pid())
		return true;
	else
		return false;
}

void TerminalWidget::processFinished()
{
	emit terminalClosed(this);
}

void TerminalWidget::start()
{
	argument.clear();
	argument << "-fn" << "-misc-fixed-medium-r-normal--18-120-100-100-c-90-iso10646-1" // xterm设置字体
			 << "-bg" << "black" << "-fg" << "white"  // 设置xterm背景色为黑色， 字符颜色为白色
			 << "-title" << "xterm"
			 << "-into" << QString::number(container->winId());
	process->start("xterm", argument);
	setWindowTitle("xterm");
}

void TerminalWidget::execute(const QString &command)
{
	argument.clear();
	argument << "-fn" << "-misc-fixed-medium-r-normal--18-120-100-100-c-90-iso10646-1" // xterm设置字体
			 << "-bg" << "black" << "-fg" << "white"   // 设置xterm背景色为黑色， 字符颜色为白色
			 << "-title" << command
			 << "-into" << QString::number(container->winId())
			 <<  "-e" << command;
	process->start("xterm", argument);
	setWindowTitle(command);
}

QString TerminalWidget::workingDirectory()
{
	return workingDir;
}
