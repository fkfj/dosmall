#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDate>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include "cproject.h"

CProject::CProject(const QString &rootPath, QObject *parent)
	: QObject(parent)
{
	settingsFileInfo = QFileInfo(rootPath + "/project.small");
	rootPathStr = settingsFileInfo.absolutePath();
	settings = new QSettings(settingsFileInfo.absoluteFilePath(), QSettings::IniFormat);
	lastUpdateTime = QDateTime(QDate(1, 1, 1));
}

bool CProject::update()
{
	if (!isValid())
		return false;

	settingsFileInfo.refresh();
	if (lastUpdateTime >= settingsFileInfo.lastModified()) {
		return true;
	}

	updateStr();
	updateInclude();
	updateLibrary();
	updateLdPath();
	updateMainSources();
	updateMainSourceDepend();
	updateTargets();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);

	return true;
}

bool CProject::isValid()
{
	QDir dir(rootPathStr);
	if (!dir.exists()) {
		errorStr = "directory \"" + rootPathStr + "\" is not exists.";
		emit exceptionInfo(errorStr);
		return false;
	}

	settingsFileInfo.refresh();
	if (!(settingsFileInfo.exists() || settingsFileInfo.isFile())) {
		errorStr = "File \"" + settingsFileInfo.absoluteFilePath() + "\" is not exits.";
		emit exceptionInfo(errorStr);
		return false;
	}

	settings->sync();
	if (settings->status() == QSettings::AccessError) {
		errorStr = settingsFileInfo.absoluteFilePath() + " AccessError.";
		emit exceptionInfo(errorStr);
		return false;
	} else if (settings->status() == QSettings::FormatError) {
		errorStr = settingsFileInfo.absoluteFilePath() + " FormatError.";
		emit exceptionInfo(errorStr);
		return false;
	}

	errorStr = "No Error";

	return true;
}

QString CProject::errorInfo()
{
	return errorStr;
}

QString CProject::rootPath()
{
	update();
	return rootPathStr;
}

QString CProject::type()
{
	update();
	return typeStr;
}

QString CProject::name()
{
	update();
	return nameStr;
}

QString CProject::projectName()
{
	return name();
}

QString CProject::developMode()
{
	update();
	return developModeStr;
}

QString CProject::warnings()
{
	update();
	return warningsStr;
}

QString CProject::platform()
{
	update();
	return platformStr;
}

QString CProject::std()
{
	update();
	return stdStr;
}

QString CProject::cc()
{
	update();
	return ccStr;
}

QString CProject::ccc()
{
	update();
	return cccStr;
}

QString CProject::make()
{
	update();
	return makeStr;
}

QString CProject::makefile()
{
	update();
	return makefileStr;
}

QString CProject::otherFlag()
{
	update();
	return otherFlagStr;
}

QStringList CProject::includePath()
{
	update();
	return includePathList;
}

QStringList CProject::includeFile()
{
	update();
	return includeFileList;
}

QStringList CProject::libraryPath()
{
	update();
	return libraryPathList;
}

QStringList CProject::libraryFile()
{
	update();
	return libraryFileList;
}

QStringList CProject::ldLibraryPath()
{
	update();
	return ldPathList;
}

QStringList CProject::mainSources()
{
	update();
	return mainSourceList;
}

QStringList CProject::absoluteMainSources()
{
	update();
	QStringList absoluteMainSourceFilePath(mainSourceList);
	absoluteMainSourceFilePath.replaceInStrings(QRegExp("^"), rootPathStr + "/");
	return absoluteMainSourceFilePath;
}

QStringList CProject::executeFiles()
{
	update();
	return executeFileList;
}

QStringList CProject::targets()
{
	update();
	return targetList;
}

QStringList CProject::mainSourceDepend(const QString &fileName)
{
	update();
	return mainSourceDependMap[fileName];
}

QStringList CProject::executeFileDepend(const QString &fileName)
{
	update();
	return executeFileDependMap[fileName];
}

void CProject::setName(const QString &projectName)
{
	update();
	nameStr = projectName;
	settings->setValue("Name", nameStr);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setType(const QString &str)
{
	update();
	if (str == "C")
		typeStr = str;
	else
		typeStr = "C++";
	settings->setValue("Type", typeStr);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}


void CProject::setDevelopMode(const QString &mode)
{
	update();
	if (mode == "Debug" || mode == "Release" || mode == "UserDefined") {
		developModeStr = mode;
		settings->setValue("DevelopMode", mode);
		settings->sync();
		settingsFileInfo.refresh();
		lastUpdateTime = settingsFileInfo.lastModified();
		emit updated(nameStr);
	}
}
	
void CProject::setWarnings(const QString &str)
{
	update();
	warningsStr = str;
	settings->setValue("Warnings", warningsStr);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setPlatform(const QString &str)
{
	update();
	platformStr = str;
	settings->setValue("Platform", platformStr);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setStd(const QString &str)
{
	update();
	stdStr = str;
	settings->setValue("Std", stdStr);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setCC(const QString &str)
{
	update();
	ccStr = str;
	settings->setValue("CC", ccStr);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
}

void CProject::setCCC(const QString &str)
{
	update();
	cccStr = str;
	settings->setValue("CCC", cccStr);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setMake(const QString &str)
{
	update();
	stdStr = str;
	settings->setValue("Std", stdStr);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setMakefile(const QString &str)
{
	update();
	makefileStr = str;
	settings->setValue("Makefile", makefileStr);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setOtherFlag(const QString &str)
{
	update();
	otherFlagStr = str;
	settings->setValue("OtherFlag", otherFlagStr);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setIncludePath(const QString &str)
{
	setIncludePath(str.split(QRegExp("\\s*:\\s*"), QString::SkipEmptyParts));
	emit updated(nameStr);
}

// 不检查list的格式是否正确, 正确的格式是"-I$(Path)", 如果$(Path)含有空格, 则要把$(Path)放在""内
void CProject::setIncludePath(const QStringList &list)
{
	update();
	includePathList = list;
	settings->setValue("IncludePath", includePathList.join(" : "));
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setIncludeFile(const QString &str)
{
	setIncludeFile(str.split(QRegExp("\\s*:\\s*"), QString::SkipEmptyParts));
	emit updated(nameStr);
}

// 不检查list的格式是否正确, 正确的格式是"-include$(File)", 如果$(File)含有空格, 则要把$(File)放在""内
void CProject::setIncludeFile(const QStringList &list)
{
	update();									
	includeFileList = list;
	settings->setValue("IncludeFile", includeFileList.join(" : "));
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setLibraryPath(const QString &str)
{
	setLibraryPath(str.split(QRegExp("\\s*:\\s*"), QString::SkipEmptyParts));
	emit updated(nameStr);
}

// 不检查list的格式是否正确, 正确的格式是"-L$(Path)", 如果$(Path)含有空格, 则要把$(Path)放在""内
void CProject::setLibraryPath(const QStringList &list)
{
	update();
	libraryPathList = list;
	settings->setValue("LibraryPath", libraryPathList.join(" : "));
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	updateLdPath();
	emit updated(nameStr);
}

void CProject::setLibraryFile(const QString &str)
{
	setLibraryFile(str.split(QRegExp("\\s*:\\s*"), QString::SkipEmptyParts));
	emit updated(nameStr);
}

// 不检查list的格式是否正确, 正确的格式是"-l$(File)", 如果$(File)含有空格, 则要把$(File)放在""内
void CProject::setLibraryFile(const QStringList &list)
{
	update();
	libraryFileList = list;
	settings->setValue("LibraryFile", libraryFileList.join(" : "));
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	updateLdPath();
	emit updated(nameStr);
}

void CProject::setMainSources(const QString &str)
{
	QStringList tmpList = str.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	setMainSources(tmpList);
	emit updated(nameStr);
}

void CProject::setMainSources(const QStringList &list)
{
	update();
	QStringList tmpList = list;
	tmpList.replaceInStrings(QRegExp("^./"), "");

	// 首先将list中存在 而mainSourceList中不存在的 mainSource 添加到mainSourceList中,
	// 并设置相应mainSourceDepenMap 和 executeFileDpendMap.
	QStringList::const_iterator constIterator = tmpList.constBegin();
	QStringList::const_iterator endIterator = tmpList.constEnd();
	while (constIterator != endIterator) {
		// 如果mainSourceDependMap中已经有了该文件, 则不需要修改它的依赖关系, 因为用户可能已经设置了该文件的依赖关系
		// 否则, 初始化该文件的依赖关系为 "all", 且设置executeFileList 和 executeFileDependMap
		if(!mainSourceList.contains(*constIterator)) {
			mainSourceList << *constIterator;
			mainSourceDependMap[*constIterator] = QStringList() << "all";
			QFileInfo fileInfo(*constIterator);
			QString executeFile = fileInfo.path() + "/" + fileInfo.completeBaseName();
			executeFile.remove(QRegExp("^./"));
			executeFileList << executeFile;
			executeFileDependMap[executeFile] = QStringList() << "all" << executeFile + ".o";
		}
		++constIterator;
	}

	// 将原mainSourceList中存在, 而list中不存在的mainSource去掉
	constIterator = mainSourceList.constBegin();
	endIterator = mainSourceList.constEnd();
	while (constIterator != endIterator) {
		if (!tmpList.contains(*constIterator)) {
			mainSourceList.removeAll(*constIterator);
			mainSourceDependMap.remove(*constIterator);
			QFileInfo fileInfo(*constIterator);
			QString executeFile = fileInfo.path() + "/" + fileInfo.completeBaseName();
			executeFile.remove(QRegExp("^./"));
			executeFileDependMap.remove(executeFile);
		}
		++constIterator;
	}
	settings->setValue("MainSources", mainSourceList.join(" "));
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::setMainSourceDepend(const QString &str)
{
	setMainSourceDepend(str.split(QRegExp("\\s*:\\s*"), QString::SkipEmptyParts));
	emit updated(nameStr);
}

void CProject::setMainSourceDepend(const QStringList &list)
{
	update();
	// 首先根据list中依赖关系修改原来的依赖关系, 然后将list中没有包含的mainSource的依赖关系重置为all
	QStringList resetedDepend;
	
	// 根据list中依赖关系修改原来的依赖关系
	QStringList dependList = list;
	QStringList::const_iterator constIterator = dependList.constBegin();
	QStringList::const_iterator endIterator = dependList.constEnd();
	// 分析组内的依赖关系, 保存在dependMap中
	while (constIterator != endIterator) {
		// 将组内文件名保存在childList中
		QStringList childList = (*constIterator).split(QRegExp("\\s+"), QString::SkipEmptyParts);
		// 如果组内的单词个数小于2, 则无效, 放弃它处理下一个.
		if (childList.count() < 2) {
			++constIterator;
			continue;
		}

		// 主内第一个名称是main文件的依赖方式标志, "all"表示所有, "only"表示不依赖其它源文件, "part"标示依赖指定的源文件
		QString mode = childList.at(0).toLower();
		if (!(mode == "all" || mode == "part" || mode == "only")) {
			++constIterator;
			continue;
		}
		
		// 组内的第二个文件名是含main() 的源文件, 保存在mainSource中
		QString mainSource = childList.at(1);
		mainSource.remove(QRegExp("^./"));
		// 如果mainSourceList 中没有该mainSource, 则放弃该mainSource处理下一个.
		if (!mainSourceList.contains(mainSource)) {
			++constIterator;
			continue;
		}

		resetedDepend << mainSource; // 标记不需要重置为依赖"all"
		
		// 若mode = "part", 则它依赖于指定的源文件, dependSources, 为 "part" 和 mainSource后面的文件
		// 若mode = "only", 则它不依赖于任何其它源文件, dependSources为"only";
		// 若mode 不是上面两种情况, 则保持mainSourceDependMap中原来的值"all"
		QFileInfo fileInfo(mainSource);
		QString executeFile = fileInfo.path() + "/" + fileInfo.completeBaseName();
		executeFile.remove(QRegExp("^./"));
		if (mode == "part") {
			QStringList dependSources;
			QStringList objects;
			dependSources << "part";
			objects << "part" << executeFile + ".o";
			QString source = QString();
			QString object = QString();
			int count = childList.count();
			int i = 2;
			while (i < count) {
				source = childList.at(i);
				source.remove(QRegExp("^./"));
				dependSources << source;
				fileInfo.setFile(source);
				object = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".o";
				object.remove(QRegExp("^./"));
				objects << object;
				++i;
			}
			mainSourceDependMap[mainSource] = dependSources;
			executeFileDependMap[executeFile] = objects;
		} else if (mode == "only") {
			mainSourceDependMap[mainSource] = QStringList() << "only";
			executeFileDependMap[executeFile] = QStringList() << "only" << executeFile + ".o";
		} else {
			mainSourceDependMap[mainSource] = QStringList() << "all";
			executeFileDependMap[executeFile] = QStringList() << "all" << executeFile + ".o";
		}
		++constIterator;
	}

	// 将list中没有包含的mainSource的依赖关系重置为all
	constIterator = mainSourceList.constBegin();
	endIterator = mainSourceList.constEnd();
	while (constIterator != endIterator) {
		if (!resetedDepend.contains(*constIterator)) {
			mainSourceDependMap[*constIterator] = QStringList() << "all";
			QFileInfo fileInfo(*constIterator);
			QString executeFile = fileInfo.path() + "/" + fileInfo.completeBaseName();
			executeFile.remove(QRegExp("./"));
			executeFileDependMap[executeFile] = QStringList() << "all" << executeFile + ".o";
		}
		++constIterator;
	}

	// 将新的设置转化为字符串, 并写入settings中
	QString mainSourceDependValue = QString();
	constIterator = mainSourceList.constBegin();
	endIterator = mainSourceList.constEnd();
	while (constIterator != endIterator) {
		QStringList list = mainSourceDependMap[*constIterator];
		mainSourceDependValue += list.at(0);
		mainSourceDependValue += " " + *constIterator + " ";
		list.removeAt(0);
		mainSourceDependValue += list.join(" ") + ":";
		++constIterator;
	}
	mainSourceDependValue.remove(QRegExp(":$"));
	settings->setValue("MainSourceDepend", mainSourceDependValue);
	settings->sync();
	settingsFileInfo.refresh();
	lastUpdateTime = settingsFileInfo.lastModified();
	emit updated(nameStr);
}

void CProject::updateStr()
{
	typeStr = settings->value("Type").toString();
	nameStr = settings->value("Name").toString();
	developModeStr = settings->value("DevelopMode", "Debug").toString();
	warningsStr = settings->value("Warnings").toString();
	platformStr = settings->value("Platform").toString();
	stdStr = settings->value("Std").toString();
	ccStr = settings->value("CC", "gcc").toString();
	cccStr = settings->value("CCC", "g++").toString();
	makeStr = settings->value("Make").toString();
	makefileStr = settings->value("Makefile").toString();
	if (!makefileStr.isEmpty()) {
		QDir rootDir(rootPathStr);
		QFileInfo fileInfo(makefileStr);
		if (rootDir.cd(fileInfo.path())) {
			makefileStr = rootDir.absolutePath() + "/" + fileInfo.fileName();
			if (!QFile::exists(makefileStr))
				makefileStr = QString();
		} else {
			makefileStr = QString();
		}
	}
	otherFlagStr = settings->value("OtherFlag").toString();
}

void CProject::updateInclude()
{
	QStringList::const_iterator constIterator;
	QStringList::const_iterator endIterator;

	// includePathList
	QString includePathValue = settings->value("IncludePath").toString();
	if (!includePathValue.isEmpty()) {
		QStringList list = includePathValue.split(QRegExp("\\s*:\\s*"), QString::SkipEmptyParts);
		constIterator = list.constBegin();
		endIterator = list.constEnd();
		while (constIterator != endIterator) {
			QString item = *constIterator;
			item.remove(QRegExp("^-I\\s*"));
			if (!item.startsWith("\"")  && item.contains(" ")) {
				item = "\"" + item + "\"";
			}
			includePathList<<  "-I" + item;
			++constIterator;
		}
	}

	// includeFileList
	QString includeFileValue = settings->value("IncludeFile").toString();
	if (!includeFileValue.isEmpty()) {
		QStringList list = includeFileValue.split(QRegExp("\\s*:\\s*"), QString::SkipEmptyParts);
		constIterator = list.constBegin();
		endIterator = list.constEnd();
		while (constIterator != endIterator) {
			QString item = *constIterator;
			item.remove(QRegExp("^-include\\s*"));
			if (!item.startsWith("\"")  && item.contains(" ")) {
				item = "\"" + item + "\"";
			}
			includeFileList << "-include" + item;
			++constIterator;
		}
	}

}

void CProject::updateLibrary()
{
	QStringList::const_iterator constIterator;
	QStringList::const_iterator endIterator;

	//libraryPathList
	QString libraryPathValue = settings->value("LibraryPath").toString();
	if (!libraryPathValue.isEmpty()) {
		QStringList list = libraryPathValue.split(QRegExp("\\s*:\\s*"), QString::SkipEmptyParts);
		constIterator = list.constBegin();
		endIterator = list.constEnd();
		while (constIterator != endIterator) {
			QString item = *constIterator;
			item.remove(QRegExp("^-L\\s*"));
			if (!item.startsWith("\"")  && item.contains(" ")) {
				item = "\"" + item + "\"";
			}
			libraryPathList << "-L" + item;
			++constIterator;
		}
	}

	// librayFileList
	QString libraryFileValue = settings->value("LibraryFile").toString();
	if (!libraryFileValue.isEmpty()) {
		QStringList list = libraryFileValue.split(QRegExp("\\s*:\\s*"), QString::SkipEmptyParts);
		constIterator = list.constBegin();
		endIterator = list.constEnd();
		while (constIterator != endIterator) {
			QString item = *constIterator;
			item.remove(QRegExp("^-l\\s*"));
			if (!item.startsWith("\"")  && item.contains(" ")) {
				item = "\"" + item + "\"";
			}
			libraryFileList << "-l" + item;
			++constIterator;
		}
	}

}

void CProject::updateLdPath()
{
	// ldPathList
	QStringList paths = libraryPathList;
	QStringList files = libraryFileList;

	if (!paths.isEmpty()) {
		// 去除 paths 中开头的"-L", 开头的双引号, 末尾的双引号
		paths.replaceInStrings(QRegExp("^-L\\s*"), "");
		paths.replaceInStrings(QRegExp("^\""), "");
		paths.replaceInStrings(QRegExp("\"$"), "");
		ldPathList << paths;
	}

	if (!files.isEmpty()) {
		// 去除 files 中开头的"-l", 开头的双引号, 末尾的双引号 
		files.replaceInStrings(QRegExp("^-l\\s*"), "");
		files.replaceInStrings(QRegExp("^\""), "");
		files.replaceInStrings(QRegExp("\"$"), "");
		
		// 如果 files 中的链接文件不是单独的"-l名称", 而是"-l路径/名称", 则将路径加入到ldPathList中.
		QStringList::const_iterator constIterator = files.constBegin();
		QStringList::const_iterator endIterator = files.constEnd();
		while (constIterator != endIterator) {
			QString str = *constIterator;
			if (str.contains("/")) { // 如果包含"/"
				str.remove(str.lastIndexOf("/"), str.length() - 1); // 去掉最后一个"/" 到末尾
				if (!ldPathList.contains(str))						// 如果ldPathList没有包含str
					ldPathList << str;
			}
			++constIterator;
		}
	}
}

void CProject::updateMainSources()
{
	mainSourceList.clear();
	QString mainSourcesValue = settings->value("MainSources").toString();
	if (mainSourcesValue.isEmpty()) {
		executeFileList << nameStr;
		executeFileDependMap[nameStr] = QStringList() << "all";
		return;
	}

	mainSourceList = mainSourcesValue.split(QRegExp("\\s+"), QString::SkipEmptyParts);
	mainSourceList.replaceInStrings(QRegExp("^./"), "");
	QStringList::const_iterator constIterator = mainSourceList.constBegin();
	QStringList::const_iterator endIterator = mainSourceList.constEnd();
	executeFileList.clear();
	mainSourceDependMap.clear();
	executeFileDependMap.clear();
	while (constIterator != endIterator) {
		QFileInfo fileInfo(*constIterator);
		mainSourceDependMap[*constIterator] = QStringList() << "all";
		QString executeFile = fileInfo.path() + "/" + fileInfo.completeBaseName();
		executeFile.remove(QRegExp("^./"));
		executeFileList << executeFile;
		executeFileDependMap[executeFile] = QStringList() << "all" << executeFile + ".o";
		++constIterator;
	}
}

void CProject::updateMainSourceDepend()
{
	// mainSourceDependMap
	QString mainSourceDependValue = settings->value("MainSourceDepend").toString();
	if (mainSourceDependValue.isEmpty())
		return;

	// 将分组保存在dependList中
	QStringList dependList = mainSourceDependValue.split(QRegExp("\\s*:\\s*"), QString::SkipEmptyParts);
	QStringList::const_iterator constIterator = dependList.constBegin();
	QStringList::const_iterator endIterator = dependList.constEnd();
	// 分析组内的依赖关系, 保存在dependMap中
	while (constIterator != endIterator) {
		// 将组内文件名保存在childList中
		QStringList childList = (*constIterator).split(QRegExp("\\s+"), QString::SkipEmptyParts);
		// 如果组内的单词个数小于2, 则无效, 放弃它处理下一个.
		if (childList.count() < 2) {
			++constIterator;
			continue;
		}

		// 主内第一个名称是main文件的依赖方式标志, "all"表示所有, "only"表示不依赖其它源文件, "part"标示依赖指定的源文件
		QString mode = childList.at(0);
		// 组内的第二个文件名是含main() 的源文件, 保存在mainSource中
		QString mainSource = childList.at(1);
		mainSource.remove(QRegExp("^./"));
		// 如果mainSourceList 中没有该mainSource, 则放弃该mainSource处理下一个.
		if (!mainSourceList.contains(mainSource)) {
			++constIterator;
			continue;
		}

		// 若mode = "only", 则它不依赖于任何其它源文件, dependSources为"only";
		// 若mode = "part", 则它依赖于指定的源文件, dependSources, 为 "part" 和 mainSource后面的文件
		// 若mode 不是上面两种情况, 则保持mainSourceDependMap中原来的值"all"
		QFileInfo fileInfo(mainSource);
		QString executeFile = fileInfo.path() + "/" + fileInfo.completeBaseName();
		executeFile.remove(QRegExp("^./"));
		if (mode == "only") {
			mainSourceDependMap[mainSource] = QStringList() << "only";
			executeFileDependMap[executeFile].removeAt(0);
			executeFileDependMap[executeFile].insert(0, "only");
		} else if (mode == "part") {
			QStringList dependSources;
			QStringList objects;
			dependSources << "part";
			objects << "part" << executeFile + ".o";
			QString source = QString();
			QString object = QString();
			int count = childList.count();
			int i = 2;
			while (i < count) {
				source = childList.at(i);
				source.remove(QRegExp("^./"));
				dependSources << source;
				fileInfo.setFile(source);
				object = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".o";
				object.remove(QRegExp("^./"));
				objects << object;
				++i;
			}
			mainSourceDependMap[mainSource] = dependSources;
			executeFileDependMap[executeFile] = objects;
		}
		++constIterator;
	}
}

void CProject::updateTargets()
{
	targetList << "all" << executeFileList << "clean";
}
	
