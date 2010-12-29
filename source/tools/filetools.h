#ifndef FILETOOLS_H
#define FILETOOLS_H

class FileTools
{
public:
	enum Error { NoError, ExistsButNotDir, ExistsButNotFile, NoPermission, Unknown};
	FileTools();
	bool mkdir(const QString &filePath);
	bool mkfile(const QString &filePath);
	QString errorString();
	void clearError();
private:
	QString errorStr;
	int error;
};

#endif
