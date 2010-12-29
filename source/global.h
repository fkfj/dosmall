#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore/QDir>
#include <QtCore/QSettings>

#include "project/projectmanager.h"

class Global
{
public:
	// 返回项目主文件夹, 默认创建项目的位置
	static QString projectHome() {
		if (projectHomeVar.isEmpty()) {
			QSettings *settings = new QSettings(globalConfigPathVar + "/conf", QSettings::IniFormat);
			if (!settings || settings->status() != QSettings::NoError)
				projectHomeVar = QDir::homePath();
			else 
				projectHomeVar = settings->value("ProjectHomePath").toString();
		}
		return projectHomeVar;
	}

	// 返回本应用程序的配置文件路径
	static QString globalConfigPath() {
		return globalConfigPathVar;
	}

	// 返回project配置文件模板文件的路径
	static QString projectConfigTemplate() {
		return projectConfigTemplateVar;
	}

	// 返回历史项目配置文件的路径
	static QString historyProjectFile() {
		return historyProjectFileVar;
	}

	static ProjectManager* projectManager() {
		if (!projectManagerVar)
			projectManagerVar = new ProjectManager(Global::historyProjectFileVar);;
		return projectManagerVar;
	}

private:
	static QString globalConfigPathVar;
	static QString projectHomeVar;
	static QString projectConfigTemplateVar;
	static QString historyProjectFileVar;
	static ProjectManager *projectManagerVar;
};

#endif
