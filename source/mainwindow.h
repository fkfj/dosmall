#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCloseEvent>
#include <QtGui/QDockWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QSplitter>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

#include "editor/editor.h"
#include "build/builderwidget.h"
#include "build/buildoutput.h"
#include "project/projecttreeview.h"
#include "project/cproject.h"
#include "terminal/terminalview.h"

class MainWindow: public QMainWindow
{
	Q_OBJECT;
	
public:
	MainWindow();

protected:
	virtual void closeEvent(QCloseEvent *e);

private slots:
	// file
	void newSource(const QString &path = QString(), const QString &projectName = QString());
	void newHeader(const QString &path = QString(), const QString &projectName = QString());
	void newTextFile(const QString &path = QString(), const QString &projectName = QString());
	void newFile(const QString &title, const QString &path, const QString &filter);
	void newProject();
	void openFile(const QString &filePath = QString());
	void openFile(const QString &filePath, ProjectTreeView::FileType fileType,
				  const QString &projectName = QString());
	void openProject();

	// about build
	void build();
	void build(const QString &target, const QString &projectName);
	void buildCurrentProject(const QString &target);
	void run();
	void run(const QString &executeFile, const QString &args, const QString &projectName);
	void runCurrentProject(const QString &executeFile);
	void deleteBuilderWidget(BuilderWidget *builderWidget);

	// about debug
	void debug();
	
	// about project
	void closeProjectFile(const QString &rootPath);
	void configProject();
	void configProject(const QString &projectName);
	void currentProjectChanged(const QString &projectName);
	void projectUpdated(const QString &projectName);
	void projectOpened(const QString &projectName);
	void projectClosed(const QString &projectName);
	void projectDeleted(const QString &projectName);

	// about window
	void toggleBuildOutputView();
	void toggleEditorView();
	void toggleProjectView();
	void toggleTerminalView();

	void help();

	void quit();

private:
	void setUi();
	void retranslateUi();

	void createMenuBar();
	void createFileMenu();
	void createBuildMenu();
	void createDebugMenu();
	void createToolsMenu();
	void createWindowMenu();
	void createHelpMenu();

	void createToolBar();
	void createFileToolBar();
	void createEditToolBar();
	void createBuildToolBar();
	void createDebugToolBar();
	void createWindowToolBar();
	void createStatusBar();

	BuilderWidget * getBuilderWidget(const QString &projectName);
	Editor *editor;
	ProjectTreeView *projectTreeView;
	BuildOutput *buildOutput;
	TerminalView *terminalView;
	QSplitter *mainSplitter;
	QSplitter *midSplitter;
	QWidget *widgetBox;
	
	ProjectManager *projectManager;
	QMap<QString, BuilderWidget *> builderWidgetMap; // QMap<projectName, builderWidget>
	
	// File
	QAction *action_New_Source;
	QAction *action_New_Header;
	QAction *action_New_TextFile;
	QAction *action_New_Project;

    QAction *action_Open_File;
	QAction *action_Open_Project;

    QAction *action_Save;
	QAction *action_Save_All;
    QAction *action_Save_As;

	QAction *action_Close;
    QAction *action_Quit;

	// Build
	Menu *menu_CurrentProject;
	Menu *menu_BuildTarget;
	Menu *menu_RunTarget;
	QAction *action_ConfigProject;

	// Debug
	QAction *action_Debug;

	// Window
	QAction *action_EditorView;
	QAction *action_ProjectView;
	QAction *action_BuildOutputView;
	QAction *action_TerminalView;

	// Help
	QAction *action_Help;
	
	// menubar
    QMenuBar *menubar;
    QMenu *menu_File;
	QMenu *menu_Build;
	QMenu *menu_Debug;
	QMenu *menu_Tools;
	QMenu *menu_Window;
	QMenu *menu_Help;

	// file toolbar
    QToolBar *file_ToolBar;

	// build toolbar
	QToolBar *build_ToolBar;
	QComboBox *currentProjectCombo;
	QComboBox *buildCombo;
	QComboBox *runCombo;
	QLineEdit *argsLineEdit;
	QAction *action_Build;
	QAction *action_Run;

	// window toolbar
	QToolBar *window_ToolBar;

	// statusbar
	QStatusBar *statusbar;
};

#endif
