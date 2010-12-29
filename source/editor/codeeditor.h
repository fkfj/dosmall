#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QtCore>
// #include <QtGui>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QTextEdit>

#include "plaineditor.h"

class CodeEditor : public QWidget
{
    Q_OBJECT;
    
public:
    CodeEditor(bool readOnly = false, QWidget *parent = 0);
    CodeEditor(const QString &filePath, bool readOnly = false, QWidget *parent = 0); 

	QString getFileName();		// 文件名
    QString getPath();			// 文件路径
    QString getFilePath();      // 文件路径/文件名
	void setFileName(const QString &fileName);
	void setPath(const QString &path);
    void setFilePath(const QString &filePath); // 重设 "文件名/文件名"
    PlainEditor* getPlainEditor();				   
	bool isModified();			// 是否修改了缓存区
					 
public slots:
    void modificationChanged(bool b);	// 接受textEdit 的changed()信号，并发送信号changedSignal()。
	void fileNew();
	void fileOpen();
	void reload();
	bool fileSave();			// 保存缓存区到文件。
	bool fileSaveAll();
	bool fileSaveAs();			// 另存为缓存区。
	void format();
       
signals:
    void modificationChangedSignal(bool b, CodeEditor *codeEditor); // 缓存区修改了的信号。
	// void fileNewSignal();                             // 以当前文件的path为起始路径。
	void fileNewSignal(const QString &filePath);      // 指定绝对路径和文件名。可用于“创建当前文件的.c、.h、test文件”等
	void fileNewWithPath(const QString &path);
	void fileNewWithPathSignal(const QString &path);  // 以指定的路径作为起始路径。可以创建与当前项目有关的文件，如在test文件夹创建测试文件
	void fileOpenSignal();                            // 以当前文件的path为起始路径。
	void fileOpenSignal(const QString &filePath);	    // 指定绝对路径和文件名。
	void fileOpenSignal(const QStringList &filePathList); // 可用于“打开当前文件的所有头文件”等 
	void fileOpenWithPathSignal(const QString &path); // 以指定的路径作为起始路径。暂时没想到
	void renamedSignal(const QString &oldFilePath, CodeEditor *codeEditor);
	bool fileSaveAllSignal();                 		// 保存所有打开且修改过的文件。
    
protected:
	void setUi();					// 设置界面
	void setConnect();
	
	// base
    QString path;
    QString fileName;
    PlainEditor *plainEditor;
	bool readOnly;

	// Action
	QAction *action_New;
	QAction *action_Open;
	QAction *action_Save;
	QAction *action_Save_All;
	QAction *action_Save_As;
	QAction *action_Undo;
	QAction *action_Redo;
	QAction *action_Cut;
	QAction *action_Copy;
	QAction *action_Paste;
	QAction *action_Format;

	// ToolBar
	QToolBar *toolBar;

	// layout
    QFrame *frame;
    QHBoxLayout *hbox;
    QVBoxLayout *vbox;
};

#endif
