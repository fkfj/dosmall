#ifndef PROJECTTREEVIEW_H
#define PROJECTTREEVIEW_H

#include <QtCore/QFileInfo>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTreeView>

#include "../global.h"
#include "../menu.h"

class ProjectTreeView : public QTreeView
{
	Q_OBJECT;
	
public:
	enum FileType { OpenedProject, ClosedProject, Directory, Source, Header, Makefile,
					Shell, Sharedlib, Archive, Object, Executable, Text, Unknown };
	ProjectTreeView(QWidget *parent = 0);

public slots:
	void loadProject(const QString &projectName);
	void loadFile(const QString &filePath);
	void loadDirectory(const QString &path);
	void loadFileOrDirectory(const QString &filePath); // 在适当的位置添加文件节点,
	// 如果该文件的父目录或者更上层的目录已经在treeView中, 则添加到适合的位置,
	// 否则, 新开一个顶级节点, 将该文件添加到这个节点.
	void closeProjectItem(const QString &projectName);
	void renameProjectItem(const QString &oldName, const QString &newName);
	void deleteProjectItem(const QString &projectName);

private slots:
	void doubleClicked(const QModelIndex &index);

	// for Action
	// about opened project
	void closeProject();		// 关闭当前项目
	void deleteProject();		// 删除删除当前项目
	void configProject();		// 设置当前项目
	void renameProject();		// 重命名当前项目

	void openClosedProject();

	void build(const QString &target); // 对当前项目执行 make target
	void run(const QString &execFile); // 对当前项目执行 execFile 程序
	void debug();				// 调试当前项目
	void setCurrentProject();	    // 设置当前项目为主项目

	// about code
	void openFile();			// 打开当前文件
	void newSourceFile();		// 创建源文件,在当前目录下
	void newHeaderFile();		// 创建头文件,在当前目录下
	void newTextFile();		    // 创建空文本文件,在当前目录下
	void newDirectory();		// 创建目录,在当前目录下
	void renameFile();			// 重命名当前文件
	void renameDirectory();		// 重命名当前目录
	void findFile();			// 查找文件
	void findCode();			// 查找源代码
	void deleteFile();			// 删除文件
	void deleteDirectory();		// 删除文件夹

	//
	void refresh();				// 刷新目录文件树
	void loadFile();			// 导入文件到 treeView 中
	void loadDirectory();		// 导入文件夹到 treeView 中

signals:
	// about code
	void directoryDeleted(const QString filePath, const QString &projectName);
	void fileRenamed(const QString &oldName, const QString &newName, const QString &projectName);
	void fileDeleted(const QString &filePath, const QString &projectName);
	void newSourceWithPath(const QString &path, const QString &projectName);
	void newHeaderWithPath(const QString &path, const QString &projectName);
	void newTextFileWithPath(const QString &path, const QString &projectName);
	void openFile(const QString &filePath, ProjectTreeView::FileType fileType, const QString &projectName);
	void openProject();
	void pathChanged(const QString &oldPath, const QString &newPath, const QString &projectName);
	
	// about project
	void build(const QString &target, const QString &projectName);
	void configProject(const QString &projectName);
	void run(const QString &executeFile, const QString &args, const QString &projectName);
	
protected:
	virtual void mousePressEvent(QMouseEvent *event);
	// 设置treeView 界面和 connect
	void setUi();				           
	// 导入历史 project, 即historyProject文件中记录的project
	void loadOpenedProject();
	void loadClosedProject();
	void loadHistoryFile();
	QString typeText(const QModelIndex &index);
	QString typeText(const QStandardItem *item);
	QString parentTypeText(const QModelIndex &index);
	QString parentTypeText(const QStandardItem *item);
	QString pathText(const QModelIndex &index);
	QString parentPathText(const QModelIndex &index);
	QString projectNameText(const QModelIndex &index);
	QStandardItem *headItem(const QModelIndex &index);
	QStandardItem *pathItem(const QModelIndex &index);
	QStandardItem *parentPathItem(const QModelIndex &index);
	QStandardItem *typeItem(const QModelIndex &index);
	QStandardItem *parentTypeItem(const QModelIndex &index);
	QStandardItem *projectNameItem(const QModelIndex &index);
	// 递归的将parent的子节点的文件的路径中的oldPath改为newPath
	void changeChildrenPath(const QStandardItem *parent, const QString &oldPath,
							const QString &newPath); 
	// 递归将parent的字节点的项目名称改为newName
	void changeChildrenProjectName(const QStandardItem *parent, const QString &newName);
	// 为 model创建 openedProject节点
	QList<QStandardItem *> buildOpenedProjectItem(const QString projectName);
	// 为 model闯将 closedProject节点
	QList<QStandardItem *> buildClosedProjectItem(const QString &projectName);
	// 为 model 创建 directroy 节点
	QList<QStandardItem *> buildDirectoryItem(const QString &path, const QString &projectName = QString());
	// 为 model 创建 file 节点
	QList<QStandardItem *> buildFileItem(const QString &filePath, const QString &projectName = QString()); 
	// 根据文件名返回对应的Icon, 并设置 mimeType
	QIcon mimeTypeIcon(const QFileInfo &fileInfo, QString &mimeType); 
	void sortChildrenItem(QStandardItem *parent);
	void refreshDirectory(const QModelIndex &index);
	void refreshProject(const QModelIndex &index);
	
	ProjectManager *projectManager;
	QStandardItemModel *model;	// treeView 的模型
	QMap<QString, QStandardItem*> openedProjectItemMap; // <projectName, headItem>
	QMap<QString, QStandardItem*> closedProjectItemMap; // <projectName, headItem>
	QMap<QString, QStandardItem*> topFilePathItemMap;	// <filePath, headItem>
	QStringList mimeTypeList;

	Menu *mouseMenu;
	Menu *buildMenu;
	Menu *runMenu;
	
	QList<QAction *> openedProjectActionList;
	QList<QAction *> closedProjectActionList;
	QList<QAction *> directoryActionList;
	QList<QAction *> textFileActionList;
	QList<QAction *> otherFileActionList;
	
	// Action for opened project
	QAction *action_OpenProject;
	QAction *action_CloseProject;
	QAction *action_DeleteProject;
	QAction *action_ConfigProject;
	QAction *action_RenameProject;
	QAction *action_Debug;
	QAction *action_SetAsCurrentProject;

	// action for closed project
	QAction *action_OpenClosedProject;

	// Action for code
	QAction *action_OpenFile;
	QAction *action_NewSource;
	QAction *action_NewHeader;
	QAction *action_NewTextFile;
	QAction *action_NewDirectory;
	QAction *action_RenameFile;
	QAction *action_RenameDirectory;
	QAction *action_FindFile;
	QAction *action_FindCode;
	QAction *action_DeleteFile;
	QAction *action_DeleteDirectory;

	// Action for other
	QAction *action_Refresh;
	QAction *action_LoadFile;
	QAction *action_LoadDirectory;
};

#endif
