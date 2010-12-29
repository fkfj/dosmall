#ifndef CBUILDTHREAD_H
#define CBUILDTHREAD_H

#include <QtCore/QByteArray>
#include <QtCore/QSettings>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QProcess>

#include "../project/cproject.h"

class CBuilder : public QThread
{
	Q_OBJECT;
	
public:
	CBuilder(CProject *cproject, QObject *parent = 0);
	~CBuilder();
	void run();
	bool isBuilding();
							  
public slots:
	void build();
	void build(const QString &target);
	void buildAll();
	void breakBuild();
	void rebuild();

private slots:
	void make();
	void makeError(QProcess::ProcessError error);
	void makeFinished(int exitCode, QProcess::ExitStatus exitStatus);
	
signals:
	void buildError(const QString &error);
	void buildFinished(const QString &text);
	void buildBreaked(const QString &text);
	void promptText(const QString &text);
	
protected:
	void setTargets(const QString &target);
	void setScan(bool reScan);
	void start();				// 将start设为只有自己及子类可见, 因为run只完成了build的部分工作
	void scanSources(const QString &dirPath);  // 扫表源文件, 并调用buildSubdirMk
	bool buildSubdirMk(const QString &dirPath);	// 根据扫描的源文件创建subdir.mk文件
	bool buildMakefile();		// 根据配置文件创建Makefile文件
	void addVariable(const QString &var, const QStringList &list,
					 QByteArray &block); // 添加变量 var = list 到block中
	QString includes();
	QString libraries();


	CProject *cprojectObject;
	QString rootPath;
	QString developMode;
	QStringList absoluteMainSources;
	QStringList subdirs;		// 含源文件的目录列表
	QString makefile;	        // makefile文件的路径, 如果是相对路径, 则以rootPath, 为起始路径,
	                            // 用户自定义模式下才会用到这个变量成员
	QString workingDir;
	bool scan;					// 是否重新扫描源文件
	bool valid;				// 可以执行makeProcess的标志
	bool userDefinedMode;		// 用户自定义开发模式模式,
	                            // 不扫描源文件, 使用用户指定的Makefile文件,
	                            // 在Makefile文件的目录下执行makeProcess
	QString buildErrorStr;		// 保存除了执行makeProcess之外的所有报错信息,
	                            // 由信号buildError() 发送出去
	QProcess *makeProcess;		// 执行make的qprocess
	QStringList targetList;		// 执行makeProcess时的参数, 也就是系统命令 make 的的参数

	QStringList mainSources;	// 含main() 的源文件
	QStringList cSources;		// .c 源文件列表
	QStringList cppSources;		// .cpp 源文件列表
	QStringList ccSources;		// .cc 源文件列表
	QStringList cxxSources;		// .cxx 源文件列表
	QStringList cpppSources;	// .c++ 源文件
};

#endif
