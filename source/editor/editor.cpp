#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#include "editor.h"

// public

Editor::Editor(QWidget *parent)
	: QWidget(parent)
{
	setUi();
	connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(fileClose(int)));
} // Editor

Editor::Editor(const QString &filePath, QWidget *parent)
	: QWidget(parent)
{
	setUi();
	connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(fileClose(int)));
	// 若文件存在，则打开它，否则新建文件
	QFileInfo fileInfo(filePath);
	if (fileInfo.exists())
		fileOpen(filePath);
	else
		fileNew(filePath);
} // Editor

Editor::Editor(const QStringList &filePathList, QWidget *parent)
	: QWidget(parent)
{
	setUi();
	fileOpen(filePathList);
}

// public slots

void Editor::fileNew()
{
    QString filePath =
		QFileDialog::getSaveFileName(this, tr("新建文件"), QDir::homePath(),
									 tr("All Files (*)"));
    if (filePath.isEmpty())
		return;

	fileNew(filePath);
} // fileNew

void Editor::fileNew(const QString &filePath)
{
	// 检查是否有父母录的写权限
	QFileInfo fileInfo(filePath);
	QFileInfo parentDir(fileInfo.absolutePath());
	if (!parentDir.exists()){
		QMessageBox::warning(this, tr("该目录不存在"),
							 tr("%1").arg(parentDir.absoluteFilePath()),
							 QMessageBox::Ok);
		return ;
	}
	if (!parentDir.isWritable()){
		QMessageBox::warning(this, tr("权限不够"),
								   tr("无法在此创建文件: %1/").arg(parentDir.absoluteFilePath()),
								   QMessageBox::Ok );
		return ;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("写入出错"),
							 tr("无法创建该文件: %1/").arg(filePath),
							 QMessageBox::Ok );
		return;
	}
	file.write(QByteArray());
	file.close();
	emit fileCreated(filePath);
	
	CodeEditor *codeEditor = new CodeEditor(filePath);
	QIcon icon(getIconFilePath(filePath));
	tabWidget->addTab(codeEditor, icon, fileInfo.fileName());
	tabWidget->setTabToolTip(tabWidget->indexOf(codeEditor), codeEditor->getFilePath());
	setConnect(codeEditor);
	tabWidget->setCurrentWidget(codeEditor);
	openedFileMap[filePath] = codeEditor;
} // fileNew

void Editor::fileNewWithPath(const QString &path)
{
	if (path.isEmpty())
		fileNew();
	
	QString filePath =
		QFileDialog::getSaveFileName(this, tr("新建文件"), path,
									 tr("All Files (*)"));
    if (filePath.isEmpty())
		return;

	fileNew(filePath);
} // fileNewWithPath

void Editor::fileOpen()
{
	QStringList files =
		QFileDialog::getOpenFileNames(this,
									  tr("请选择要打开文件"),
									  QDir::homePath(), tr("All Files (*)"));
	QStringList::Iterator it = files.begin();
	while (it != files.end()) {
		fileOpen(*it);
		++it;
	}
} // fileOpen

void Editor::fileOpen(const QString &filePath, bool reload)
{
	if (openedFileMap.contains(filePath)) {
		if (reload)
			(openedFileMap.value(filePath))->reload();
		tabWidget->setCurrentWidget(openedFileMap.value(filePath));
		return ;
	}

	QFileInfo fileInfo(filePath);
	if (!fileInfo.exists()){
		QMessageBox::information(this, tr("该文件不存在"),
							 tr("%1").arg(filePath),
							 QMessageBox::Ok);
		return ;
	} else if (!fileInfo.isFile()) {
		QMessageBox::information(this, tr("该文件不是普通文件"),
								 tr("%1").arg(filePath),
								 QMessageBox::Ok);
		return ;
	}
	
    QFile file(filePath);
    if (!(file.open(QFile::ReadOnly | QIODevice::Text))) {
		QMessageBox::information(this, tr("无法读取该文件"),
							 tr("%1").arg(filePath),
							 QMessageBox::Ok);
		return ;
	} else {
		CodeEditor *codeEditor = new CodeEditor(filePath);
		QTextStream in(&file);
		codeEditor->getPlainEditor()->setPlainText(in.readAll());
		file.close();

		QIcon icon(getIconFilePath(filePath));
		tabWidget->addTab(codeEditor, icon, codeEditor->getFileName());
		tabWidget->setTabToolTip(tabWidget->indexOf(codeEditor), codeEditor->getFilePath());
		setConnect(codeEditor);
		tabWidget->setCurrentWidget(codeEditor);
		openedFileMap[filePath] = codeEditor;
		emit fileOpened(filePath);
	}
} // fileOpen

void Editor::fileOpen(const QStringList &filePathList, bool reload)
{
	QStringList::const_iterator it = filePathList.begin();
	while (it != filePathList.end()) {
		fileOpen(*it, reload);
		++it;
	}
} // fileOpen

void Editor::fileOpenWithPath(const QString &path)
{
	if (path.isEmpty())
		fileOpen();
   
	QStringList files =
		QFileDialog::getOpenFileNames(this,
									  tr("请选择要打开的文件"),
									  path, tr("All Files (*)"));
	QStringList::Iterator it = files.begin();
	while (it != files.end()) {
		fileOpen(*it);
		++it;
	}
} // fileOpenWithPath

bool Editor::fileSave()
{
    CodeEditor *currentEditor =
		qobject_cast<CodeEditor*>(tabWidget->currentWidget());
	return fileSave(currentEditor);
} // fileSave

bool Editor::fileSave(int index)
{
	CodeEditor *codeEditor =
		qobject_cast<CodeEditor *>(tabWidget->widget(index));
	return fileSave(codeEditor);
} // fileSave

bool Editor::fileSaveAs()
{
    CodeEditor *currentEditor = qobject_cast<CodeEditor*>(tabWidget->currentWidget());
	if (!currentEditor)
		return false;
	return fileSaveAs(currentEditor);
} // fileSaveAs

bool Editor::fileSaveAll()
{
	CodeEditor *codeEditor;
	QMap<QString, CodeEditor*>::const_iterator begin =
		openedFileMap.constBegin();
	QMap<QString, CodeEditor*>::const_iterator end =
		openedFileMap.constEnd();
	QMap<QString, CodeEditor*>::const_iterator it = begin;

	for (it = begin; it != end; ++it)  {
		codeEditor = it.value();
		if (codeEditor->isModified())
			fileSave(codeEditor);
	}
	return true;
} // fileSaveAll

void Editor::fileClose()
{
	return fileClose(tabWidget->currentIndex());
} // fileClose

void Editor::fileClose(int index)
{
	if (index < 0 && index >= tabWidget->count())
		return ;

	CodeEditor *codeEditor = qobject_cast<CodeEditor*>(tabWidget->widget(index));
	// 如果类型转换不成功, 则标示index页不是codeEditor, 直接将它一触即可

	if (!codeEditor) {
		tabWidget->removeTab(index);
	} else if (maybeSave(codeEditor)) { // 如果转换成功
		openedFileMap.remove(codeEditor->getFilePath());
		tabWidget->removeTab(index);
		delete codeEditor;
	}
} // fileClose

void Editor::format()
{

} // format

void Editor::setMode()
{

} // setMode
// new

void Editor::setReadOnly()
{

} // setReadOnly

void Editor::modificationChanged(bool b, CodeEditor *codeEditor)
{
	int index = tabWidget->indexOf(codeEditor);
	if (b) {
		tabWidget->setTabText(index, "* " + codeEditor->getFileName());
	} else {
		tabWidget->setTabText(index, codeEditor->getFileName());
	}
} // modificationChanged

void Editor::renameTab(const QString &oldFilePath, CodeEditor *codeEditor)
{
	openedFileMap.remove(oldFilePath);
	openedFileMap[codeEditor->getFilePath()] = codeEditor;
	int index = tabWidget->indexOf(codeEditor);
	tabWidget->setTabText(index, codeEditor->getFileName());
	tabWidget->setTabToolTip(index, codeEditor->getFilePath());
}	

void Editor::filePathChanged(const QString &oldFilePath, const QString &newFilePath)
{
	if (!openedFileMap.contains(oldFilePath))
		return;

	openedFileMap[newFilePath] = openedFileMap[oldFilePath];
	openedFileMap.remove(oldFilePath);
	CodeEditor *codeEditor = openedFileMap[newFilePath];
	codeEditor->setFilePath(newFilePath);
	tabWidget->setTabText(tabWidget->indexOf(codeEditor), codeEditor->getFileName());
	tabWidget->setTabToolTip(tabWidget->indexOf(codeEditor), codeEditor->getFilePath());
}

void Editor::pathChanged(const QString &oldPath, const QString &newPath)
{
	QStringList list = openedFileMap.keys();
	QRegExp rx("^" + oldPath + ".*");
	int index = list.indexOf(rx);
	while(index >= 0) {
		QString filePath = list.at(index);
		CodeEditor *codeEditor = openedFileMap[filePath];
		openedFileMap.remove(filePath);
		filePath.replace(QRegExp("^" + oldPath), newPath);
		codeEditor->setFilePath(filePath);
		openedFileMap[filePath] = codeEditor;
		tabWidget->setTabToolTip(tabWidget->indexOf(codeEditor), codeEditor->getFilePath());
		list.removeAt(index);
		index = list.indexOf(rx);
	}
}

void Editor::fileDeleted(const QString &filePath)
{
	if (!openedFileMap.contains(filePath))
		return;

	tabWidget->removeTab(tabWidget->indexOf(openedFileMap[filePath]));
	openedFileMap.remove(filePath);
}

void Editor::directoryDeleted(const QString &dirPath)
{
	QStringList list = openedFileMap.keys();
	QRegExp rx("^" + dirPath + ".*");
	int index = list.indexOf(rx);
	while(index >= 0) {
		QString filePath = list.at(index);
		CodeEditor *codeEditor = openedFileMap[filePath];
		tabWidget->removeTab(tabWidget->indexOf(codeEditor));
		openedFileMap.remove(filePath);
		delete codeEditor;
		list.removeAt(index);
		index = list.indexOf(rx);
	}
}

void Editor::closePathFile(const QString &path)
{
	QStringList list = openedFileMap.keys();
	QRegExp rx("^" + path + ".*");
	int index = list.indexOf(rx);
	while(index >= 0) {
		QString filePath = list.at(index);
		CodeEditor *codeEditor = openedFileMap[filePath];
		fileClose(tabWidget->indexOf(codeEditor));
		list.removeAt(index);
		index = list.indexOf(rx);
	}
}


bool Editor::quit()
{
	return maybeSaveAll();
} // quit

// privaet

bool Editor::fileSave(CodeEditor *codeEditor)
{
	if ((!codeEditor) ||
		(!codeEditor->isModified()))
		return true;
	
	QString filePath = codeEditor->getFilePath();
	QFile file(codeEditor->getFilePath());
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::question(this, tr("无法保存该文件"),
									tr("是否选择另存为该文件: %1",
									   "点击【OK】另存为， 点击【Cancel】放弃该文件的修改").arg(filePath),
									QMessageBox::Ok | QMessageBox::Cancel,
									QMessageBox::Ok);
		
		switch (ret) {
		case QMessageBox::Ok:
			while(!fileSaveAs(codeEditor));
			return true;
		case QMessageBox::Cancel:
			return false;
		default:
			return false;
		}
	}
	QTextStream out(&file);
	out << codeEditor->getPlainEditor()->toPlainText();
	file.close();
	
	codeEditor->getPlainEditor()->document()->setModified(false);
	return true;	
} // fileSave

bool Editor::fileSaveAs(CodeEditor *codeEditor)
{
	if (!codeEditor) {
		return false;
	}

	QString filePath =
		QFileDialog::getSaveFileName(this,
									 tr("另存为..."),
									 codeEditor->getFilePath(),
									 tr("All Files (*)"));

    if (filePath.isEmpty()) {
		return false;
	}

	// 检查是否有父母录的写权限
	QFileInfo fileInfo(filePath);
	QFileInfo parentDir(fileInfo.absolutePath());
	if (!parentDir.exists()){
		QMessageBox::information(this, tr("该文件不存在"),
							 tr("%1").arg(parentDir.absoluteFilePath()),
							 QMessageBox::Ok);
		return false;
	}
	QMessageBox::StandardButton ret;
	if (!parentDir.isWritable()){
		ret = QMessageBox::warning(this, tr("权限不够"),
							 tr("无法在此创建文件: %1").arg(parentDir.absoluteFilePath()),
								   QMessageBox::Ok | QMessageBox::Discard,
								   QMessageBox::Ok);
		if (ret == QMessageBox::Ok)
			return false;
		else
			return true;
	}

    QFile file(filePath);
    QTextStream out(&file);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		ret = QMessageBox::warning(this, tr("写入文件失败"),
							 tr("%1").arg(filePath),
							 QMessageBox::Ok);
		if (ret == QMessageBox::Ok)
			return false;
		else
			return true;
	}
    out << codeEditor->getPlainEditor()->toPlainText(); // 写入文件
	file.close();
	
	QString oldFilePath = codeEditor->getFilePath(); // 暂存修改前的文件名
    codeEditor->setFilePath(filePath); // 更改文件名
    tabWidget->setTabText(tabWidget->indexOf(codeEditor),
						  codeEditor->getFileName());  // 修改tab标题为新的文件名
    codeEditor->getPlainEditor()->document()->setModified(false);
	openedFileMap.remove(oldFilePath); 	// 更新"已打开文件列表"
	openedFileMap[filePath] = codeEditor;

    return true;
} // fileSaveAs

bool Editor::maybeSave(CodeEditor *codeEditor)
{
	if ((!codeEditor) ||
		(!codeEditor->isModified()))
		return true;
	
	QMessageBox::StandardButton ret;
	ret = QMessageBox::question(this, tr("文件已修改"),
							   tr("是否保存该文件的修改?"),
								QMessageBox::Save | QMessageBox::Discard
								| QMessageBox::Cancel,
								QMessageBox::Save);
	
	if (ret == QMessageBox::Save)
		return fileSave(codeEditor);
	else if (ret == QMessageBox::Cancel)
		return false;
	else
		return true;
} // maybeSave

bool Editor::maybeSaveAll()
{
	CodeEditor *codeEditor;
	QMap<QString, CodeEditor*>::const_iterator begin =
		openedFileMap.constBegin();
	QMap<QString, CodeEditor*>::const_iterator end =
		openedFileMap.constEnd();
	QMap<QString, CodeEditor*>::const_iterator it = begin;

	for (; it != end; ++it) {
		codeEditor = it.value();
		if (codeEditor->isModified()) {
			QMessageBox::StandardButton ret;
			ret = QMessageBox::question(this, tr("修改的文件没有保存"),
										tr("是否保存所有文件的修改?"),
										QMessageBox::SaveAll | QMessageBox::Discard
										| QMessageBox::Cancel,
										QMessageBox::SaveAll);
			switch (ret) {
			case QMessageBox::SaveAll:
				return fileSaveAll();
			case QMessageBox::Cancel:
				return false;
			default:
				break;
			}
			break;
		}
	}
	return true;
} // maybeSaveAll

void Editor::setUi()
{
	hbox = new QHBoxLayout(this);
	hbox->setObjectName(QString::fromUtf8("hbox"));
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->setSpacing(1);
	
	tabWidget = new QTabWidget(this);
	tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
	tabWidget->setTabsClosable(true);
	tabWidget->setMovable(true);
	tabWidget->setIconSize(QSize(24, 24));
		
	hbox->addWidget(tabWidget);
	setLayout(hbox);
} // setUi

void Editor::setConnect(CodeEditor *codeEditor)
{
	connect(codeEditor, SIGNAL(modificationChangedSignal(bool, CodeEditor *)),
			this, SLOT(modificationChanged(bool, CodeEditor *)));

	connect(codeEditor, SIGNAL(fileNewWithPathSignal(const QString &)), this, SLOT(fileNewWithPath(const QString &)));
 	connect(codeEditor, SIGNAL(fileOpenWithPathSignal(const QString &)), this, SLOT(fileOpenWithPath(const QString&)));
	connect(codeEditor, SIGNAL(fileSaveAllSignal()), this, SLOT(fileSaveAll()));
	connect(codeEditor, SIGNAL(renamedSignal(const QString &, CodeEditor *)), this, SLOT(renameTab(const QString&, CodeEditor *)));
} // setConnect

QString Editor::getIconFilePath(const QString &filePath)
{
	QFileInfo fileInfo(filePath);
	QString suffix = fileInfo.suffix();
	QString fileName = fileInfo.fileName();
	
	if (suffix.isEmpty()) {
		if (fileName == "Readme" || fileName == "README" ||
			fileName == "readme")
			return QString(":/images/filetype/text-x-readme.png");
		else if (fileName == "Makefile")
			return QString(":/images/filetype/text-x-makefile.png");
		else
			return QString(":/images/filetype/text-x-generic.png");
	}
	
	if (suffix == "c")
		return QString(":/images/filetype/text-x-c.png");
	else if (suffix == "cpp" || suffix == "cc")
		return QString(":/images/filetype/text-x-c++.png");
	else if (suffix == "h")
		return QString(":/images/filetype/text-x-chdr.png");
	else if (suffix == "sh")
		return QString(":/images/filetype/text-x-script");
	else
		return QString(":/images/filetype/text-x-generic.png");
}
