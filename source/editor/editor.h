#ifndef EDITOR_H
#define EDITOR_H

#include <QtGui/QWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QGridLayout>
#include <QtCore/QString>

#include "codeeditor.h"

class Editor: public QWidget
{
	Q_OBJECT;
	
public:
	Editor(QWidget *parent = 0);
	Editor(const QString &filePath, QWidget *parent = 0);
	Editor(const QStringList &filePathList, QWidget *parent = 0);

public slots:
	void fileNew();  // 新建文件, 有 QFileDialog, 以 ~/ 为起始目录
	void fileNew(const QString &filePath);          // 新建 filePath 文件
	void fileNewWithPath(const QString &path);		// 以 path 为起始目录新建文件
	void fileOpen();		                        // 有 QFileDialog, 以 ~/ 为起始目录
	void fileOpen(const QString &filePath, bool reload = false);         // 打开 filePath 文件 
	void fileOpen(const QStringList &filePathList, bool reload = false); // 打开多个文件
	void fileOpenWithPath(const QString &path);		// 以 path 为起始目录打开文件
	bool fileSave();		   // 保存当前页文件 
	bool fileSave(int index);  // 保存 index 页文件，index最小为 0
	bool fileSaveAs();		   // 另存为，以当前文件位置为起始位置
	bool fileSaveAll();		   // 询问并保存所有修改过的文件 
	void fileClose();		   // 关闭当前页
	void fileClose(int index); // 关闭 index 页

	void format();			   // 格式化当前页
	void setMode();			   // 设置文本格式
	void setReadOnly();		   // 设置当前页缓存区为只读
	void modificationChanged(bool b, CodeEditor *codeEditor);
	void renameTab(const QString &oldFilePath, CodeEditor *codeEditor);

	void filePathChanged(const QString &oldFilePath, const QString &newFilePath); // 当目录已重命名, 则修该目录下的文件信息
	void pathChanged(const QString &oldPath, const QString &newPath);
	void fileDeleted(const QString &filePath); // 当文件已删除, 则关闭该文件对应的文件编辑页
	void directoryDeleted(const QString &dirPath); // 当目录已删除, 则关闭该目录下的文件编辑页
	void closePathFile(const QString &path);	   // 关闭path目录下所有文件

	bool quit();			   // 退出 Editor 前的处理

signals:
	void fileOpened(const QString &filePath);
	void fileCreated(const QString &filePath);
	
protected:
	bool fileSave(CodeEditor *codeEditor); // 保存 codeEditor 的文件
	bool fileSaveAs(CodeEditor *codeEditor); // 另存为 codeEditor 的文件
	bool maybeSave(CodeEditor *codeEditor);	 // 检查并保存 codeEditor 的文件
	bool maybeSaveAll();
	void setUi();							 // 设置界面
	void setConnect(CodeEditor *codeEditor);
	QString getIconFilePath(const QString &filePath);

	QTabWidget *tabWidget;
	QHBoxLayout *hbox;
	QMap<QString, CodeEditor*> openedFileMap; // 已打开文件
	QStringList closedFileMap;                // 关闭了的文件列表,用于"最近访问"
};

#endif
