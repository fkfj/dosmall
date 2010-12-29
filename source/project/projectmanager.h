#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QtCore/QSettings>
#include "cproject.h"

class ProjectManager : public QObject
{
	Q_OBJECT;
	
public:
	ProjectManager(const QString &history, QObject *parent = 0);

	// 返回所有项目的名称列表
	QStringList allProjectNames();

	QStringList allProjectPaths();

	// 返回当前项目的名称
	QString currentProjectName();

	// 返回当前项目对象
	CProject* currentProjectObject();

	// 返回已关闭项目名称列表
	QStringList closedProjectNames();

	// 将该projectName项目从所有项目列表和已打开项目列表中删除
	void deleteProject(const QString &projectName, bool rmFile = false);

	// 删除 projectName 对应的 projectObject
	void deleteProjectObject(const QString &projectName);

	// 检查所有项目名称列表中是否有projectName项目, 如果有则返回true, 否则返回false
	bool hasProject(const QString &projectName);

	// 提示信息列表, 每次调用会请空该列表
	QStringList hintInfo();
	
	bool isClosedProject(const QString &projectName);
	
	// 检查projectName是否在已打开项目列表中, 如果有则返回true, 否则返回false
	bool isOpenedProject(const QString &projectName);

	// 返回已打开项目名称列表
	QStringList openedProjectNames();

	// 返回项目projectName的根目录
	QString projectPath(const QString &projectName);

	// 返回项目名称为projectName的CProject对象
	CProject* projectObject(const QString &projectName);

	// 修改项目名称
	bool renameProject(const QString &oldName, const QString &newName, bool changePath = false);
	
	bool setProjectObject(const QString &projectName, CProject *project);

public slots:
	// 将该projectName项目从已打开项目列表中删除.
	void closeProject(const QString &projectName);

	bool openClosedProject(const QString &projectName);

	bool openProject(const QString &rootPath);

	// 设置当前项目为projectName
	bool setCurrentProject(const QString &projectName);
	
signals:
	void currentProjectChanged(const QString &projectName);
	void projectClosed(const QString &projectName);
	void projectOpened(const QString &projectName);
	void projectDeleted(const QString &projectName);
	void projectRenamed(const QString &oldName, const QString newName);
	void projectUpdated(const QString &projectName);
	void hint(const QString &text);
	
private:
	void loadHistoryProject();
	QString validProjectName(const QString &projectName);
	void ifNoCurrentProject(const QString &projectName = QString());
	
	QSettings *historySettings;
	QString currentProject;
	QMap<QString, QString> allProjectMap;	  // 所有项目 <name, rootPath> 的Map
	QMap<QString, CProject *> projectObjectMap;
	QStringList openedProjectList;            // 已导入 treeView 中的 projectName
	QStringList closedProjectList;
	QStringList pathList;
	QStringList hintInfoList;
};

#endif
