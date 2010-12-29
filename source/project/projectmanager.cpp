#include <QtCore/QProcess>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QFile>

#include "../global.h"
#include "projectmanager.h"

ProjectManager::ProjectManager(const QString &history, QObject *parent)
	:QObject(parent)
{
	historySettings = new QSettings(history, QSettings::IniFormat);
	currentProject = QString();
	allProjectMap.clear();	  // 所有项目 <name, rootPath> 的Map
	projectObjectMap.clear();
	openedProjectList.clear();            // 已导入 treeView 中的 projectName
	closedProjectList.clear();
	pathList.clear();
	loadHistoryProject();
	ifNoCurrentProject();
}

// 返回所有项目的名称列表
QStringList ProjectManager::allProjectNames()
{
	return allProjectMap.keys();
}

QStringList ProjectManager::allProjectPaths()
{
	return pathList;
}

// 返回当前项目的名称
QString ProjectManager::currentProjectName()
{
	return currentProject;
}

CProject* ProjectManager::currentProjectObject()
{
	return projectObject(currentProject);
}
		
QStringList ProjectManager::closedProjectNames()
{
	return closedProjectList;
}

// 将该projectName项目从已打开项目列表中删除.
void ProjectManager::closeProject(const QString &projectName)
{
	if (!allProjectMap.contains(projectName))
		return;

	openedProjectList.removeAll(projectName);
	closedProjectList << projectName;
	historySettings->setValue(projectName + "/Status", "Off");
	historySettings->sync();
	emit projectClosed(projectName);
	
	if (currentProject == projectName) {
		currentProject = QString();
		ifNoCurrentProject();
	}
}

// 将该projectName项目从所有项目列表和已打开项目列表中删除
void ProjectManager::deleteProject(const QString &projectName, bool deleteFile)
{
	if (!allProjectMap.contains(projectName))
		return;

	if (deleteFile) 
		QProcess::execute("rm", QStringList() << "-rf" << allProjectMap[projectName]);

	pathList.removeAll(allProjectMap[projectName]);
	allProjectMap.remove(projectName);
	openedProjectList.removeAll(projectName);
	closedProjectList.removeAll(projectName);
	deleteProjectObject(projectName);
	historySettings->remove(projectName);
	historySettings->sync();

	emit projectDeleted(projectName);

	if (currentProject == projectName) {
		currentProject = QString();
		ifNoCurrentProject();
	}
}

void ProjectManager::deleteProjectObject(const QString &projectName)
{
	if (projectObjectMap.contains(projectName)) {
		disconnect(projectObjectMap[projectName], 0, this, 0);
		delete projectObjectMap[projectName];
		projectObjectMap.remove(projectName);
	}
}

// 检查所有项目名称列表中是否有projectName项目, 如果有则返回true, 否则返回false
bool ProjectManager::hasProject(const QString &projectName)
{
	return allProjectMap.contains(projectName);
}

// 返回提示信息列表, 每次调用都会清空提示信息列表
QStringList ProjectManager::hintInfo()
{
	QStringList list = hintInfoList;
	hintInfoList.clear();
	return list;
}

bool ProjectManager::isClosedProject(const QString &projectName)
{
	return closedProjectList.contains(projectName);
}

void ProjectManager::ifNoCurrentProject(const QString &projectName)
{
	if (currentProject.isEmpty()) {
		if (!projectName.isEmpty()) {
			setCurrentProject(projectName);
		} else if (openedProjectList.count() > 0) {
			setCurrentProject(openedProjectList.at(0));
		}
	}
}

// 检查projectName是否在已打开项目列表中, 如果有则返回true, 否则返回false
bool ProjectManager::isOpenedProject(const QString &projectName)
{
	return openedProjectList.contains(projectName);
}

void ProjectManager::loadHistoryProject()
{
	// 配置文件的每个group 都是项目根目录的绝对路径, 这样可以保持treeview中的每个group都是唯一的
	QStringList projectList = historySettings->childGroups();
	QStringList::const_iterator constIterator = projectList.constBegin();
	QStringList::const_iterator endIterator = projectList.constEnd();
	// 初始化 allProjectMap
	while (constIterator != endIterator) {
		if (!allProjectMap.contains(*constIterator))
			allProjectMap[*constIterator] = QString();
		++constIterator;
	}

	qDebug() << "导入历史项目中 ...";
	constIterator = projectList.constBegin();
	while (constIterator != endIterator) {
		qDebug() << "开始导入项目" << *constIterator << "...";
		historySettings->beginGroup(*constIterator);
		QString rootPath = historySettings->value("RootPath").toString();
		if (rootPath.contains(QRegExp("/\\s*$"))) {
			rootPath.remove(QRegExp("/\\s*$"));
			historySettings->setValue("RootPath", rootPath);
		}

		// 如果 pathList 中含有rootPath, 则说明该位置的项目已经导入了, 不需要重新导入它
		// 从 historySettings 和 allProjectMap 中删除它, 并继续下一个项目
		if (pathList.contains(rootPath)) {
			historySettings->endGroup();
			qDebug() << "目录" << rootPath << "中的项目已经导入, 不需要重新导入";
			historySettings->remove(*constIterator);
			allProjectMap.remove(*constIterator);
			++constIterator;
			continue;
		}
			
		// 如果rootPath 为空 或者 不存在该目录, 则从historySettings和allProjectMap中删除该项目
		if (rootPath.isEmpty() || !QFile::exists(rootPath)) {
			historySettings->endGroup();
			qDebug() << "目录" << rootPath + "不存在, 放弃该项目" << *constIterator;
			allProjectMap.remove(*constIterator);
			historySettings->remove(*constIterator);
			++constIterator;
			continue;
		}

		// 查看是否存在project.small文件, 如果不存在, 则拷贝一份模板, 如果拷贝失败, 则从historySettings和allProjectMap中删除该项目
		// 之所以在不存在的情况加拷贝一份模板, 是因为用户可能误删了配置文件, 所以重新建立它
		bool isNew = false;
		QFileInfo fileInfo(rootPath + "/project.small");
		if (!fileInfo.exists()) {
			if (!QFile::copy(Global::projectConfigTemplate(), fileInfo.absoluteFilePath())) {
				QString errorInfo = "拷贝项目配置文件失败 \ncopy from:" + Global::projectConfigTemplate() +
					"\n copy to:" + fileInfo.absoluteFilePath() + "\n没有导入该项目\"" + *constIterator + "\"";
				qDebug() << errorInfo;
				hintInfoList << errorInfo;
				emit hint(errorInfo);
				historySettings->endGroup();
				allProjectMap.remove(*constIterator);
				historySettings->remove(*constIterator);
				++constIterator;
				continue;
			}
			isNew = true;
		} else if (!fileInfo.isFile() || !fileInfo.isWritable() || !fileInfo.isReadable()) {
			// 如果project.small不是普通文件或者没有读写权限, 则从historySettings和allProjectMap中删除该项目
			QString errorInfo = "文件\"" + fileInfo.absoluteFilePath() + "\"不是普通文件, 或者您没有它读写权限"
				+ "\n没有导入该项目\"" + *constIterator + "\"";
			qDebug() << errorInfo;
			hintInfoList << errorInfo;
			emit hint(errorInfo);
			historySettings->endGroup();
			allProjectMap.remove(*constIterator);
			historySettings->remove(*constIterator);
			++constIterator;
			continue;
		}
		
		QSettings *projectSettings = new QSettings(fileInfo.absoluteFilePath(), QSettings::IniFormat);
		// 如果配置文件是复制得到的,  设置 Name = *constIterator
		if (isNew || projectSettings->value("Name").toString().isEmpty())	{
			projectSettings->setValue("Name", *constIterator);
			projectSettings->sync();
		}

		QString projectName = projectSettings->value("Name").toString();

		// 如果project.small文件中的Name 与 历史配置文件的项目名称变量不一致, 则使之尽可能与项目配置文件一致
		if (projectName != *constIterator) {
			projectName = validProjectName(projectName);
			qDebug() << "项目名称" << *constIterator << "与project.small配置文件中的Name不一致, 修改项目名称";
			qDebug() << "修改后的名称为" << projectName;
			// 暂存historySettings中项目oldName配置
			QString status = historySettings->value("Status", "On").toString();
			QString isCurrent = historySettings->value("IsCurrent", "False").toString();
			QString path = historySettings->value("RootPath").toString();
			historySettings->endGroup();

			// 移除 historySettings中的oldName配置, 加入newName的配置
			historySettings->remove(*constIterator);
			historySettings->setValue(projectName + "/Status", status);
			historySettings->setValue(projectName + "/IsCurrent", isCurrent);
			historySettings->setValue(projectName + "/RootPath", path);
			historySettings->sync();
			historySettings->beginGroup(projectName);
			
			// 更新项目配置文件
			QSettings *projectSettings = new QSettings(rootPath + "/project.small", QSettings::IniFormat);
			projectSettings->setValue("Name", projectName);
			projectSettings->sync();
			delete projectSettings;
		}

		allProjectMap[projectName] = rootPath;
		pathList << rootPath;
		// 如果Status = On/on/oN
		if (historySettings->value("Status", "On").toString().toLower() == "on") {
			openedProjectList << projectName;
			QString isCurrent = historySettings->value("IsCurrent", "False").toString().toLower();
			// 如果currentProject为空, 且IsCurrent = True, 则设置currentProject = projectName
			if (isCurrent == "true" && currentProject.isEmpty())
				currentProject = projectName;
		} else {
			closedProjectList << projectName;
		}

		historySettings->endGroup();
		qDebug() << "导入项目" <<  projectName <<  "成功.";
		++constIterator;
	}
	historySettings->sync();
	qDebug() << "所有项目导入完成.";
}

// 将项目projectName添加到一打开项目列表中, 如果在所有项目列表中没有该项目, 则返回false, 否则返回true
bool ProjectManager::openClosedProject(const QString &projectName)
{
	if (allProjectMap.contains(projectName) && closedProjectList.contains(projectName)) {
		openedProjectList << projectName;
		closedProjectList.removeAll(projectName);
		historySettings->setValue(projectName + "/Status", "On");
		historySettings->sync();
		emit projectOpened(projectName);
		ifNoCurrentProject(projectName);
		return true;
	} else {
		return false;
	}
}

// 返回已打开项目名称列表
QStringList ProjectManager::openedProjectNames()
{
	return openedProjectList;
}

// 将projectName添加到所有项目列表以及一打开项目列表中, 如果在所有项目列表中已经有了该项目,
// 则检查该项目的path是否相等, 如果相等则添加该项目到一打开项目列表中并返true, 否则什么都不做并返回false.
bool ProjectManager::openProject(const QString &rootPath)
{
	QString path = rootPath;
	path.remove(QRegExp("/\\s*$"));
	QFileInfo fileInfo(path + "/project.small");
	if (!fileInfo.exists() || !fileInfo.isFile())
		return false;
	
	QSettings *projectSettings = new QSettings(fileInfo.absoluteFilePath(), QSettings::IniFormat);
	QString name = projectSettings->value("Name").toString();

	// 5种情况
	// 1. allProjectMap 没有key name, 且path也不为 allProjectMap中的值
	//    这种情况属于打开一个不在历史项目中的项目, 正常导入该项目, 发送已导入信号
	// 2. allProjectMap 没有key name, 且path = allProjectMap[otherName], otherName != name
	//    这种情况属于打开一个已经在allProjectMap中的项目, 但是项目配置文件中的项目名称已经与导入时的项目名称不同了,
	//    应用程序保证了如果修改了项目名称, 则allProjectMap等数也将相应的修改, 所以属于异常
	//    处理方式: 将该项目名称改回来, 如果该项目已关闭, 则打开它
	// 3. allProjectMap 有了key name, 且path = allProjectMap[name], 且closedProjectList含有name
	//    这种情况属于打开一个已经关闭的项目
	// 4. allProjectMap 有了key name, 且path != allProjectMap[name]
	//    这种情况属于打开一个不在历史项目中的项目, 但是该项目名称与已有的项目重名了
	//    处理方式: 重命名该项目使之不重名, 导入该项目, 发送已导入信号
	// 5. allProjectMap 有了key name, 且path = allProjectMap[name], 且openedProjectList含有name
	//    这种情况属于打开一个已经关闭的项目
	if (!allProjectMap.contains(name) && !pathList.contains(path)) {
		allProjectMap[name] = path;
		openedProjectList << name;
		pathList << path;
		historySettings->setValue(name + "/Status", "On");
		historySettings->setValue(name + "/IsCurrent", "False");
		historySettings->setValue(name + "/RootPath", path);
		historySettings->sync();
		emit projectOpened(name);
		ifNoCurrentProject(name);
		return true;
	} else if (!allProjectMap.contains(name) && pathList.contains(path)) {
		QMap<QString, QString>::const_iterator i = allProjectMap.constBegin();
		QString projectName = QString();
		while (i != allProjectMap.constEnd()) {
			if (i.value() == path) {
				projectName = i.key();
				projectSettings->setValue("Name", projectName);
				break;
			}
			++i;
		}
		// 如果该项目已关闭, 则打开它
		if (closedProjectList.contains(projectName))
			openClosedProject(projectName);
		return true;
	} else if (allProjectMap.contains(name) && path == allProjectMap[name]
			   && closedProjectList.contains(name)) {
		return openClosedProject(name);
	} else if (allProjectMap.contains(name) && path != allProjectMap[name]) {
		name = validProjectName(name);
		projectSettings->setValue("Name", name);
		allProjectMap[name] = path;
		openedProjectList << name;
		pathList << path;
		emit projectOpened(name);
		ifNoCurrentProject(name);
		return true;
	} else {
		return true;
	}
}


// 返回项目projectName的根目录
QString ProjectManager::projectPath(const QString &projectName)
{
	return allProjectMap[projectName];
}


CProject* ProjectManager::projectObject(const QString &projectName)
{
	if (!hasProject(projectName))
		return 0;

	CProject *project = 0;
	if (projectObjectMap.contains(projectName)) {
		project = projectObjectMap[projectName];
		if (!project->isValid())
			project->update();
		return project;
	} else {
		CProject *project = new CProject(projectPath(projectName));
		connect(project, SIGNAL(updated(const QString &)), this, SIGNAL(projectUpdated(const QString &)));
		projectObjectMap[projectName] = project;
		return project;
	}
}

// 修改项目名称
bool ProjectManager::renameProject(const QString &oldName, const QString &newName, bool changePath)
{
	// 如果项目列表中没有oldName项目, 则返回false
	if (!allProjectMap.contains(oldName))
		return false;

	// 如果项目列表中有项目newName项目, 新项目名称重名了, 则返回false
	if (allProjectMap.contains(newName))
		return false;

	if (changePath) { 	// 如果选择重命名文件夹
		QString oldPath = allProjectMap[oldName];
		QDir dir(oldPath);
		if(dir.cdUp() && dir.rename(oldName, newName)) { // 如果重命名文件夹成功
			QString newPath = dir.path() + "/" + newName;
			allProjectMap.remove(oldName);
			allProjectMap[newName] = newPath;
			pathList.removeAll(oldPath);
			pathList << newPath;

			if (openedProjectList.contains(oldName)) {
				openedProjectList.removeAll(oldName);
				openedProjectList << newName;
			}

			if (closedProjectList.contains(oldName)) {
				closedProjectList.removeAll(oldName);
				closedProjectList << newName;
			}

			deleteProjectObject(oldName);
			// 暂存historySettings中项目oldName配置
			historySettings->beginGroup(oldName);
			QString status = historySettings->value("Status", "On").toString();
			QString isCurrent = historySettings->value("IsCurrent", "False").toString();
			historySettings->endGroup();

			// 移除 historySettings中的oldName配置, 加入newName的配置
			historySettings->remove(oldName);
			historySettings->setValue(newName + "/Status", status);
			historySettings->setValue(newName + "/IsCurrent", isCurrent);
			historySettings->setValue(newName + "/RootPath", newPath);
			historySettings->sync();

			// 更新项目配置文件
			QSettings *projectSettings = new QSettings(newPath + "/project.small", QSettings::IniFormat);
			projectSettings->setValue("Name", newName);
			projectSettings->sync();
			delete projectSettings;
			
			emit projectRenamed(oldName, newName);
			return true;
		} else {				// 如果重命名文件夹不成功
			return false;
		}
	} else { 					// 如果不重命名文件夹
		allProjectMap[newName] = allProjectMap[oldName];
		allProjectMap.remove(oldName);

		if (openedProjectList.contains(oldName)) {
			openedProjectList.removeAll(oldName);
			openedProjectList << newName;
		}

		if (closedProjectList.contains(oldName)) {
			closedProjectList.removeAll(oldName);
			closedProjectList << newName;
		}

		if (projectObjectMap.contains(oldName)) {
			projectObjectMap[newName] = projectObjectMap[oldName];
			projectObjectMap[newName]->setName(newName);
		} else {
			// 更新项目配置文件
			QSettings *projectSettings = new QSettings( projectPath(newName) + "/project.small", QSettings::IniFormat);
			projectSettings->setValue("Name", newName);
			projectSettings->sync();
			delete projectSettings;
		}

		// 暂存historySettings中项目oldName配置
		historySettings->beginGroup(oldName);
		QString status = historySettings->value("Status", "On").toString();
		QString isCurrent = historySettings->value("IsCurrent", "False").toString();
		QString path = historySettings->value("RootPath").toString();
		historySettings->endGroup();

		// 移除 historySettings中的oldName配置, 加入newName的配置
		historySettings->remove(oldName);
		historySettings->setValue(newName + "/Status", status);
		historySettings->setValue(newName + "/IsCurrent", isCurrent);
		historySettings->setValue(newName + "/RootPath", path);
		historySettings->sync();
		return true;
	}
}

// 设置当前项目为projectName
bool ProjectManager::setCurrentProject(const QString &projectName)
{
	if (projectName == currentProject) {
		return true;
	} else if (projectName.isEmpty()) {
		currentProject = QString();
		emit currentProjectChanged(currentProject);
		return true;
	} else if (openedProjectList.contains(projectName)) {
		currentProject = projectName;
		emit currentProjectChanged(currentProject);
		return true;
	} else {
		return false;
	}
}


bool ProjectManager::setProjectObject(const QString &projectName, CProject *project)
{
	if (!hasProject(projectName))
		return false;

	if (project != 0 && project->name() == projectName
		&& project->rootPath() == allProjectMap[projectName]) {
		deleteProjectObject(projectName);
		projectObjectMap[projectName] = project;
		connect(project, SIGNAL(update(const QString &)), this, SIGNAL(projectUpdated(const QString &)));
		return true;
	} else {
		return false;
	}
}

QString ProjectManager::validProjectName(const QString &projectName)
{
	QStringList allNames = allProjectNames();
	if (!allNames.contains(projectName))
		return projectName;
	
	char i = '1';
	QString tmpName = projectName;
	while (allNames.contains(tmpName)) {
		tmpName = projectName + "_" + i;
		++i;
	}
	return tmpName;
}
