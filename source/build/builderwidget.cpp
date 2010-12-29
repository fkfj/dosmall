#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMessageBox>

#include <QtCore/QDebug>
#include "builderwidget.h"
#include "processaction.h"

BuilderWidget::BuilderWidget(const QString &projectName, CProject *cproject, QWidget *parent)
	: QWidget(parent)
{
	name = projectName;
	cprojectObject = cproject;
	toolBar = new QToolBar(this);
	textEdit = new PlainEditor(this);
	cbuilder = 0;
	valid = false;
	
	if (cprojectObject) {
		cbuilder = new CBuilder(cprojectObject);
		action_Clear = new QAction("清空输出", toolBar);
		QIcon icon_Clear;
		icon_Clear.addFile(QString::fromUtf8(":/images/clear.png"),
						   QSize(), QIcon::Normal, QIcon::On);
		action_Clear->setIcon(icon_Clear);
		toolBar->addAction(action_Clear);
		connect(action_Clear, SIGNAL(triggered()), textEdit, SLOT(clear()));
		connect(cbuilder, SIGNAL(buildError(const QString &)), this, SLOT(appendText(const QString &)));
		connect(cbuilder, SIGNAL(buildFinished(const QString &)), this, SLOT(appendText(const QString &)));
		connect(cbuilder, SIGNAL(buildBreaked(const QString &)), this, SLOT(appendText(const QString &)));
		connect(cbuilder, SIGNAL(promptText(const QString &)), this, SLOT(appendText(const QString &)));
		setWindowTitle(cprojectObject->name() + " Builder");

		valid = true;
	} else {
		valid = false;
		QMessageBox::warning(this, QString("项目不可用"), QString(),
							 QMessageBox::Ok);
		setWindowTitle("无效Builder");
	}

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(toolBar);
	vbox->addWidget(textEdit);
}

void BuilderWidget::addProcessAction(ProcessAction *processAction)
{
	toolBar->addAction(processAction);
}

void BuilderWidget::appendText(const QString &text)
{
	textEdit->appendPlainText(text);
}

void BuilderWidget::breakBuild()
{
	cbuilder->breakBuild();
}

void BuilderWidget::build(const QString &target)
{
	if (!valid) {
		QMessageBox::warning(this, QString("项目不可用"), QString(),
							 QMessageBox::Ok);
		emit deleteThis(this);
		return;
	}

	if (!cprojectObject->isValid()) {
		QMessageBox::warning(this, QString("该项目不可用"),
							  QString(cprojectObject->errorInfo()),
							 QMessageBox::Ok);
		return;
	}
	
	if (cbuilder->isRunning()) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::question(this, QString("有线程正在Build"),
									QString("要中断当前Builder程序并重新Build吗?"),
									QMessageBox::Ok | QMessageBox::Cancel,
									QMessageBox::Ok);
		switch (ret) {
		case QMessageBox::Ok:
			cbuilder->quit();
			break;
		default:
			return;
		}
	}
	cbuilder->build(target);
}

void BuilderWidget::buildAll()
{
	build("all");
}

void BuilderWidget::buildClean()
{
	cbuilder->build("clean");
}

QString BuilderWidget::checkExecFile(const QString &executeFile)
{
	QString file = cprojectObject->rootPath() + "/" + cprojectObject->developMode() + "/" + executeFile;
	QFileInfo fileInfo(file);
	if (fileInfo.exists()) {
		if (fileInfo.isFile() && fileInfo.isExecutable()) {
			return QString();
		} else {
			return QString("文件" + executeFile + "不是可执行文件, 或者您没有它的执行权限");
		}
	} else {
		return QString("文件" + executeFile + "不存在, 您需要先 build 它");
	}
}

void BuilderWidget::cleanProject()
{
	buildClean();
}

void BuilderWidget::clearText()
{
	textEdit->clear();
}

bool BuilderWidget::close()
{
	if (!valid) {
		emit deleteThis(this);
		return true;
	}
	// 检查是否正在 build, 如果是, 则询问是否退出, 如果选择否, 则返回false, 否则检查是否有有程序正在执行
	if (cbuilder->isBuilding()) {
		int ret = QMessageBox::question(this, QString("正在build本项目中..."),
										QString("真的要关闭吗?"),
										QMessageBox::Ok | QMessageBox::Cancel,
										QMessageBox::Cancel);
		if (ret == QMessageBox::Ok) {
			cbuilder->breakBuild();
		} else {
			return false;
		}
	}
	// 检查是否有程序正在执行, 如果是, 则询问是否退出正在执行的程序, 如果是, 则返回true
	if (hasRunningProcess()) {
		int ret = QMessageBox::question(this, QString("有程序正在执行"),
										QString("真的要关闭吗? 关闭将退出这些程序."),
										QMessageBox::Ok | QMessageBox::Cancel,
										QMessageBox::Cancel);
		if (ret == QMessageBox::Ok) {
			exitAllRunProcess();
		} else {
			return false;
		}
	}
	emit deleteThis(this);
	return true;
}

void BuilderWidget::exitAllRunProcess()
{
 	QList<ProcessAction *>::const_iterator constIterator = processActionList.constBegin();
 	QList<ProcessAction *>::const_iterator endiIterator= processActionList.constEnd();
	while (constIterator != endiIterator) {
		(*constIterator)->exitProcess();
		++constIterator;
	}
}

bool BuilderWidget::hasRunningProcess()
{
	return !processActionList.isEmpty();
}

bool BuilderWidget::isBuilding()
{
	if (!valid)
		return false;
	return cbuilder->isBuilding();
}

QString BuilderWidget::projectName()
{
	return name;
}

void BuilderWidget::remvoeProcessAction(ProcessAction *processAction)
{
	toolBar->removeAction(processAction);
	processActionList.removeAll(processAction);
}

void BuilderWidget::run(const QString &executeFile, const QString &args)
{
	if (valid) {
		if (!cprojectObject->isValid()) {
			QMessageBox::warning(this, QString("该项目不可用"),
								 QString(cprojectObject->errorInfo()),
								 QMessageBox::Ok);
			return;
		}

		QString checkInfo = checkExecFile(executeFile);
		if (checkInfo.isEmpty()) {
			ProcessAction *processAction =
				new ProcessAction(cprojectObject->rootPath() + "/" + cprojectObject->developMode(),
								  executeFile, args, cprojectObject->ldLibraryPath(), this);
			processAction->setText(executeFile);
			processAction->setToolTip(executeFile + " " + args + "\n点击退出该程序");
			QIcon icon_Break;
			icon_Break.addFile(QString::fromUtf8(":/images/break.png"),
							  QSize(), QIcon::Normal, QIcon::On);
			processAction->setIcon(icon_Break);
			processActionList << processAction;
			connect(processAction, SIGNAL(started(ProcessAction*)), this, SLOT(addProcessAction(ProcessAction *)));
			connect(processAction, SIGNAL(finished(ProcessAction*)), this, SLOT(remvoeProcessAction(ProcessAction *)));
			processAction->startProcess();
		} else {
			QMessageBox::information(this, QString("提示"), checkInfo, QMessageBox::Ok);
		}
	} else {
		QMessageBox::warning(this, QString("无效Builder"), QString(), QMessageBox::Ok);
		emit deleteThis(this);
	}
}

