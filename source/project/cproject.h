#ifndef CPROJECT_H
#define CPROJECT_H

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QSettings>

class CProject : public QObject
{
	Q_OBJECT;
	
public:
	CProject(const QString &rootPath, QObject *parent = 0);

	bool update();
	bool isValid();
	QString errorInfo();
	QString rootPath();
	QString type();
	QString name();
	QString projectName();
	QString developMode();
	QString warnings();
	QString platform();
	QString std();
	QString cc();
	QString ccc();
	QString make();
	QString makefile();
	QString otherFlag();
	QStringList includePath();
	QStringList includeFile();
	QStringList libraryPath();
	QStringList libraryFile();
	QStringList ldLibraryPath();
	QStringList mainSources();
	QStringList executeFiles();
	QStringList targets();
	QStringList absoluteMainSources();
	QStringList mainSourceDepend(const QString &fileName);
	QStringList executeFileDepend(const QString &fileName);

	void setName(const QString &projectName);
	void setType(const QString &str);
	void setDevelopMode(const QString &mode);
	void setWarnings(const QString &str);
	void setPlatform(const QString &str);
	void setStd(const QString &str);
	void setCC(const QString &str);
	void setCCC(const QString &str);
	void setMake(const QString &str);
	void setMakefile(const QString &str);
	void setOtherFlag(const QString &str);
	void setIncludePath(const QString &str);
	void setIncludePath(const QStringList &list);
	void setIncludeFile(const QString &str);
	void setIncludeFile(const QStringList &list);
	void setLibraryPath(const QString &str);
	void setLibraryPath(const QStringList &list);
	void setLibraryFile(const QString &str);
	void setLibraryFile(const QStringList &list);
	void setMainSources(const QString &str);
	void setMainSources(const QStringList &list);
	void setMainSourceDepend(const QString &str);
	void setMainSourceDepend(const QStringList &list);
	// void setExecuteFile(const QString &str);
	// void setExecuteFile(const QStringList &list);
	
signals:
	void exceptionInfo(const QString &errorStr);
	void updated(const QString &projectName);
	
private:
	void updateStr();
	void updateInclude();
	void updateLibrary();
	void updateLdPath();
	void updateTargets();
	void updateMainSources();
	void updateMainSourceDepend();
	void updateExecuteFileDepend();
	
	QDateTime lastUpdateTime;
	QSettings *settings;
	QFileInfo settingsFileInfo;
	QString errorStr;
	bool valid;
	
	QString rootPathStr;
	QString typeStr;
	QString nameStr;
	QString developModeStr;
	QString warningsStr;
	QString platformStr;
	QString stdStr;
	QString ccStr;
	QString cccStr;
	QString makeStr;
	QString makefileStr;
	QString otherFlagStr;
	QStringList includePathList;
	QStringList includeFileList;
	QStringList libraryPathList;
	QStringList libraryFileList;
	QStringList ldPathList;
	QStringList targetList;
	QStringList mainSourceList;
	QStringList executeFileList;
	QMap<QString, QStringList> mainSourceDependMap;
	QMap<QString, QStringList> executeFileDependMap;
	QMap<QString, QString> executeFileRenameMap;
};

#endif
