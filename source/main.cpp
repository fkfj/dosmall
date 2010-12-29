#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include "mainwindow.h"

bool init();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
	if (init()) {
		MainWindow *window = new MainWindow();
		window->show();
	}
    return app.exec();
} // main

bool init()
{
	// 检查configPath目录
	QString configPath = QDir::homePath() + "/.dosmall";
	QFileInfo fileInfo(configPath);
	QDir dir(QDir::homePath());
	bool success = false;
	QString errorString = QString();
	if (!fileInfo.exists()) {
		if (dir.mkdir(".dosmall")) {
			success = true;
		} else {
			success = false;
			errorString = "无法创建\"" + configPath + "\"目录.程序无法正常使用.";
		}
	} else if (!fileInfo.isDir()) {
		if (!dir.remove(".dosmall")) {
			success = false;
			errorString = "文件\"" + configPath + "\"已经存在, 但不是目录.程序无法正常使用.";
		} else if (!dir.mkdir(".dosmall")) {
			success = false;
			errorString = "无法创建\""+ configPath + "\"目录.程序无法正常使用.";
		}
	} else {
		success = true;
	}
	
	if (!success) {
		qDebug() << "1";
		qDebug() << errorString;
		QMessageBox::warning(0, "程序启动失败", errorString, QMessageBox::Ok);
		return false;
	}

	// 检查configPath + "/conf"文件
	bool create = false;
	success = false;
	fileInfo.setFile(configPath + "/conf");
	if (!fileInfo.exists()) {
		create = true;
	} else if (!fileInfo.isFile()) {
		dir.setPath(configPath);
		if (!dir.remove("conf")) {
			errorString = "文件\"" + fileInfo.absoluteFilePath() + "\"存在, 但不是普通可读写文件文件.";
			success = false;
			qDebug() << errorString;
			qDebug() << "1";
			QMessageBox::warning(0, "程序启动失败", errorString, QMessageBox::Ok);
			return false;
		} else {
			create = true;
		}
	} else if (!fileInfo.isReadable() || !fileInfo.isWritable()){
		errorString = "文件\"" + fileInfo.absoluteFilePath() + "\"存在, 但您没有读写权限.";
		success = false;
		qDebug() << "12";
	qDebug() << errorString;
		QMessageBox::warning(0, "程序启动失败", errorString, QMessageBox::Ok);
		return false;
	}

	if (create) {
		QFile file(configPath + "/conf");
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			errorString = "无法创建\"" + fileInfo.absoluteFilePath() + "\"文件.";
			qDebug() << "13";
		qDebug() << errorString;
			QMessageBox::warning(0, "程序启动失败", errorString, QMessageBox::Ok);
			return false;
		} else {
			file.write(QByteArray());
			file.close();
		}
	}

	QSettings *confSettings = new QSettings(configPath + "/conf", QSettings::IniFormat);
	QString projectHomePath = confSettings->value("ProjectHomePath", QString()).toString();
	while (projectHomePath.isEmpty()) {
		projectHomePath = QFileDialog::getExistingDirectory(0, QString("选择目录作为工作空间"),
															QDir::homePath(),
															QFileDialog::ShowDirsOnly
															| QFileDialog::DontResolveSymlinks);
	}
	confSettings->setValue("ProjectHomePath", projectHomePath);
	confSettings->sync();

	// 检查configPath + "/historyproject"文件
	fileInfo.setFile(configPath + "/historyproject");
	if (!fileInfo.exists()) {
		QFile file(configPath + "/historyproject");
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			errorString = "无法创建\"" + fileInfo.absoluteFilePath() + "\"文件.";
			qDebug() << "14";
		qDebug() << errorString;
			QMessageBox::warning(0, "程序启动失败", errorString, QMessageBox::Ok);
			return false;
		} else {
			file.write(QByteArray());
			file.close();
		}
	}

	// 检查cproject.template
	create = false;
	fileInfo.setFile(configPath + "/cproject.template");
	if (!fileInfo.exists()) {
		create = true;
	} else if (!fileInfo.isFile()) {
		dir.setPath(configPath);
		if (!dir.remove("cproject.template")) {
			errorString = "文件\"" + fileInfo.absoluteFilePath() + "\"存在, 但不是普通可读写文件文件.";
			success = false;
			qDebug() << "1";
			qDebug() << errorString;
			QMessageBox::warning(0, "程序启动失败", errorString, QMessageBox::Ok);
			return false;
		} else {
			create = true;
		}
	} else if (!fileInfo.isReadable() || !fileInfo.isWritable()){
		errorString = "文件\"" + fileInfo.absoluteFilePath() + "\"存在, 但您没有读写权限.";
		success = false;
		qDebug() << "15";
		qDebug() << errorString;
		QMessageBox::warning(0, "程序启动失败", errorString, QMessageBox::Ok);
		return false;
	}
	
	if (create) {
		QByteArray block;
		block += "Name = NoName\n";
		block += "Type = C\n";
		block += "DevelopMode = Debug\n";
		block += "Platform = \n";
		block += "Std = \n";
		block += "CC = gcc\n";
		block += "CCC = g++\n";
		block += "Make = make\n";
		block += "IncludePath = \n";
		block += "IncludeFile = \n";
		block += "LibraryPath = \n";
		block += "LibraryFile = \n";
		block += "MainSources = \n";
		block += "MainSourceDepend = \n";
		block += "OtherFlag = \n";

		QFile file(fileInfo.absoluteFilePath());
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			errorString = "无法创建\"" + fileInfo.absoluteFilePath() + "\"文件.";
			qDebug() << "16";
			qDebug() << errorString;
			QMessageBox::warning(0, "程序启动失败", errorString, QMessageBox::Ok);
			return false;
		} else {
			file.write(block);
			file.close();
		}
	}
	return true;
}
