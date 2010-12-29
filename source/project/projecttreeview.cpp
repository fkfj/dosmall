#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtGui/QIcon>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QVBoxLayout>

#include "projecttreeview.h"
#include "projectdialog.h"
#include "cprojectconfigdialog.h"
#include "../global.h"

// public:
ProjectTreeView::ProjectTreeView(QWidget *parent)
	:QTreeView(parent)
{
	setUi();

	mimeTypeList << "A" << "B" << "C" << "D" << "E" << "F" << "G"
				 << "H" << "I" << "J" << "K" << "L" << "M";
	projectManager = Global::projectManager();
	connect(projectManager, SIGNAL(projectOpened(const QString &)), this, SLOT(loadProject(const QString &)));
	connect(projectManager, SIGNAL(projectClosed(const QString &)), this, SLOT(closeProjectItem(const QString &)));
	connect(projectManager, SIGNAL(projectDeleted(const QString &)), this, SLOT(deleteProjectItem(const QString &)));
	connect(projectManager, SIGNAL(projectRenamed(const QString &, const QString &)),
			this, SLOT(renameProjectItem(const QString &, const QString &)));
	loadOpenedProject();
	loadClosedProject();
}

void ProjectTreeView::build(const QString &target)
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	QString projectName = projectNameText(currentIdx);
	emit build(target, projectName);
}

// 创建一个项目projectName的文件树, 并返回该树的顶行
QList<QStandardItem *> ProjectTreeView::buildClosedProjectItem(const QString &projectName)
{
	QList<QStandardItem *> itemList;
	QIcon icon(":/images/closed_sign.png");
	QStandardItem *headItem = new QStandardItem(icon, projectName);
 	headItem->setToolTip("已关闭项目\n" + projectManager->projectPath(projectName));
	headItem->setEditable(false);
	QStandardItem *pathItem = new QStandardItem(QString());
	pathItem->setEditable(false);
	QStandardItem *typeItem = new QStandardItem(mimeTypeList.at(ProjectTreeView::ClosedProject));
	typeItem->setEditable(false);
	QStandardItem *projectItem = new QStandardItem(projectName);
	projectItem->setEditable(false);
	itemList << headItem << pathItem << typeItem << projectItem;
	return itemList;
}

// 创建一个path目录的文件树, 并返回该树的顶行
QList<QStandardItem *> ProjectTreeView::buildDirectoryItem(const QString &path, const QString &projectName)
{
	QList<QStandardItem *> itemList;
	QDir dir(path);
	if (!dir.exists())
		return itemList;

	// 创建path目录的itemList
	QIcon icon(":/images/filetype/folder.png");
	QStandardItem *headItem = new QStandardItem(icon, dir.dirName());
 	headItem->setToolTip(dir.path());
	headItem->setEditable(false);
	QStandardItem *pathItem = new QStandardItem(dir.path());
	pathItem->setEditable(false);
	QStandardItem *typeItem = new QStandardItem(mimeTypeList.at(ProjectTreeView::Directory));
	typeItem->setEditable(false);
	QStandardItem *projectItem = new QStandardItem(projectName);
	projectItem->setEditable(false);

	itemList << headItem << pathItem << typeItem << projectItem;

	// 扫描path目录子文件, 将它们加入headItem的自节点中
	dir.setFilter(QDir::Dirs | QDir::Files);
	dir.setSorting(QDir::DirsFirst);
	QFileInfoList fileInfoList = dir.entryInfoList();

	QList<QFileInfo>::const_iterator constIterator = fileInfoList.constBegin();
	QList<QFileInfo>::const_iterator endIterator = fileInfoList.constEnd();
	while (constIterator != endIterator) {
		QFileInfo fileInfo = *constIterator;
		if (fileInfo.fileName() == "." || fileInfo.fileName() == "..") {
			++constIterator;
			continue ;
		} else if (fileInfo.isDir()){
			QList<QStandardItem *> list = buildDirectoryItem(fileInfo.filePath(), projectName);
			if (!list.isEmpty())
				headItem->appendRow(list);
		} else {
			QList<QStandardItem *> list = buildFileItem(fileInfo.filePath(), projectName);
			if (!list.isEmpty())
				headItem->appendRow(list);
		}
		++constIterator;
	}
	sortChildrenItem(headItem);
	
	return itemList;
}

// 创建一个filePath文件的节点行, 并返回该行
QList<QStandardItem *> ProjectTreeView::buildFileItem(const QString &filePath, const QString &projectName)
{
	QList<QStandardItem *> itemList;
	QFileInfo fileInfo(filePath);
	if (!fileInfo.exists() || !fileInfo.isFile())
		return itemList;

	QString mimeType;
	QIcon icon = mimeTypeIcon(fileInfo, mimeType);
	QStandardItem *headItem = new QStandardItem(icon, fileInfo.fileName());
	headItem->setToolTip(fileInfo.absoluteFilePath());
	headItem->setEditable(false);
	QStandardItem *pathItem = new QStandardItem(fileInfo.absoluteFilePath());
	pathItem->setEditable(false);
	QStandardItem *typeItem = new QStandardItem(mimeType);
	typeItem->setEditable(false);
	QStandardItem *projectItem = new QStandardItem(projectName);
	projectItem->setEditable(false);

	itemList << headItem << pathItem << typeItem << projectItem;
	
	return itemList;
}

QList<QStandardItem *> ProjectTreeView::buildOpenedProjectItem(const QString projectName)
{
	QList<QStandardItem *> itemList =
		buildDirectoryItem(projectManager->projectPath(projectName), projectName);
	if (itemList.isEmpty())
		return itemList;
	itemList.at(0)->setText(projectName); // 修改名称为项目名称
	// QIcon icon(":/images/filetype/c_cpp_project.png");
	QIcon icon;
	icon.addFile(QString::fromUtf8(":/images/filetype/c_cpp_project.png"), QSize(48, 48), QIcon::Normal, QIcon::On);
	itemList.at(0)->setIcon(icon);
	itemList.at(2)->setText(mimeTypeList.at(ProjectTreeView::OpenedProject)); // 修改类型为项目类型
	return itemList;
}

// 递归的将index的子节点的文件的路径中的oldPath改为newPath
void ProjectTreeView::changeChildrenPath(const QStandardItem *parent,
										 const QString &oldPath,
										 const QString &newPath)
{
	int rowCount = parent->rowCount();
	if (rowCount < 0 )
		return;
	QStandardItem *item0 = 0;
	QStandardItem *item1 = 0;
	QString path = QString();
	for (int row = 0; row < rowCount; ++row) {
		item1 = parent->child(row, 1);
		path = item1->text();
		path.replace(QRegExp("^" + oldPath), newPath);
		item1->setText(path);
		item0 = parent->child(row, 0);
		item0->setToolTip(path);
		changeChildrenPath(item0, oldPath, newPath);
	}
}

void ProjectTreeView::changeChildrenProjectName(const QStandardItem *parent,
												const QString &newName)
{
	int rowCount = parent->rowCount();
	if (rowCount < 0 )
		return;
	QStandardItem *item = 0;
	for (int row = 0; row < rowCount; ++row) {
		item = parent->child(row, 3);
		item->setText(newName);
		changeChildrenProjectName(item, newName);
	}
}

void ProjectTreeView::closeProject()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	int row = currentIdx.row();
	QString projectName = model->item(row, 3)->text();
	projectManager->closeProject(projectName);
}

void ProjectTreeView::closeProjectItem(const QString &projectName)
{
	if (openedProjectItemMap.contains(projectName)) {
		QStandardItem *pathItemVar = pathItem(openedProjectItemMap[projectName]->index());
		topFilePathItemMap.remove(pathItemVar->text());
		model->removeRows(openedProjectItemMap[projectName]->index().row(), 1);
		openedProjectItemMap.remove(projectName);
	}
	QList<QStandardItem *> list = buildClosedProjectItem(projectName);
	int count = openedProjectItemMap.count() + closedProjectItemMap.count();
	model->insertRow(count, list);
	closedProjectItemMap[projectName] = list.at(0);
}

void ProjectTreeView::configProject()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	QString projectName = projectNameText(currentIdx);
	emit configProject(projectName);
}

void ProjectTreeView::debug()
{
	QMessageBox::information(this, "暂时未实现该功能", "   期待您的参与!   ", QMessageBox::Ok);
	// 暂时不实现这个功能
}

void ProjectTreeView::deleteDirectory()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;
	QString type = typeText(currentIdx);
	QString path = pathText(currentIdx);
	QString name = currentIdx.data().toString();
	// 如果当前节点类型是不是 Project 或者 目录
	if (type == mimeTypeList.at(ProjectTreeView::Directory)) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, tr("删除目录"),
								   tr("确实要删除目录\"%1\"即其目录下的子文件?\n此操作不可恢复!")
								   .arg(name),
								   QMessageBox::Ok | QMessageBox::Cancel,
								   QMessageBox::Cancel);

		if (ret == QMessageBox::Ok) {
			QStandardItem *item = model->itemFromIndex(currentIdx);
			QStandardItem *parent = item->parent();
			parent->removeRow(item->row()); // 从父节点删除该目录节点
			QProcess::execute("rm",  QStringList() << "-rf" << path); // 删除该目录
			emit directoryDeleted(path, projectNameText(currentIdx)); // 发送目录已删除信号
		}
	}
}

void ProjectTreeView::deleteFile()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;
	QString type = typeText(currentIdx);
	QString path = pathText(currentIdx);
	QString name = currentIdx.data().toString();
	// 如果当前节点类型是不是 Project 或者 目录
	if (type != mimeTypeList.at(ProjectTreeView::OpenedProject) &&
		type != mimeTypeList.at(ProjectTreeView::Directory)) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, tr("删除文件"),
								   tr("确实要删除文件\"%1\"?\n此操作不可恢复!").arg(name),
								   QMessageBox::Ok | QMessageBox::Cancel,
								   QMessageBox::Cancel);
		if (ret == QMessageBox::Ok) {
			QStandardItem *item = model->itemFromIndex(currentIdx);
			QStandardItem *parent = item->parent();
			parent->removeRow(item->row()); // 从父节点删除该节点
			QProcess::execute("rm",  QStringList() << "-f" << path); // 删除文件
			emit fileDeleted(path, projectNameText(currentIdx)); // 发送已删除文件信号
		}
	}
}

void ProjectTreeView::deleteProjectItem(const QString &projectName)
{
	if (openedProjectItemMap.contains(projectName)) {
		QStandardItem *headItem = openedProjectItemMap[projectName];
		QStandardItem *pathItemVar = pathItem(headItem->index());
		topFilePathItemMap.remove(pathItemVar->text());
		openedProjectItemMap.remove(projectName);
		model->removeRows(headItem->index().row(), 1);
	} else if (closedProjectItemMap.contains(projectName)) {
		QStandardItem *headItem = closedProjectItemMap[projectName];
		model->removeRows(headItem->index().row(), 1);
		closedProjectItemMap.remove(projectName);
	}
}

void ProjectTreeView::deleteProject()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	int row = currentIdx.row();
	QString projectName = model->item(row, 0)->text();
	QString projectPath = model->item(row, 1)->text();
	// dialog确认是否删除该项目以及项目目录
	DeleteDialog *deleteDialog =
		new DeleteDialog("删除项目", "真的要删除项目\"" + projectName + "\"吗?",
						 "同时删除\"" + projectPath + "\"目录下的所有文件?\n此操作不可恢复.");
	if (deleteDialog->exec() == QDialog::Accepted) {
		bool deleteFile = false;
		// 如果checkBox 选中, 即要删除该项目目录
		if (deleteDialog->checkState() == 2) 
			deleteFile = true;
		projectManager->deleteProject(projectName, deleteFile);
	}
	delete deleteDialog;
}

// private slots:
void ProjectTreeView::doubleClicked(const QModelIndex &index)
{
	QString type = typeText(index);
	QString projectName = projectNameText(index);
	if (type == mimeTypeList.at(ProjectTreeView::Source) ||
		type == mimeTypeList.at(ProjectTreeView::Header) ||
		type == mimeTypeList.at(ProjectTreeView::Makefile) ||
		type == mimeTypeList.at(ProjectTreeView::Shell) ||
		type == mimeTypeList.at(ProjectTreeView::Text)) {
		emit openFile(pathText(index), ProjectTreeView::Text, projectName);
	}
} 

void ProjectTreeView::findCode()
{
	QMessageBox::information(this, "暂时未实现该功能", "    期待您的参与!    ", QMessageBox::Ok);
	// 暂时不实现这个功能
}

void ProjectTreeView::findFile()
{
	QMessageBox::information(this, "暂时未实现该功能", "    期待您的参与!   ", QMessageBox::Ok);
	// 暂时不实现这个功能
}

void ProjectTreeView::loadClosedProject()
{
	QStringList closedProject = projectManager->closedProjectNames();
	QStringList::const_iterator constIterator = closedProject.constBegin();
	QStringList::const_iterator endIterator = closedProject.constEnd();
	int count = openedProjectItemMap.count();
	while (constIterator != endIterator) {
		QList<QStandardItem *> list = buildClosedProjectItem(*constIterator);
		model->insertRow(count, list);
		closedProjectItemMap[*constIterator] = list.at(0);
		++count;
		++constIterator;
	}
}

// 用于右击选择了"导入文件夹"选项, 初选一个对话框选择要导入的文件, 之后将文件夹导入到相应的位置
void ProjectTreeView::loadDirectory()
{
	QString dirPath =
		QFileDialog::getExistingDirectory(this, tr("导入目录"),
										  QDir::homePath());
	if (!dirPath.isEmpty()) {
		loadFileOrDirectory(dirPath);
	}
}

void ProjectTreeView::loadDirectory(const QString &path)
{
	loadFileOrDirectory(path);
}

void ProjectTreeView::loadFile()
{
	QStringList fileList = QFileDialog::getOpenFileNames(this, tr("导入文件到项目浏览器"),
														 QDir::homePath());
	if (fileList.isEmpty())
		return;
	QStringList::const_iterator constIterator = fileList.constBegin();
	QStringList::const_iterator endIterator  = fileList.constEnd();
	while (constIterator != endIterator) {
		loadFileOrDirectory(*constIterator);
		++constIterator;
	}
}

void ProjectTreeView::loadFile(const QString &filePath)
{
	loadFileOrDirectory(filePath);
}

// 用于将 filePath 放到 treeView 中对应的位置, 并设置该节点为当前节点
void ProjectTreeView::loadFileOrDirectory(const QString &filePath)
{
	QFileInfo fileInfo(filePath);
	if (!fileInfo.exists())
		return;

	QString tmpFilePath = fileInfo.absoluteFilePath();
	QMap<QString, QStandardItem*>::const_iterator constIterator = topFilePathItemMap.constBegin();
	QMap<QString, QStandardItem*>::const_iterator endIterator = topFilePathItemMap.constEnd();
	// 从topFilePathItemMap中查找是否有该文件路径或者该文件父目录的节点
	// 如果找到该文件的节点, 则设置该节点为当前节点
	// 如果没有找到该文件的节点, 但是找到它的父目录的节点, 则在父目录节点添加该文件节点
	while (constIterator != endIterator) {
		// 如果topFilePathItemMap包含该路径
		if (tmpFilePath.contains(constIterator.key())) {
			// 如果tmpFilePath与顶节点路径相等, 则设置该顶节点为当前节点并返回
			if (tmpFilePath == (constIterator.key())) {
				setCurrentIndex(constIterator.value()->index());
				return;
			} else {	// 如果是包含关系而不是相等关系, 则在该顶节点下查找是否有该文件节点
				QStandardItem *headItemVar = constIterator.value();
				QString projectNameVar = projectNameText(headItemVar->index());
				QString str = tmpFilePath;

				str.remove(QRegExp("^" + constIterator.key())); 
				QStringList list = str.split("/", QString::SkipEmptyParts);
				int row = headItemVar->rowCount() - 1;
				int index = 0;
				while (row >= 0 && index < list.count()) {
					// 如果子节点的text与list.at(index)相等, 则该节点是文件filePath的节点或者是该文件的父目录的节点
					if (headItemVar->child(row, 0)->text() == list.at(index)) {
						// 如果index = list.count()-1,则说明该节点就是文件filePath的节点
						if (index == list.count()-1) { 
							setCurrentIndex(headItemVar->child(row, 0)->index());
							return;
						} else { // 如果不是, 则从该子节点查找
							headItemVar = headItemVar->child(row, 0);
							row = headItemVar->rowCount() - 1;
							++index;
							continue;
						}
					}
					--row;
				}

				// 如果row < 0, 则说明没有找到该文件的节点, 但是找到它的父目录的节点, 在父目录节点下添加该文件节点
				if (row < 0) {
					QList<QStandardItem*> itemList;
					if (fileInfo.isDir())
						itemList = buildDirectoryItem(tmpFilePath, projectNameVar);
					else
						itemList = buildFileItem(tmpFilePath, projectNameVar);
					headItemVar->appendRow(itemList);
					sortChildrenItem(headItemVar);
					setCurrentIndex(itemList.at(0)->index());
					return;
				}
			}
		}
		++constIterator;
	}

	// 如果在topFilePathMap中没有找到该文件及其父目录的节点, 则在最后添加该文件的节点
	QList<QStandardItem*> itemList;
	if (fileInfo.isDir())
		itemList = buildDirectoryItem(tmpFilePath, QString());
	else
		itemList = buildFileItem(tmpFilePath, QString());
	model->appendRow(itemList);
	topFilePathItemMap[tmpFilePath] = itemList.at(0);
	setCurrentIndex(itemList.at(0)->index());
}

// 根据settings文件导入历史项目
void ProjectTreeView::loadOpenedProject()
{
	QStringList openedProject = projectManager->openedProjectNames();
	QStringList::const_iterator constIterator = openedProject.constBegin();
	QStringList::const_iterator endIterator = openedProject.constEnd();
	while (constIterator != endIterator) {
		loadProject(*constIterator);
		++constIterator;
	}
}

void ProjectTreeView::loadProject(const QString &projectName)
{
	// 如果已经导入了该项目, 则直接返回
	if (openedProjectItemMap.contains(projectName))
		return;
	
	// 如果该项目在closedProject中, 则从closedProjectMap中去除它
	if (closedProjectItemMap.contains(projectName)) {
		QModelIndex index = closedProjectItemMap[projectName]->index();
		closedProjectItemMap.remove(projectName);
		int row = index.row();
		model->removeRows(row, 1);
	}
	// 建立该项目的节点, 且把它插入到最后一个已打开项目的节点的后面
	QList<QStandardItem *> list = buildOpenedProjectItem(projectName);
	model->insertRow(openedProjectItemMap.count(), list);
	topFilePathItemMap[list.at(1)->text()] = list.at(0);
	openedProjectItemMap[projectName] = list.at(0);
}

// 检查文件fileInfo的文件类型, 根据类型创建并返回相应的Icon, 并设置mimeType
QIcon ProjectTreeView::mimeTypeIcon(const QFileInfo &fileInfo,
									QString &mimeType)
{
	QString suffix = fileInfo.suffix();
	QString filePath = fileInfo.absoluteFilePath();
	QProcess fileProcess;

	fileProcess.start("file", QStringList() << "--mime-type" << filePath);
	fileProcess.waitForFinished();
	QString ret = fileProcess.readAllStandardOutput();
	ret.remove(QRegExp("\n"));
	ret.remove(QRegExp("^.*:\\s*"));
	QString str1 = ret.split("/").at(0);
	QString str2 = ret.split("/").at(1);

	QIcon icon;
	if (str1 == "text") {
		if (str2 == "x-c") {
			if (suffix == "h") {
				mimeType = mimeTypeList.at(ProjectTreeView::Header);
				icon = QIcon(":/images/filetype/text-x-chdr.png");
			} else if (suffix == "c") {
				icon = QIcon(":/images/filetype/text-x-c.png");
				mimeType = mimeTypeList.at(ProjectTreeView::Source);
			} else if (suffix == "cpp" ||
					   suffix == "cc" ||
					   suffix == "cxx") {
				icon = QIcon(":/images/filetype/text-x-c++.png");
				mimeType = mimeTypeList.at(ProjectTreeView::Source);
			}
		} else if (str2 == "x-java") {
			icon = QIcon(":/images/filetype/text-x-java.png");
			mimeType = mimeTypeList.at(ProjectTreeView::Source);
		} else if (str2 == "x-makefile") {
			icon = QIcon(":/images/filetype/text-x-makefile.png");
			mimeType = mimeTypeList.at(ProjectTreeView::Makefile);
		} else if (str2 == "x-shellscript") {
			icon = QIcon(":/images/filetype/text-x-script.png");
			mimeType = mimeTypeList.at(ProjectTreeView::Shell);
		} else {
			icon = QIcon(":/images/filetype/text-x-generic.png");
			mimeType = mimeTypeList.at(ProjectTreeView::Text);
		}
	} else if (str1 == "application") {
		if (str2 == "x-executable") {
			icon = QIcon(":/images/filetype/application-x-executable.png");
			mimeType = mimeTypeList.at(ProjectTreeView::Executable);
		} else if (str2 == "x-object"){
			icon = QIcon(":/images/filetype/application-x-object.png");
			mimeType = mimeTypeList.at(ProjectTreeView::Object);
		} else if (str2 == "x-sharedlib"){
			icon = QIcon(":/images/filetype/application-x-sharedlib.png");
			mimeType = mimeTypeList.at(ProjectTreeView::Sharedlib);
		} else if (str2 == "x-archive") {
			icon = QIcon(":/images/filetype/package.png");
			mimeType = mimeTypeList.at(ProjectTreeView::Archive);
		} else if (str2 == "x-empty" || str2 == "octet-stream") {
			mimeType = mimeTypeList.at(ProjectTreeView::Text);
			icon = QIcon(":/images/filetype/text-x-generic.png");
		} else {
			icon = QIcon(":/images/filetype/unknown.png");
			mimeType = mimeTypeList.at(ProjectTreeView::Unknown);
		}
	} else {
		icon = QIcon(":/images/filetype/unknown.png");
		mimeType = mimeTypeList.at(ProjectTreeView::Unknown);
	}
	
	return icon;
}

// 处理右击事件
void ProjectTreeView::mousePressEvent (QMouseEvent *event)
{
	if(event->button() == Qt::RightButton) {
		int xPos = event->x();
		int yPos = event->y();
		// 返回点击处所在的记录
		QModelIndex idx = indexAt(QPoint(xPos, yPos));
		if (!idx.isValid()) {
			return;
		}
		setCurrentIndex(idx);

		mouseMenu->clear();
		buildMenu->clear();
		runMenu->clear();
		
		QModelIndex parent = idx.parent();
		QList<QAction *>::const_iterator constIterator;
		QList<QAction *>::const_iterator endIterator;
		if (parent.isValid()) {
			QString type = parent.child(idx.row(), 2).data().toString();
			if (type == mimeTypeList.at(ProjectTreeView::Directory)) {
				mouseMenu->addActions(directoryActionList);
				mouseMenu->exec(event->globalPos());
			} else if (type == mimeTypeList.at(ProjectTreeView::Text)) {
				mouseMenu->addActions(textFileActionList);
				mouseMenu->exec(event->globalPos());
			} else {
				mouseMenu->addActions(otherFileActionList);
				mouseMenu->exec(event->globalPos());
			}
		} else {
			QString type = model->item(idx.row(), 2)->text();
			if (type == mimeTypeList.at(ProjectTreeView::OpenedProject)) {
				QString projectName = projectNameText(idx);
				if (!projectManager->hasProject(projectName))
					return;
				
				CProject *project = projectManager->projectObject(projectName);

				buildMenu->addActions(project->targets());
				runMenu->addActions(project->executeFiles());
				mouseMenu->addActions(openedProjectActionList);
				mouseMenu->insertMenu(action_Debug, runMenu);
				mouseMenu->insertMenu(action_Debug, buildMenu);
				mouseMenu->exec(event->globalPos());
			} else if (type == mimeTypeList.at(ProjectTreeView::ClosedProject)) {
				mouseMenu->addActions(closedProjectActionList);
				mouseMenu->exec(event->globalPos());
			} else if (type == mimeTypeList.at(ProjectTreeView::Directory)) {
				mouseMenu->addActions(directoryActionList);
				mouseMenu->exec(event->globalPos());
			} else {
				mouseMenu->addActions(otherFileActionList);
				mouseMenu->exec(event->globalPos());
			}
		}
	}
	QTreeView::mousePressEvent(event);
}

QStandardItem* ProjectTreeView::headItem(const QModelIndex &index)
{
	if (!index.isValid())
		return 0;
	QModelIndex parent = index.parent();
	if (parent.isValid()) {
		QModelIndex pathIndex = parent.child(index.row(), 0);
		return model->itemFromIndex(pathIndex);
	} else {
		return model->item(index.row(), 0);
	}
}

void ProjectTreeView::newDirectory()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;
	
	QString type = typeText(currentIdx);
	QString projectName = projectNameText(currentIdx);
	QString parentPath = "";	    // 新目录的父目录
	QStandardItem *parentItem = 0;	// 新目录挂的父节点
	// 如果当前节点类型是Project 或者 目录, 则path 为当前节点的path
	if (type == mimeTypeList.at(ProjectTreeView::OpenedProject) ||
		type == mimeTypeList.at(ProjectTreeView::Directory)) {
		parentPath = pathText(currentIdx);
		parentItem = model->itemFromIndex(currentIdx);
	} else { // 否则path为当前节点父节点的path
		parentPath = parentPathText(currentIdx);
		parentItem = model->itemFromIndex(currentIdx.parent());
	}

	if (parentPath.isEmpty() || parentItem == 0)
		return;
	
	QDir parentDir(parentPath);
	RenameDialog *renameDialog = new RenameDialog("新建文件夹", parentDir.entryList(), "", this);
	if (renameDialog->exec() == QDialog::Accepted) {
		QString newDir = renameDialog->newName();
		if (parentDir.mkdir(newDir)) {
			QList<QStandardItem *> list = buildDirectoryItem(parentDir.path() + "/" + newDir, projectName);
			parentItem->appendRow(list);
			sortChildrenItem(parentItem);
			setCurrentIndex(list.at(0)->index());
		}
	}
}	

void ProjectTreeView::newHeaderFile()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	QString type = typeText(currentIdx);
	QString parentPath = QString();
	QString projectName = projectNameText(currentIdx);
	// 如果当前节点类型是Project 或者 目录, 则path 为当前节点的path
	if (type == mimeTypeList.at(ProjectTreeView::OpenedProject) ||
		type == mimeTypeList.at(ProjectTreeView::Directory)) {
		parentPath = pathText(currentIdx);
	} else { // 否则path为父节点的path, 这里假定所有节点是文件/目录/Project, 且文件节点的父节点是目录或Project, 目前没有引进虚拟节点
		parentPath = parentPathText(currentIdx);
	}

	if (!parentPath.isEmpty())
		emit newHeaderWithPath(parentPath, projectName);
}

void ProjectTreeView::newSourceFile()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	QString type = typeText(currentIdx);
	QString parentPath = QString();
	QString projectName = projectNameText(currentIdx);
	// 如果当前节点类型是Project 或者 目录, 则path 为当前节点的path
	if (type == mimeTypeList.at(ProjectTreeView::OpenedProject) ||
		type == mimeTypeList.at(ProjectTreeView::Directory)) {
		parentPath = pathText(currentIdx);
	} else {  // 否则path为父节点的path, 这里假定所有节点是文件/目录/Project, 且文件节点的父节点是目录或Project, 目前没有引进虚拟节点
		parentPath = parentPathText(currentIdx);
	}
	
	if (!parentPath.isEmpty())
		emit newSourceWithPath(parentPath, projectName);
}

void ProjectTreeView::newTextFile()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	QString type = typeText(currentIdx);
	QString parentPath = QString();
	QString projectName = projectNameText(currentIdx);
	// 如果当前节点类型是Project 或者 目录, 则path 为当前节点的path
	if (type == mimeTypeList.at(ProjectTreeView::OpenedProject) ||
		type == mimeTypeList.at(ProjectTreeView::Directory)) {
		parentPath = pathText(currentIdx);
	} else { // 否则path为父节点的path, 这里假定所有节点是文件/目录/Project, 且文件节点的父节点是目录或Project, 目前没有引进虚拟节点
		parentPath = parentPathText(currentIdx);
	}
	
	if (!parentPath.isEmpty())
		emit newTextFileWithPath(parentPath, projectName);
}

void ProjectTreeView::openClosedProject()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	int row = currentIdx.row();
	QString projectName = model->item(row, 0)->text();
	projectManager->openClosedProject(projectName);
}

void ProjectTreeView::openFile()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;
	QString type = typeText(currentIdx);
	QString path = pathText(currentIdx);
	QString projectName = projectNameText(currentIdx);
	if (type == mimeTypeList.at(ProjectTreeView::Text) && !path.isEmpty())
		emit openFile(path, ProjectTreeView::Text, projectName);
}

// 返回index节点对应的路径节点
QStandardItem* ProjectTreeView::pathItem(const QModelIndex &index)
{
	if (!index.isValid())
		return 0;
	QModelIndex parent = index.parent();
	if (parent.isValid()) {
		QModelIndex pathIndex = parent.child(index.row(), 1);
		return model->itemFromIndex(pathIndex);
	} else {
		return model->item(index.row(), 1);
	}
}

// 返回index节点的文件路径
QString ProjectTreeView::pathText(const QModelIndex &index)
{
	if (!index.isValid())
		return QString();
	
	QModelIndex parent = index.parent();
	if (parent.isValid())
		return parent.child(index.row(), 1).data().toString();
	else
		return model->item(index.row(), 1)->text();
}

// 返回index节点的父节点的文件路径
QString ProjectTreeView::parentPathText(const QModelIndex &index)
{
	if (!index.isValid())
		return QString();
	
	QModelIndex parent = index.parent();
	if (!parent.isValid()) 
		return QString();
	
	QModelIndex grandfather = parent.parent();
	if (grandfather.isValid())
		return grandfather.child(parent.row(), 1).data().toString();
	else
		return model->item(parent.row(), 1)->text();
}

// 返回index节点的父节点的路径节点
QStandardItem* ProjectTreeView::parentPathItem(const QModelIndex &index)
{
	if (!index.isValid())
		return 0;
	QModelIndex parent = index.parent();

	if (!parent.isValid())
		return 0;
	
	QModelIndex grandfather = parent.parent();
	if (grandfather.isValid()) {
		QModelIndex pathIndex = grandfather.child(parent.row(), 1);
		return model->itemFromIndex(pathIndex);
	} else {
		return model->item(parent.row(), 1);
	}
}

// 返回index节点的父节点的类型
QString ProjectTreeView::parentTypeText(const QModelIndex &index)
{
	if (!index.isValid())
		return QString();
	
	QModelIndex parent = index.parent();
	if (!parent.isValid())
		return QString();
	
	QModelIndex grandfather = parent.parent();
	if (grandfather.isValid())
		return grandfather.child(parent.row(), 2).data().toString();
	else
		return model->item(parent.row(), 2)->text();
}

// 返回index节点的父节点对应的类型节点
QStandardItem* ProjectTreeView::parentTypeItem(const QModelIndex &index)
{
	if (!index.isValid())
		return 0;
	QModelIndex parent = index.parent();

	if (!parent.isValid())
		return 0;
	
	QModelIndex grandfather = parent.parent();
	if (grandfather.isValid()) {
		QModelIndex pathIndex = grandfather.child(parent.row(), 2);
		return model->itemFromIndex(pathIndex);
	} else {
		return model->item(parent.row(), 2);
	}
}

// 返回index节点所在项目的项目名称, 如果不是项目的文件则返回空字符串
QString ProjectTreeView::projectNameText(const QModelIndex &index)
{
	if (!index.isValid()) 
		return QString();
	
	QModelIndex parent = index.parent();
	if (parent.isValid())
		return parent.child(index.row(), 3).data().toString();
	else
		return model->item(index.row(), 3)->text();
}

QStandardItem* ProjectTreeView::projectNameItem(const QModelIndex &index)
{
	if (!index.isValid())
		return 0;
	
	QModelIndex parent = index.parent();
	if (parent.isValid()) {
		QModelIndex projectNameIndex = parent.child(index.row(), 3);
		return model->itemFromIndex(projectNameIndex);
	} else {
		return model->item(index.row(), 3);
	}
}

void ProjectTreeView::refresh()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	QString type = typeText(currentIdx);
	if (type == mimeTypeList.at(ProjectTreeView::Directory)) {
		refreshDirectory(currentIdx);
	} else if (type == mimeTypeList.at(ProjectTreeView::OpenedProject)){
		refreshProject(currentIdx);
	}
}

void ProjectTreeView::refreshDirectory(const QModelIndex &index)
{
	if (!index.isValid())
		return;
	
	QString type = typeText(index);
	if (type != mimeTypeList.at(ProjectTreeView::Directory))
		return;
	
	QString path = pathText(index);
	QString projectName = projectNameText(index);
	QList<QStandardItem*> list = buildDirectoryItem(path, projectName);
	QModelIndex parentIndex = index.parent();
	int row = index.row();
	if (parentIndex.isValid()) {
		QStandardItem *parentItem = model->itemFromIndex(parentIndex);
		parentItem->removeRow(row);
		parentItem->insertRow(row, list);
	} else {
		topFilePathItemMap[path] = list.at(0);
		model->removeRows(row, 1);
		model->insertRow(row, list);
	}
}

void ProjectTreeView::refreshProject(const QModelIndex &index)
{
	if (!index.isValid())
		return;
	
	QString type = typeText(index);
	if (type != mimeTypeList.at(ProjectTreeView::OpenedProject))
		return;
	int row = index.row();
	QString path = pathText(index);
	QString projectName = projectNameText(index);
	QList<QStandardItem*> list = buildOpenedProjectItem(projectName);
	topFilePathItemMap[path] = list.at(0);
	openedProjectItemMap[projectName] = list.at(0);
	model->removeRows(row, 1);
	model->insertRow(row, list);
}


void ProjectTreeView::renameDirectory()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	QString type = typeText(currentIdx);
	if (type != mimeTypeList.at(ProjectTreeView::Directory))
		return;

	// 如果当前节点类型是目录
	QDir parentDir(parentPathText(currentIdx));
	if (!parentDir.exists()) // 如果父目录不存在, 则返回
		return;

	QStandardItem *item0 = model->itemFromIndex(currentIdx);
	QStandardItem *item1 = pathItem(currentIdx);

	QString oldName = currentIdx.data().toString();
	RenameDialog *renameDialog = new RenameDialog("重命名", parentDir.entryList(), oldName);
	// 如果Accepted, 则重命名该目录, 如果成功, 则修改当前节点及字节点的路径为新路径, 并发送路劲已修改信号
	if (renameDialog->exec() == QDialog::Accepted) { 
		QString newName = renameDialog->newName();			 
		if (parentDir.rename(oldName, newName)) {		 
			QString oldPath = item1->text();		 
			QString newPath = parentDir.path() + "/" + newName; 
			item0->setText(newName); // 更新treeView 的第一列
			item0->setToolTip(newPath);
			item1->setText(newPath);	       // 更新treeView 的第二列
			changeChildrenPath(item0, oldPath, newPath); // 更新第一列的子节点的第二列
			emit pathChanged(oldPath, newPath, projectNameText(currentIdx)); // 发送路径改变信号
		}
	}
}

void ProjectTreeView::renameFile()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;
	QString type = typeText(currentIdx);
	// 如果当前节点类型是 Project 或者 目录, 则返回
	if (type == mimeTypeList.at(ProjectTreeView::OpenedProject) ||
		type == mimeTypeList.at(ProjectTreeView::Directory))
		return;

	QString parentPathStr = parentPathText(currentIdx);
	if (parentPathStr.isEmpty())
		return;
	
	QDir parentDir(parentPathStr);
	if (!parentDir.exists())
		return;

	QStandardItem *item0 = model->itemFromIndex(currentIdx);
	QStandardItem *item1 = pathItem(currentIdx);
	QString oldName = currentIdx.data().toString();
	RenameDialog *renameDialog = new RenameDialog("重命名文件", parentDir.entryList(), oldName, this);
	if (renameDialog->exec() == QDialog::Accepted) {
		QString newName = renameDialog->newName();
		if (parentDir.rename(oldName, newName)) {
			item0->setText(newName); // 更新第一列
			item0->setToolTip(parentDir.path() + "/" + newName);
			item1->setText(parentDir.path() + "/" + newName); // 更新第二列
			QString oldFilePath = parentDir.path() + "/" + oldName;
			QString newFilePath = parentDir.path() + "/" + newName;
			emit fileRenamed(oldFilePath, newFilePath, projectNameText(currentIdx)); // 发送文件名已修改信号
		}
	}
}

void ProjectTreeView::renameProject()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	int row = currentIdx.row();
	QString oldName = model->item(row, 0)->text();
	QString oldPath = model->item(row, 1)->text();
	QString type = model->item(row, 2)->text();

	// 如果节点的类型不是是OpenedProject
	if (type != mimeTypeList.at(ProjectTreeView::OpenedProject))
		return;

	// 所有项目
	QStringList existsNameList = projectManager->allProjectNames();
	RenameDialog *renameDialog =
		new RenameDialog("项目重命名", existsNameList, oldName,
						 "是否将该项目目录也重命名", this);
	// 如果Dialog返回Accepted
	if (renameDialog->exec() == QDialog::Accepted) {
		QString newName = renameDialog->newName();
		bool renamePath = false;
		if (renameDialog->checkState() == 2)   // 如果用户选择了重命名文件夹
			renamePath = true;
		projectManager->renameProject(oldName, newName, renamePath);
	}
	delete renameDialog;
}

void ProjectTreeView::renameProjectItem(const QString &oldName, const QString &newName)
{
	if (oldName == newName || !projectManager->hasProject(newName))
		return;

	if (openedProjectItemMap.contains(oldName)) {
		QStandardItem *headItem = openedProjectItemMap[oldName];
		headItem->setText(newName);
		QModelIndex index = headItem->index();
		QStandardItem *pathItem = model->item(index.row(), 1);
		QStandardItem *projectNameItem = model->item(index.row(), 3);
		openedProjectItemMap[newName] = headItem;
		openedProjectItemMap.remove(oldName);
		projectNameItem->setText(newName);
		changeChildrenProjectName(headItem, newName);       // 更新子节点的projectName节点

		// 如果路径也修改了, 则将该节点及其字节点的路径也修改
		if (projectManager->projectPath(newName) != pathItem->text()) {
			QString oldPath = pathItem->text();
			QString newPath = projectManager->projectPath(newName);
			pathItem->setText(newPath);
			headItem->setToolTip(newPath);
			topFilePathItemMap[newPath] = topFilePathItemMap[oldPath];
			topFilePathItemMap.remove(oldPath);
			changeChildrenPath(headItem, oldPath, newPath); // 更新子节点的path节点
		}
	} else if (closedProjectItemMap.contains(oldName)) {
		QStandardItem *headItem = closedProjectItemMap[oldName];
		headItem->setText(newName);
		QModelIndex index = headItem->index();
		QStandardItem *projectNameItem = model->item(index.row(), 3);
		projectNameItem->setText(newName);
	}
}

void ProjectTreeView::run(const QString &execFile)
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;

	QString projectName = projectNameText(currentIdx);
	emit run(execFile, QString(), projectName);
}

// 用于右击选择了"导入文件"选项, 初选一个对话框选择要导入的文件, 之后将文件导入到相应的位置
// private:

void ProjectTreeView::setCurrentProject()
{
	QModelIndex currentIdx = currentIndex();
	if (!currentIdx.isValid())
		return;
	projectManager->setCurrentProject(currentIdx.data().toString());
}

// 初始化 treeView
void ProjectTreeView::setUi()
{
	model = new QStandardItemModel(0, 4);
	setHeaderHidden(true);
	setModel(model);
	setColumnHidden(1, true);
	setColumnHidden(2, true);
	setColumnHidden(3, true);

	connect(this, SIGNAL(doubleClicked(const QModelIndex &)),
			this, SLOT(doubleClicked(const QModelIndex &)));

	mouseMenu = new Menu;
	QIcon icon_Build;
	icon_Build.addFile(QString::fromUtf8(":/images/build.png"), QSize(), QIcon::Normal, QIcon::On);
	buildMenu = new Menu;
	buildMenu->setTitle("编译");
	buildMenu->setIcon(icon_Build);

	QIcon icon_Run;
	icon_Run.addFile(QString::fromUtf8(":/images/run.png"), QSize(), QIcon::Normal, QIcon::On);
	runMenu = new Menu;
	runMenu->setTitle("运行");
	runMenu->setIcon(icon_Run);

	connect(buildMenu, SIGNAL(text(const QString &)), this, SLOT(build(const QString &)));
	connect(runMenu, SIGNAL(text(const QString &)), this, SLOT(run(const QString &)));
	
	// Action for opened project
	action_OpenProject = new QAction("打开项目", this);
	action_CloseProject = new QAction("关闭项目", this);

	QIcon icon_Delete;
	icon_Delete.addFile(QString::fromUtf8(":/images/delete.png"), QSize(), QIcon::Normal, QIcon::On);
	action_DeleteProject = new QAction("删除项目", this);
	action_DeleteProject->setIcon(icon_Delete);

	QIcon icon_ConfigProject;
	icon_ConfigProject.addFile(QString::fromUtf8(":/images/config.png"), QSize(), QIcon::Normal, QIcon::On);
	action_ConfigProject = new QAction("项目属性", this);
	action_ConfigProject->setIcon(icon_ConfigProject);

	action_RenameProject = new QAction("重命名项目", this);

	QIcon icon_Debug;
	icon_Debug.addFile(QString::fromUtf8(":/images/debug.png"), QSize(), QIcon::Normal, QIcon::On);
	action_Debug = new QAction("调试", this);
	action_Debug->setIcon(icon_Debug);

	action_SetAsCurrentProject = new QAction("设为当前项目", this);

	// action for closed project
	action_OpenClosedProject = new QAction("打开项目", this);
	
	// Action for code
	action_OpenFile = new QAction("打开", this);

	QIcon icon_NewSource;
	icon_NewSource.addFile(QString::fromUtf8(":/images/c_cpp_source.png"), QSize(), QIcon::Normal, QIcon::On);
	action_NewSource = new QAction("新建源文件", this);
	action_NewSource->setIcon(icon_NewSource);

	QIcon icon_NewHeader;
	icon_NewHeader.addFile(QString::fromUtf8(":/images/header.png"), QSize(), QIcon::Normal, QIcon::On);
	action_NewHeader = new QAction("新建头文件", this);
	action_NewHeader->setIcon(icon_NewHeader);

	QIcon icon_NewTextFile;
	icon_NewTextFile.addFile(QString::fromUtf8(":/images/text_file.png"), QSize(), QIcon::Normal, QIcon::On);
	action_NewTextFile = new QAction("新建文本", this);
	action_NewTextFile->setIcon(icon_NewTextFile);

	QIcon icon_NewDirectory;
	icon_NewDirectory.addFile(QString::fromUtf8(":/images/filetype/folder.png"), QSize(), QIcon::Normal, QIcon::On);
	action_NewDirectory = new QAction("创建目录", this);
	action_NewDirectory->setIcon(icon_NewDirectory);

	action_RenameFile = new QAction("重命名", this);
	action_RenameDirectory = new QAction("重命名", this);
	action_FindFile = new QAction("查找文件", this);
	action_FindCode = new QAction("查找代码", this);
	action_DeleteFile = new QAction("删除", this);
	action_DeleteFile->setIcon(icon_Delete);

	action_DeleteDirectory = new QAction("删除", this);
	action_DeleteDirectory->setIcon(icon_Delete);

	// Action for other
	QIcon icon_Refresh;
	icon_Refresh.addFile(QString::fromUtf8(":/images/refresh.png"), QSize(), QIcon::Normal, QIcon::On);
	action_Refresh = new QAction("刷新", this);
	action_Refresh->setIcon(icon_Refresh);

	action_LoadFile = new QAction("导入文件", this);
	action_LoadDirectory = new QAction("导入目录", this);

	openedProjectActionList << action_NewSource << action_NewHeader
							<< action_NewTextFile << action_NewDirectory
							<< action_Debug << action_Refresh
							<< action_SetAsCurrentProject << action_RenameProject
							<< action_FindFile << action_FindCode
							<< action_CloseProject << action_DeleteProject
							<< action_ConfigProject;

	closedProjectActionList << action_OpenClosedProject << action_RenameProject
							<< action_DeleteProject;
	
	directoryActionList	<< action_NewSource << action_NewHeader
						<< action_NewTextFile << action_NewDirectory
						<< action_Refresh << action_RenameDirectory
						<< action_FindFile << action_FindCode
						<< action_DeleteDirectory;

	textFileActionList << action_OpenFile << action_NewSource
					   << action_NewHeader << action_RenameFile
					   << action_FindCode << action_FindFile
					   << action_DeleteFile;

	otherFileActionList << action_NewHeader << action_NewSource
						<< action_RenameFile << action_DeleteFile;

	connect(action_OpenProject, SIGNAL(triggered()), this, SIGNAL(openProject()));
	connect(action_CloseProject, SIGNAL(triggered()), this, SLOT(closeProject()));
	connect(action_DeleteProject, SIGNAL(triggered()), this, SLOT(deleteProject()));
	connect(action_ConfigProject, SIGNAL(triggered()), this, SLOT(configProject()));
	connect(action_RenameProject, SIGNAL(triggered()), this, SLOT(renameProject()));
	connect(action_Debug, SIGNAL(triggered()), this, SLOT(debug()));
	connect(action_SetAsCurrentProject, SIGNAL(triggered()), this, SLOT(setCurrentProject()));

	connect(action_OpenClosedProject, SIGNAL(triggered()), this, SLOT(openClosedProject()));
	
	connect(action_OpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
	connect(action_NewSource, SIGNAL(triggered()), this, SLOT(newSourceFile()));
	connect(action_NewHeader, SIGNAL(triggered()), this, SLOT(newHeaderFile()));
	connect(action_NewTextFile, SIGNAL(triggered()), this, SLOT(newTextFile()));
	connect(action_NewDirectory, SIGNAL(triggered()), this, SLOT(newDirectory()));
	connect(action_RenameFile, SIGNAL(triggered()), this, SLOT(renameFile()));
	connect(action_RenameDirectory, SIGNAL(triggered()), this, SLOT(renameDirectory()));
	connect(action_FindFile, SIGNAL(triggered()), this, SLOT(findFile()));
	connect(action_FindCode, SIGNAL(triggered()), this, SLOT(findCode()));
	connect(action_DeleteFile, SIGNAL(triggered()), this, SLOT(deleteFile()));
	connect(action_DeleteDirectory, SIGNAL(triggered()), this, SLOT(deleteDirectory()));
	
	connect(action_Refresh, SIGNAL(triggered()), this, SLOT(refresh()));
	connect(action_LoadFile, SIGNAL(triggered()), this, SLOT(loadFile()));
	connect(action_LoadDirectory, SIGNAL(triggered()), this, SLOT(loadDirectory()));
}

void ProjectTreeView::sortChildrenItem(QStandardItem *parent)
{
	if (!parent)
		return;
	parent->sortChildren(0);
	parent->sortChildren(2);
}

// 返回index节点类型
QString ProjectTreeView::typeText(const QModelIndex &index)
{
	if (!index.isValid()) 
		return QString();
	
	QModelIndex parent = index.parent();
	if (parent.isValid())
		return parent.child(index.row(), 2).data().toString();
	else
		return model->item(index.row(), 2)->text();
}

// 返回index节点对应的类型节点
QStandardItem* ProjectTreeView::typeItem(const QModelIndex &index)
{
	if (!index.isValid())
		return 0;
	QModelIndex parent = index.parent();
	if (parent.isValid()) {
		QModelIndex typeIndex = parent.child(index.row(), 2);
		return model->itemFromIndex(typeIndex);
	} else {
		return model->item(index.row(), 2);
	}
}

