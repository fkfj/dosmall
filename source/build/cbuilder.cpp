#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QByteArray>
#include <QtGui/QMessageBox>

#include <QtCore/QDebug>
#include "cbuilder.h"
#include "../tools/filetools.h"

CBuilder::CBuilder(CProject *cproject, QObject *parent)
	: QThread(parent)
{
	cprojectObject = cproject;
	makeProcess = 0;
	rootPath = cprojectObject->rootPath();
	scan = true;
	connect(this, SIGNAL(finished()), this, SLOT(make()));
} // CBuilder

CBuilder::~CBuilder()
{
}

void CBuilder::run()
{
	valid = false;
	absoluteMainSources= cprojectObject->absoluteMainSources();
	developMode = cprojectObject->developMode();
	if (developMode == "UserDefined") {
		makefile = cprojectObject->makefile();
		QFileInfo fileInfo(makefile);
		if (!fileInfo.exists() && !fileInfo.isFile()) {
			emit buildError("Makefile not exists \""
							+ fileInfo.absoluteFilePath() + "\".");
			valid = true;
			return;
		}
		workingDir = fileInfo.path();
		makefile = fileInfo.fileName();
		valid = true;
		return;
	}

	FileTools fileTools;
	if (!fileTools.mkdir(rootPath + "/" + developMode)) {
		emit buildError(fileTools.errorString());
		return;
	}

	subdirs.clear();
	buildErrorStr.clear();
	if (scan) {
		scanSources(rootPath);	// 扫描源文件
		if (!buildErrorStr.isEmpty()) {
			emit buildError(buildErrorStr);
			buildErrorStr.clear();
		}

		buildErrorStr.clear();
		if (!buildMakefile()) {
			emit buildError(buildErrorStr);
			buildErrorStr.clear();
			return;
		}
	}
	valid = true;
} // run

bool CBuilder::isBuilding()
{
	if (isRunning())
		return true;
	else if (!makeProcess)
		return false;
	else if (makeProcess->pid())
		return true;
	else 
		return false;
}

void CBuilder::build()
{
	// 如果正在运行, 则强制终止它并重新build
	if (isRunning()) {
		breakBuild();
	}
	start();
} // build();

void CBuilder::build(const QString &target)
{
	setTargets(target);
	build();
} // build

void CBuilder::buildAll()
{
	build("all");
}

void CBuilder::breakBuild()
{
	if (isRunning())
		quit();

	// 如果makeProcess 正在执行, 则kill它
	if (makeProcess->pid())
		makeProcess->kill();
} // breakBuild

void CBuilder::rebuild()
{
	breakBuild();
	start();
} // rebuild

void CBuilder::setTargets(const QString &target)
{
	targetList.clear();
	targetList << target;
} // setTargets

void CBuilder::setScan(bool reScan)
{
	scan = reScan;
} // setScan

void CBuilder::makeError(QProcess::ProcessError error)
{
	if (error == QProcess::FailedToStart) {
		emit buildError("make process failed start.");
	}
} // processError

void CBuilder::makeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QString str = makeProcess->readAll();

	if (exitStatus == QProcess::CrashExit)
		str += "build program crashed";
	emit buildFinished(str);
} // makeProcessFinished

void CBuilder::start()
{
	QThread::start();
} // start

void CBuilder::make()
{
	if (!valid) {
		return;
	}
		
	makeProcess = new QProcess;
	if (!makeProcess) {
		emit buildError("create build process failed.");
		return;
	}
	connect(makeProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this,
			SLOT(makeFinished(int, QProcess::ExitStatus)));
	connect(makeProcess, SIGNAL(error(QProcess::ProcessError)), this,
			SLOT(makeError(QProcess::ProcessError)));

	makeProcess->setProcessChannelMode(QProcess::MergedChannels);

	QStringList args;
	if (developMode == "UserDefined") {		// 如果是用户自定义模式
		makeProcess->setWorkingDirectory(workingDir);
		args << "-f" << makefile;
	} else {
		makeProcess->setWorkingDirectory(rootPath + "/" + developMode);
	}

	if (targetList.isEmpty()) 
		args << "all";
	else
		args << targetList;

	QString makeTool = cprojectObject->make();
	if (makeTool.isEmpty())
		makeTool = "make";
	makeProcess->start(makeTool, args);
} // runMakeProcess

// 递归扫描 dirPath目下的源文件, 并在含源文件的目录下创建subdir.mk文件
void CBuilder::scanSources(const QString &dirPath)
{
	QDir dir(dirPath);
	if (!dir.exists())
		return;

	// 扫描dirPath前, 先把上一个目录的少描记录清除.
	mainSources.clear();
	cSources.clear();
	cppSources.clear();
	ccSources.clear();
	cxxSources.clear();
	cpppSources.clear();
	
	// 先扫描源文件
	dir.setFilter(QDir::Files);
	QFileInfoList list = dir.entryInfoList();
	int size = list.size();
	for (int i = 0; i < size; ++i) {
		QFileInfo fileInfo = list.at(i);
		QString filePath = fileInfo.absoluteFilePath();
		QString suffix = fileInfo.suffix();

		// 如果是main() 文件, 则另存一份到mainSources
		if (absoluteMainSources.contains(filePath))
			mainSources << filePath;

		// 根据后缀名分类到cSources/cppSources/cxxSources/cpppSources
		if (suffix == "c") 
			cSources << filePath;
		else if (suffix == "cpp")
			cppSources << filePath;
		else if (suffix == "cc")
			ccSources << filePath;
		else if (suffix == "cxx") 
			cxxSources << filePath;
		else if (suffix == "c++")
			cpppSources << filePath;
	}

	// 如果源文件不为空, 则创建本目录的subdir.mk文件
	if (!(cSources.isEmpty() && cppSources.isEmpty() && ccSources.isEmpty()
		  && cxxSources.isEmpty() && cpppSources.isEmpty()))
		buildSubdirMk(dirPath);

	// 扫描子目录
	dir.setFilter(QDir::Dirs | QDir::Readable);
	list = dir.entryInfoList();
	size = list.size();
	for (int j = 0; j < size; ++j) {
		QFileInfo fileInfo = list.at(j);
		QString fileName = fileInfo.fileName();
		if (fileName == "." || fileName == "..")
			continue ;
		scanSources(fileInfo.filePath());
	}
} // scanSources

// 在dirPath 目录下创建subdir.mk文件
bool CBuilder::buildSubdirMk(const QString &dirPath)
{
	// 为了能够编译多个main()文件, 用　mainSource　保存项目所有含main() 的源文件列表.
	mainSources.replaceInStrings(rootPath, "..");  // 将源文件的绝对路径改为相对路径, 这里假设build的目录在项目根目录下
	cSources.replaceInStrings(rootPath, "..");	   // 同上
	cppSources.replaceInStrings(rootPath, ".."); 
	ccSources.replaceInStrings(rootPath, "..");
	cxxSources.replaceInStrings(rootPath, "..");
	cpppSources.replaceInStrings(rootPath, "..");

	QStringList objects;
	objects << cSources << cppSources << ccSources << cxxSources << cpppSources;  // 所有源文件将编译为.o文件
	// 用"o" 替代其它后缀
	objects.replaceInStrings(QRegExp("^../"), "./");  // 去掉源文件路径中的 "../", 因为目标文件在developMode目录下创建 
	objects.replaceInStrings(QRegExp(".c$"), ".o");   // 用后缀".o" 替换 ".c" 后缀
	objects.replaceInStrings(QRegExp(".cpp$"), ".o"); // 同上
	objects.replaceInStrings(QRegExp(".cc$"), ".o");  // 同上
	objects.replaceInStrings(QRegExp(".cxx$"), ".o"); // 同上
	objects.replaceInStrings(QRegExp(".c++$"), ".o"); // 同上

	QStringList depends;
	depends << objects;
	depends.replaceInStrings(QRegExp(".o$"), ".d");

	QStringList mainObjects;	                          // 含main() 的 源文件, 看着是特殊文件
	mainObjects << mainSources;
	mainObjects.replaceInStrings(QRegExp("^../"), "./");  // 去掉源文件路径中的 "../", 因为目标文件在developMode目录下创建 
	mainObjects.replaceInStrings(QRegExp(".c$"), ".o");   // 用后缀".o" 替换 ".c" 后缀
	mainObjects.replaceInStrings(QRegExp(".cpp$"), ".o"); // 同上
	mainObjects.replaceInStrings(QRegExp(".cc$"), ".o");  // 同上
	mainObjects.replaceInStrings(QRegExp(".cxx$"), ".o"); // 同上
	mainObjects.replaceInStrings(QRegExp(".c++$"), ".o"); // 同上

	// 从objects 中去除 mainObjects
	QStringList::const_iterator constIterator = mainObjects.constBegin();
	QStringList::const_iterator endIterator = mainObjects.constEnd();
	while (constIterator != endIterator) {
		objects.removeAll(*constIterator);
		++constIterator;
	}
	
	QByteArray block;

	addVariable("MAIN_SOURCES +=", mainSources, block);
	addVariable("C_SOURCES +=", cSources, block);
	addVariable("CPP_SOURCES +=", cppSources, block);
	addVariable("CC_SOURCES +=", ccSources, block);
	addVariable("CXX_SOURCES +=", cxxSources, block);
	addVariable("C++_SOURCES +=", cpppSources, block);
	addVariable("MAIN_OBJECTS +=", mainObjects, block);
	addVariable("OBJECTS +=", objects, block);
	addVariable("DEPENDS +=", depends, block);

	QString path = dirPath;
	path.remove(rootPath);
	path.remove(QRegExp("^/"));
	subdirs << path;

	QString subdir;
	if (!path.isEmpty()) {
		subdir = path + "/";
	}
	
	if (!cSources.isEmpty()) {
		block += "./" + subdir + "%.o: " + "../" + subdir + "%.c\n";
		block += "\t$(CC) $(FLAGS) $(LIBS) $(INCLUDES)";
		block += " -c -MMD -MP -MF\"$(@:%.o=%.d)\" -MT\"$(@:%.o=%.d)\" -o\"$@\" \"$<\"";
	}

	if (!cppSources.isEmpty()) {
		block += "./" + subdir + "%.o: " + "../" + subdir + "%.cpp\n";
		block += "\t$(CCC) $(FLAGS) $(LIBS) $(INCLUDES)";
		block += " -c -MMD -MP -MF\"$(@:%.o=%.d)\" -MT\"$(@:%.o=%.d)\" -o\"$@\" \"$<\"";
	}

	if (!ccSources.isEmpty()) {
		block += "./" + subdir + "%.o: " + "../" + subdir + "%.cc\n";
		block += "\t$(CCC) $(FLAGS) $(LIBS) $(INCLUDES)";
		block += " -c -MMD -MP -MF\"$(@:%.o=%.d)\" -MT\"$(@:%.o=%.d)\" -o\"$@\" \"$<\"";
	}

	if (!cxxSources.isEmpty()) {
		block += "./" + subdir + "%.o: " + "../" + subdir + "%.cxx\n";
		block += "\t$(CCC)  $(FLAGS) $(LIBS) $(INCLUDES)";
		block += " -c -MMD -MP -MF\"$(@:%.o=%.d)\" -MT\"$(@:%.o=%.d)\" -o\"$@\" \"$<\"";
	}

	if (!cpppSources.isEmpty()) {
		block += "./" + subdir + "%.o: " + "../" + subdir + "%.c++\n";
		block += "\t$(CCC)  $(FLAGS) $(LIBS) $(INCLUDES)";
		block += " -c -MMD -MP -MF\"$(@:%.o=%.d)\" -MT\"$(@:%.o=%.d)\" -o\"$@\" \"$<\"";
	}

	QString buildDir = rootPath + "/" + developMode + "/" + path;
	FileTools fileTools;
	if (!fileTools.mkdir(buildDir)) {
		buildErrorStr += "Error: Create directory:\"" + buildDir + "\":"
			+ fileTools.errorString() + "\n";
		return false;
	}
	
	QFile file(buildDir + "/subdir.mk");
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		buildErrorStr += "无法写入\"" + buildDir + "subdir.mk" + "\"\n";
		buildErrorStr += file.errorString() + "\n";
		return false;
	}
	file.write(block);
	file.close();
	
	return true;
} // buildSubdirMk

// 在developMode 目录下创建Makefile文件
bool CBuilder::buildMakefile()
{
	QByteArray block;
	
	// 编译器设置
	block += "CC := " + cprojectObject->cc() + "\n";
	block += "CCC := " + cprojectObject->ccc() + "\n";
	block += "STD := " + cprojectObject->std() + "\n";
	block += "PLATFORM := " + cprojectObject->platform() + "\n";
	block += "MAIN_SOURCES :=\n";
	block += "C_SOURCES :=\n";
	block += "CPP_SOURCES :=\n";
	block += "CC_SOURCES :=\n";
	block += "CXX_SOURCES :=\n";
	block += "C++_SOURCES :=\n";
	block += "MAIN_OBJECTS :=\n";
	block += "OBJECTS :=\n";
	block += "DEPENDS :=\n";
	block += "RM := rm -rf\n";
	
	// FLAGS
	block += "FLAGS := ";
	if (developMode == "Debug")
		block += " -g";
	block += " " + cprojectObject->warnings();
	block += " $(STD) $(PLATFROM)";
	if (!cprojectObject->otherFlag().isEmpty())
		block += " " + cprojectObject->otherFlag();
	block += "\n";

	// Includes
	block += "INCLUDES :=" + includes() + "\n";

	// Libraries
	block += "LIBS :=" + libraries() + "\n";

	// include subdir.mk
	if (!subdirs.isEmpty()) {
		QStringList::const_iterator constIterator = subdirs.constBegin();
		QStringList::const_iterator endIterator = subdirs.constEnd();
		while (constIterator != endIterator) {
			if ((*constIterator).isEmpty())
				block += "-include ./subdir.mk\n"; 
			else
				block += "-include ./" + *constIterator + "/subdir.mk\n";
			++constIterator;
		}
		block += "\n";
	}

	// TARGET
	block += "TARGETS := ";

	QStringList executeFiles = cprojectObject->executeFiles();
	block += executeFiles.join(" ") + "\n\n";

	// 这里加入了../project.small使得当用户没有修改程序但是修改了项目配置时, 也会重新编译, 以下的../project.small同理
	block += "all: $(TARGETS) ../project.small\n\n"; 
	QString compiler = QString();
	if (cprojectObject->type() == "C")
		compiler = "$(CC)";
	else
		compiler = "$(CCC)";

	QStringList::const_iterator constIterator = executeFiles.constBegin();
	QStringList::const_iterator endIterator = executeFiles.constEnd();
	while (constIterator != endIterator) {
		block += (*constIterator) + ": ";
		QStringList dependObjects = cprojectObject->executeFileDepend(*constIterator);
		QString mode = dependObjects.at(0);
		dependObjects.removeFirst();
		if (mode == "only") {
			block += dependObjects.at(0) + " ../project.small\n";
			block += "\t" + compiler + " " + dependObjects.at(0) + " $(FLAGS) $(LIBS) $(INCLUDES) -o $@\n";
		} else if (mode == "part") {
			block += dependObjects.join(" ") + " ../project.small\n";
			block += "\t" + compiler + " " + dependObjects.join(" ") + " $(FLAGS) $(LIBS) $(INCLUDES) -o $@\n";
		} else if (mode == "all" && dependObjects.count() > 0) {
			block += dependObjects.at(0) + " $(OBJECTS) ../project.small\n";
			block += "\t" + compiler + " " + dependObjects.at(0) + " $(FLAGS) $(LIBS) $(INCLUDES) -o $@\n";
		} else if (mode == "all" && dependObjects.count() <= 0) {
			block += " $(OBJECTS) ../project.small\n";
			block += "\t" + compiler + " $(OBJECTS) $(FLAGS) $(LIBS) $(INCLUDES) -o $@\n";
		}
		++constIterator;
	}
	block += "\n\n";
	
	// clean
	block += "clean:\n";
	block += "\t-$(RM) $(MAIN_OBJECTS) $(OBJECTS) $(DEPENDS) $(TARGETS)\n";
	block += "\t-@echo ' '\n\n";

	// .PHONY
	block += ".PHONY: clean\n\n";
	
	QFile file(rootPath + "/" + developMode + "/Makefile");
	if (!file.open(QFile::WriteOnly | QFile::Text)) {
		buildErrorStr += "无法写入\"" + rootPath + "/" + developMode + "/Makefile" + "\"\n";
		buildErrorStr += file.errorString() + "\n";
		return false;
	}
	file.write(block);
	file.close();
	return true;
} // buildMakefile

void CBuilder::addVariable(const QString &var, const QStringList &list, QByteArray &block)
{
	QStringList::const_iterator constIterator;
	QStringList::const_iterator endIterator;

	if (!list.isEmpty()) {
		block += var;
		constIterator = list.constBegin();
		endIterator = list.constEnd();
		while (constIterator != endIterator) {
			block += " \\\n    " + *constIterator;
			++constIterator;
		}
		block += "\n\n"; 				// 空一行
	}
} // addVariable

QString CBuilder::includes()
{
	QString includeStr;
	QStringList list = cprojectObject->includePath();
	QStringList::const_iterator constIterator = list.constBegin();
	QStringList::const_iterator endIterator = list.constEnd();

	while (constIterator != endIterator) {
		QString item = *constIterator;
		includeStr += "\\\n    " + *constIterator + "\n";
		++constIterator;
	}

	list = cprojectObject->includeFile();
	constIterator = list.constBegin();
	endIterator = list.constEnd();
	while (constIterator != endIterator) {
		QString item = *constIterator;
		includeStr += "\\\n    " + *constIterator + "\n";
		++constIterator;
	}

	return includeStr;
}

QString CBuilder::libraries()
{
	QString libraryStr;
	QStringList list = cprojectObject->libraryPath();
	QStringList::const_iterator constIterator = list.constBegin();
	QStringList::const_iterator endIterator = list.constEnd();

	while (constIterator != endIterator) {
		libraryStr += "\\\n    " + *constIterator + "\n";
		++constIterator;
	}

	list = cprojectObject->libraryFile();
	constIterator = list.constBegin();
	endIterator = list.constEnd();
	while (constIterator != endIterator) {
		libraryStr += "\\\n    " + *constIterator + "\n";
		++constIterator;
	}

	return libraryStr;
}
