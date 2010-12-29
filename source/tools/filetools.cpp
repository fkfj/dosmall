#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include "filetools.h"

FileTools::FileTools()
{
	error = FileTools::NoError;
}

bool FileTools::mkdir(const QString &filePath)
{
	QString path = filePath;
	QString currentPath = "/";
	QStringList list = path.split("/", QString::SkipEmptyParts);

	QFileInfo fileInfo(filePath);
	if (fileInfo.exists()) {
		if (fileInfo.isDir()) {
			return true;
		} else {
			errorStr = fileInfo.absoluteFilePath();
			error = FileTools::ExistsButNotDir;
			return false;
		}
	}
	
	QStringList::const_iterator constIterator = list.constBegin();
	QStringList::const_iterator endIterator = list.constEnd();
	while (constIterator != endIterator) {
		QDir dir(currentPath);
		QFileInfo parentDir(currentPath);
		QString childPath = currentPath + *constIterator;
		QFileInfo childDir(childPath);
		currentPath = childPath + "/";
		if (childDir.exists()) {
			if (!childDir.isDir()) {
				errorStr = childDir.absoluteFilePath();
				error = FileTools::ExistsButNotDir;
				return false;
			}
		} else if (!parentDir.isWritable()) {
			errorStr = parentDir.absoluteFilePath();
			error = FileTools::NoPermission;
			return false;
		} else if (!dir.mkdir(*constIterator)) {
			errorStr = "failed to create directory: " + childDir.absoluteFilePath();
			error = FileTools::Unknown;
			return false;
		}
		++constIterator;
	}
	return true;
}

bool FileTools::mkfile(const QString &filePath)
{
	QFileInfo fileInfo(filePath);
	QFileInfo parentDir(fileInfo.absolutePath());
	if (fileInfo.exists()) {
		if (!fileInfo.isFile()) {
			errorStr = fileInfo.absoluteFilePath();
			error = FileTools::ExistsButNotFile;
			return false;
		} else
			return true;
	} else if (!mkdir(fileInfo.absolutePath())) {
		return false;
	} else if (!parentDir.isWritable()){
		errorStr = parentDir.absolutePath();
		error = FileTools::NoPermission;
		return false;
	} else {
		QFile file(filePath);
		if (!file.open(QFile::WriteOnly | QFile::Text)) {
			errorStr = file.errorString() + ": failed to create file: " + filePath;
			error = FileTools::Unknown;
			return false;
		}
		file.write(QByteArray(""));
		file.close();
	}
	return true;
}

QString FileTools::errorString()
{
	switch (error) {
	case FileTools::ExistsButNotDir:
		return QString("Error: %1 exists,but it's not directory.").arg(errorStr);
	case FileTools::ExistsButNotFile:
		return QString("Error: %1 exists,but it's not file.").arg(errorStr);
	case FileTools::NoPermission:
		return QString("Error: no permission: %1").arg(errorStr);
	case FileTools::Unknown:
		return QString("Error: %1").arg(errorStr);
	default:
		return QString("");
	}
}

void FileTools::clearError()
{
	error = FileTools::NoError;
	errorStr.clear();
}
