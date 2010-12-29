#include <QtGui/QFileDialog>
#include <QtGui/QKeySequence>
#include <QtGui/QMessageBox>
#include <QtGui/QToolButton>
#include <QtGui/QSizePolicy>
#include "mainwindow.h"
#include "global.h"
#include "project/newprojectwizard.h"
#include "project/cprojectconfigdialog.h"

// public
MainWindow::MainWindow()
{
	setUi();
	retranslateUi();

	// connect editor
	connect(editor, SIGNAL(fileCreated(const QString &)), projectTreeView, SLOT(loadFile(const QString &)));
	connect(editor, SIGNAL(fileOpened(const QString &)), projectTreeView, SLOT(loadFile(const QString &)));

	// connect projectManager
	connect(projectManager, SIGNAL(currentProjectChanged(const QString &)),
			this, SLOT(currentProjectChanged(const QString &)));
	connect(projectManager, SIGNAL(projectOpened(const QString &)),
			this, SLOT(projectOpened(const QString &)));
	connect(projectManager, SIGNAL(projectClosed(const QString &)),
			this, SLOT(projectClosed(const QString &)));
	connect(projectManager, SIGNAL(projectUpdated(const QString &)),
			this, SLOT(projectUpdated(const QString &)));

	// connect projectTreeView
	connect(projectTreeView, SIGNAL(openFile(const QString &, ProjectTreeView::FileType, const QString &)),
			this, SLOT(openFile(const QString &, ProjectTreeView::FileType, const QString &)));
	connect(projectTreeView, SIGNAL(newSourceWithPath(const QString &, const QString &)),
			this, SLOT(newSource(const QString &, const QString &)));
	connect(projectTreeView, SIGNAL(newHeaderWithPath(const QString &, const QString &)),
			this, SLOT(newHeader(const QString &, const QString &)));
	connect(projectTreeView, SIGNAL(newTextFileWithPath(const QString &, const QString &)),
			this, SLOT(newTextFile(const QString &, const QString &)));

	connect(projectTreeView, SIGNAL(fileRenamed(const QString &, const QString &, const QString &)),
			editor, SLOT(filePathChanged(const QString &, const QString &)));
	connect(projectTreeView, SIGNAL(pathChanged(const QString &, const QString &, const QString &)),
			editor, SLOT(pathChanged(const QString &, const QString &)));
	connect(projectTreeView, SIGNAL(fileDeleted(const QString &, const QString &)),
			editor, SLOT(fileDeleted(const QString &)));
	connect(projectTreeView, SIGNAL(directoryDeleted(const QString &, const QString &)),
			editor, SLOT(directoryDeleted(const QString &)));

	connect(projectTreeView, SIGNAL(build(const QString &, const QString &)),
			this, SLOT(build(const QString &, const QString &)));
	connect(projectTreeView, SIGNAL(run(const QString &, const QString &, const QString &)),
			this, SLOT(run(const QString&, const QString &, const QString &)));
	connect(projectTreeView, SIGNAL(configProject(const QString &)),
			this, SLOT(configProject(const QString &)));

	// connect menu_File
	connect(action_New_Source, SIGNAL(triggered()), this, SLOT(newSource()));
	connect(action_New_Header, SIGNAL(triggered()), this, SLOT(newHeader()));
	connect(action_New_TextFile, SIGNAL(triggered()), this, SLOT(newTextFile()));
	connect(action_New_Project, SIGNAL(triggered()), this, SLOT(newProject()));

	connect(action_Open_File, SIGNAL(triggered()), this, SLOT(openFile()));
	connect(action_Open_Project, SIGNAL(triggered()), this, SLOT(openProject()));

	connect(action_Save, SIGNAL(triggered()), editor, SLOT(fileSave()));
	connect(action_Save_All, SIGNAL(triggered()), editor, SLOT(fileSaveAll()));
	connect(action_Save_As, SIGNAL(triggered()), editor, SLOT(fileSaveAs()));

	connect(action_Close, SIGNAL(triggered()), editor, SLOT(fileClose()));
	connect(action_Quit, SIGNAL(triggered()), this, SLOT(quit()));

	// connect menu_Build
	connect(menu_CurrentProject, SIGNAL(text(const QString &)),
			projectManager, SLOT(setCurrentProject(const QString &)));
	connect(menu_BuildTarget, SIGNAL(text(const QString &)),
			this, SLOT(buildCurrentProject(const QString &)));
	connect(menu_RunTarget, SIGNAL(text(const QString &)),
			this, SLOT(runCurrentProject(const QString &)));
	connect(action_ConfigProject, SIGNAL(triggered()), this, SLOT(configProject()));

	// connect menu_Debug
	connect(action_Debug, SIGNAL(triggered()), this, SLOT(debug()));

	// connect menu_Window
	connect(action_BuildOutputView, SIGNAL(triggered()), this, SLOT(toggleBuildOutputView()));
	connect(action_EditorView, SIGNAL(triggered()), this, SLOT(toggleEditorView()));
	connect(action_ProjectView, SIGNAL(triggered()), this, SLOT(toggleProjectView()));
	connect(action_TerminalView, SIGNAL(triggered()), this, SLOT(toggleTerminalView()));
	
	// connect build_ToolBar
	connect(currentProjectCombo, SIGNAL(currentIndexChanged(const QString &)),
			projectManager, SLOT(setCurrentProject(const QString &)));
	connect(action_Build, SIGNAL(triggered()), this, SLOT(build()));
	connect(action_Run, SIGNAL(triggered()), this, SLOT(run()));

	// connect help
	connect(action_Help, SIGNAL(triggered()), this, SLOT(help()));

	// shortecut
	action_Save->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
	action_Build->setShortcut(QKeySequence("F9"));
	action_Run->setShortcut(QKeySequence("F5"));

	currentProjectChanged(projectManager->currentProjectName());
} // MainWindow

// 处理用户点击了buildToolBar中的"build"
void MainWindow::build()
{
	QString projectName = currentProjectCombo->currentText();
	QString target = buildCombo->currentText();
	
	if (projectName.isEmpty() || !projectManager->hasProject(projectName) || target.isEmpty())
		return;

	build(target, projectName);
}

void MainWindow::build(const QString &target, const QString &projectName)
{
	midSplitter->show();
	widgetBox->show();
	buildOutput->show();
	terminalView->hide();
	BuilderWidget *builderWidget = getBuilderWidget(projectName);
	if (builderWidget) {
		buildOutput->setCurrentWidget(builderWidget);
		builderWidget->build(target);
	}
}

void MainWindow::buildCurrentProject(const QString &target)
{
	build(target, projectManager->currentProjectName());
}

// protected
void MainWindow::closeEvent(QCloseEvent *e)
{
	if (editor->quit()){
		e->accept();
    } else {
		e->ignore();
    }
} // closeEvent

// about project
void MainWindow::closeProjectFile(const QString &rootPath)
{
	editor->closePathFile(rootPath);
}

void MainWindow::configProject()
{
	if (projectManager->hasProject(projectManager->currentProjectName()))
		configProject(projectManager->currentProjectName());
}

void MainWindow::configProject(const QString &projectName)
{
	if (!projectManager->hasProject(projectName)) {
		QMessageBox::warning(this, "没有该项目", projectName, QMessageBox::Ok);
		return;
	}
	
	CProject *cproject = projectManager->projectObject(projectName);
	if (cproject == 0) {
		QMessageBox::warning(this, "项目无效", "未知错误", QMessageBox::Ok);
		return;
	} else if(!cproject->isValid()) {
		QMessageBox::warning(this, "项目无效", cproject->errorInfo(), QMessageBox::Ok);
		return;
	} else {
		CProjectConfigDialog *configDialog = new CProjectConfigDialog(cproject);
		configDialog->exec();
	}
}


void MainWindow::createMenuBar()
{
	menubar = new QMenuBar(this);
	menubar->setGeometry(QRect(0, 0, 800, 26));

	createFileMenu();
	createBuildMenu();
	createDebugMenu();
	createToolsMenu();
	createWindowMenu();
	createHelpMenu();
	
	menubar->addAction(menu_File->menuAction());
	menubar->addAction(menu_Build->menuAction());
	menubar->addAction(menu_Debug->menuAction());
	menubar->addAction(menu_Tools->menuAction());
	menubar->addAction(menu_Window->menuAction());
	menubar->addAction(menu_Help->menuAction());

	setMenuBar(menubar);
}

void MainWindow::createFileMenu()
{
	action_New_Source = new QAction(this);
	QIcon icon_New_Source;
	icon_New_Source.addFile(QString::fromUtf8(":/images/c_cpp_source.png"),
							QSize(), QIcon::Normal, QIcon::On);
	action_New_Source->setIcon(icon_New_Source);

	action_New_Header = new QAction(this);
	QIcon icon_New_Header;
	icon_New_Header.addFile(QString::fromUtf8(":/images/header.png"),
							QSize(), QIcon::Normal, QIcon::On);
	action_New_Header->setIcon(icon_New_Header);

	action_New_TextFile = new QAction(this);
	QIcon icon_New_TextFile;
	icon_New_TextFile.addFile(QString::fromUtf8(":/images/text_file.png"),
							  QSize(), QIcon::Normal, QIcon::On);
	action_New_TextFile->setIcon(icon_New_TextFile);

	action_New_Project = new QAction(this);
	QIcon icon_New_Project;
	icon_New_Project.addFile(QString::fromUtf8(":/images/c_cpp_project.png"),
							 QSize(), QIcon::Normal, QIcon::On);
	action_New_Project->setIcon(icon_New_Project);

	action_Open_File = new QAction(this);
	QIcon icon_Open_File;
	icon_Open_File.addFile(QString::fromUtf8(":/images/file_open.png"),
						   QSize(), QIcon::Normal, QIcon::On);
	action_Open_File->setIcon(icon_Open_File);

	action_Open_Project = new QAction(this);
	QIcon icon_Open_Project;
	icon_Open_Project.addFile(QString::fromUtf8(":/images/c_cpp_project.png"),
							  QSize(), QIcon::Normal, QIcon::On);
	action_Open_Project->setIcon(icon_Open_Project);

	action_Save = new QAction(this);
	QIcon icon_Save;
	icon_Save.addFile(QString::fromUtf8(":/images/file_save.png"),
					  QSize(), QIcon::Normal, QIcon::On);
	action_Save->setIcon(icon_Save);

	action_Save_All = new QAction(this);
	QIcon icon_Save_All;
	icon_Save_All.addFile(QString::fromUtf8(":/images/file_save_all.png"),
					  QSize(), QIcon::Normal, QIcon::On);
	action_Save_All->setIcon(icon_Save_All);
	
	action_Save_As = new QAction(this);
	QIcon icon_Save_As;
	icon_Save_As.addFile(QString::fromUtf8(":/images/file_save_as.png"),
						  QSize(), QIcon::Normal, QIcon::On);
	action_Save_As->setIcon(icon_Save_As);
	
	action_Close = new QAction(this);
	QIcon icon_Close;
	icon_Close.addFile(QString::fromUtf8(":/images/close.png"),
					  QSize(), QIcon::Normal, QIcon::On);
	action_Close->setIcon(icon_Close);
	
	action_Quit = new QAction(this);
	QIcon icon_Quit;
	icon_Quit.addFile(QString::fromUtf8(":/images/exit.png"),
						  QSize(), QIcon::Normal, QIcon::On);
	action_Quit->setIcon(icon_Quit);
	
	menu_File = new QMenu(menubar);
	menu_File->addAction(action_New_Source);
	menu_File->addAction(action_New_Header);
	menu_File->addAction(action_New_TextFile);
	menu_File->addAction(action_New_Project);
	menu_File->addAction(action_Open_File);
	menu_File->addAction(action_Open_Project);
	menu_File->addAction(action_Save);
	menu_File->addAction(action_Save_All);
	menu_File->addAction(action_Save_As);
	menu_File->addAction(action_Close);
	menu_File->addAction(action_Quit);
}

void MainWindow::createBuildMenu()
{
	menu_Build = new QMenu(menubar);

	menu_CurrentProject = new Menu(true, menu_Build);

	menu_BuildTarget = new Menu(false, menu_Build);
	QIcon icon_BuildTarget;
	icon_BuildTarget.addFile(QString::fromUtf8(":/images/build.png"),
							QSize(), QIcon::Normal, QIcon::On);
	menu_BuildTarget->setIcon(icon_BuildTarget);

	menu_RunTarget = new Menu(false, menu_Build);
	QIcon icon_RunTarget;
	icon_RunTarget.addFile(QString::fromUtf8(":/images/run.png"),
							 QSize(), QIcon::Normal, QIcon::On);
	menu_RunTarget->setIcon(icon_RunTarget);

	action_ConfigProject = new QAction(this);
	QIcon icon_ConfigProject;
	icon_ConfigProject.addFile(QString::fromUtf8(":/images/config.png"),
						   QSize(), QIcon::Normal, QIcon::On);
	action_ConfigProject->setIcon(icon_ConfigProject);
	
	menu_Build->addMenu(menu_CurrentProject);
	menu_Build->addMenu(menu_BuildTarget);
	menu_Build->addMenu(menu_RunTarget);
	menu_Build->addAction(action_ConfigProject);
	
	menu_CurrentProject->addActions(projectManager->openedProjectNames());
	menu_CurrentProject->setCheckedAction(projectManager->currentProjectName());
}

void MainWindow::createDebugMenu()
{
	menu_Debug = new QMenu(menubar);

	QIcon icon_Debug;
	icon_Debug.addFile(QString::fromUtf8(":/images/debug.png"),
					   QSize(), QIcon::Normal, QIcon::On);
	action_Debug = new QAction(this);
	action_Debug->setIcon(icon_Debug);

	menu_Debug->addAction(action_Debug);
}

void MainWindow::createToolsMenu()
{
	menu_Tools = new QMenu(menubar);
}

void MainWindow::createWindowMenu()
{
	action_EditorView = new QAction(this);
	QIcon icon_EditorView;
	icon_EditorView.addFile(QString::fromUtf8(":/images/codeeditor.png"),
							QSize(), QIcon::Normal, QIcon::On);
	action_EditorView->setIcon(icon_EditorView);
	
	action_ProjectView = new QAction(this);
	QIcon icon_ProjectView;
	icon_ProjectView.addFile(QString::fromUtf8(":/images/project_view.png"),
							QSize(), QIcon::Normal, QIcon::On);
	action_ProjectView->setIcon(icon_ProjectView);

	action_BuildOutputView = new QAction(this);
	QIcon icon_BuildOutputView;
	icon_BuildOutputView.addFile(QString::fromUtf8(":/images/build_output.png"),
							 QSize(), QIcon::Normal, QIcon::On);
	action_BuildOutputView->setIcon(icon_BuildOutputView);

	action_TerminalView = new QAction(this);
	QIcon icon_TerminalView;
	icon_TerminalView.addFile(QString::fromUtf8(":/images/terminal.png"),
								 QSize(), QIcon::Normal, QIcon::On);
	action_TerminalView->setIcon(icon_TerminalView);

	menu_Window = new QMenu(menubar);
	menu_Window->addAction(action_EditorView);
	menu_Window->addAction(action_ProjectView);
	menu_Window->addAction(action_BuildOutputView);
	menu_Window->addAction(action_TerminalView);
}

void MainWindow::createHelpMenu()
{
	menu_Help = new QMenu(menubar);

	action_Help = new QAction(this);
	QIcon icon_Help;
	icon_Help.addFile(QString::fromUtf8(":/images/help.png"),
						   QSize(), QIcon::Normal, QIcon::On);
	action_Help->setIcon(icon_Help);

	menu_Help->addAction(action_Help);
}

void MainWindow::createToolBar()
{
	createFileToolBar();
	createBuildToolBar();
	createWindowToolBar();

	addToolBar(Qt::TopToolBarArea, file_ToolBar);
	addToolBar(Qt::TopToolBarArea, build_ToolBar);
	// addToolBar(Qt::TopToolBarArea, window_ToolBar); // window_ToolBar放在了statusbar中, 所以并不加了
}

void MainWindow::createFileToolBar()
{
	file_ToolBar = new QToolBar(this);
	
	QMenu *newMenu = new QMenu;
	newMenu->addAction(action_New_Source);
	newMenu->addAction(action_New_Header);
	newMenu->addAction(action_New_TextFile);
	newMenu->addAction(action_New_Project);

	QToolButton *newButton = new QToolButton(file_ToolBar);
	QIcon icon_New;
	icon_New.addFile(QString::fromUtf8(":/images/file_new.png"),
					 QSize(), QIcon::Normal, QIcon::On);
	newButton->setIcon(icon_New);
	newButton->setMenu(newMenu);
	
	QMenu *openMenu = new QMenu;
	openMenu->addAction(action_Open_File);
	openMenu->addAction(action_Open_Project);

	QToolButton *openButton = new QToolButton(file_ToolBar);
	QIcon icon_Open;
	icon_Open.addFile(QString::fromUtf8(":/images/file_open.png"),
					 QSize(), QIcon::Normal, QIcon::On);
	openButton->setIcon(icon_Open);
	openButton->setMenu(openMenu);

	connect(newButton, SIGNAL(clicked()), newButton, SLOT(showMenu()));
	connect(openButton, SIGNAL(clicked()), openButton, SLOT(showMenu()));

	file_ToolBar->addWidget(newButton);
	file_ToolBar->addWidget(openButton);
	file_ToolBar->addAction(action_Save_All);
}

void MainWindow::createEditToolBar()
{
}

void MainWindow::createBuildToolBar()
{

	action_Build = new QAction(this);
	QIcon icon_Build;
	icon_Build.addFile(QString::fromUtf8(":/images/build.png"),
					 QSize(), QIcon::Normal, QIcon::On);
	action_Build->setIcon(icon_Build);
	action_Build->setEnabled(false);

	action_Run = new QAction(this);
	QIcon icon_Run;
	icon_Run.addFile(QString::fromUtf8(":/images/run.png"),
					   QSize(), QIcon::Normal, QIcon::On);
	action_Run->setIcon(icon_Run);
	action_Run->setEnabled(false);

	currentProjectCombo = new QComboBox;
	currentProjectCombo->setFixedWidth(120);
	currentProjectCombo->setToolTip("选择项目");
	
	buildCombo = new QComboBox;
	buildCombo->setFixedWidth(100);
	buildCombo->setToolTip("选择要Build的目标");
	buildCombo->setEnabled(false);
	
	runCombo = new QComboBox;
	runCombo->setFixedWidth(100);
	runCombo->setToolTip("选择运行的可执行文件");
	runCombo->setEnabled(false);
	
	argsLineEdit = new QLineEdit;
	argsLineEdit->setMinimumWidth(100);
	argsLineEdit->setMaximumWidth(200);
	argsLineEdit->setToolTip("程序运行的参数");

	build_ToolBar = new QToolBar(this);
	build_ToolBar->addWidget(currentProjectCombo);
	build_ToolBar->addWidget(buildCombo);
	build_ToolBar->addAction(action_Build);
	build_ToolBar->addWidget(runCombo);
	build_ToolBar->addAction(action_Run);
	build_ToolBar->addAction(action_ConfigProject);
	build_ToolBar->addWidget(argsLineEdit);

	currentProjectCombo->addItems(QStringList() << QString("") << projectManager->openedProjectNames());
}

void MainWindow::createDebugToolBar()
{
	action_Debug = new QAction(this);
}

void MainWindow::createWindowToolBar()
{
	window_ToolBar = new QToolBar(this);
	window_ToolBar->addAction(action_EditorView);
	window_ToolBar->addAction(action_ProjectView);
	window_ToolBar->addAction(action_BuildOutputView);
	window_ToolBar->addAction(action_TerminalView);
}

void MainWindow::createStatusBar()
{
	statusbar = new QStatusBar;
	statusbar->addWidget(window_ToolBar);
	setStatusBar(statusbar);
}


void MainWindow::currentProjectChanged(const QString &projectName)
{
	// 如果 projectName 与 currentProjectCombo->currentTExt() 不一致, 则修改currentProjectCombo与之一致
	if (projectName != currentProjectCombo->currentText()) {
		int index = currentProjectCombo->findText(projectName, Qt::MatchFixedString);
		currentProjectCombo->setCurrentIndex(index);
	}

	if (projectName != menu_CurrentProject->checkedText()) {
		menu_CurrentProject->setCheckedAction(projectName);
	}

	if (projectManager->hasProject(projectName)) {
		CProject *cproject = projectManager->projectObject(projectName);
		if (cproject == 0)
			return;
		if (!cproject->isValid()) {
			QMessageBox::warning(this, "项目无效", cproject->errorInfo(), QMessageBox::Ok);
			return;
		}

		QStringList targets = cproject->targets();
		QStringList executeFiles = cproject->executeFiles();

		buildCombo->clear();
		runCombo->clear();
		menu_BuildTarget->clear();
		menu_RunTarget->clear();

		buildCombo->addItems(targets);
		runCombo->addItems(executeFiles);
		menu_BuildTarget->addActions(targets);
		menu_RunTarget->addActions(executeFiles);

		action_Build->setEnabled(true);
		buildCombo->setEnabled(true);
		action_Run->setEnabled(true);
		runCombo->setEnabled(true);
		menu_BuildTarget->setEnabled(true);
		menu_RunTarget->setEnabled(true);
		action_ConfigProject->setEnabled(true);
	} else {
		buildCombo->clear();
		runCombo->clear();
		menu_BuildTarget->clear();
		menu_RunTarget->clear();

		action_Build->setEnabled(false);
		buildCombo->setEnabled(false);
		action_Run->setEnabled(false);
		runCombo->setEnabled(false);
		menu_BuildTarget->setEnabled(false);
		menu_RunTarget->setEnabled(false);
		action_ConfigProject->setEnabled(false);
	}
}

void MainWindow::debug()
{
	QMessageBox::information(this, "暂时未实现该功能", "    期待您的参与!   ", QMessageBox::Ok);
}


void MainWindow::deleteBuilderWidget(BuilderWidget *builderWidget)
{
	QString projectName = builderWidget->projectName();
	if (builderWidgetMap.contains(builderWidget->projectName()))
		builderWidgetMap.remove(projectName);
		// delete builderWidget;
}


BuilderWidget* MainWindow::getBuilderWidget(const QString &projectName)
{
	if (!projectManager->hasProject(projectName)) {
		QMessageBox::warning(this, QString("不存在该项目\"" + projectName + "\""),
							 QString(), QMessageBox::Ok);
		return 0;
	}
	
	BuilderWidget *builderWidget;
	if (builderWidgetMap.contains(projectName)) {
		builderWidget = builderWidgetMap[projectName];
		buildOutput->setCurrentWidget(builderWidget);
	} else {
		CProject *cproject = projectManager->projectObject(projectName);
		if (!cproject->isValid()) {
			QMessageBox::warning(this, QString("项目\"" + projectName + "\"无效"),
								 QString(cproject->errorInfo()),
								 QMessageBox::Ok);
			return 0;
		}
		builderWidget = new BuilderWidget(projectName, cproject);
		builderWidgetMap[projectName] = builderWidget;
		connect(builderWidget, SIGNAL(deleteThis(BuilderWidget *)),
				this, SLOT(deleteBuilderWidget(BuilderWidget *)));
		buildOutput->addTab(builderWidget, projectName);
		buildOutput->setCurrentWidget(builderWidget);
	}

	return builderWidget;
}

void MainWindow::help()
{
	QMessageBox::information(this, "提示","请查看与本软件一起打包的\"帮助文档.pdf\"",
							 QMessageBox::Ok);
}


void MainWindow::newFile(const QString &title, const QString &path, const QString &filter)
{
	QString fromPath = path;
	// 如果path为空， 则设置fromPath为当前项目的根目录，根目录也为空， 则设置为主目录
	if (fromPath.isEmpty()) {
		fromPath = projectManager->projectPath(projectManager->currentProjectName());
		if (fromPath.isEmpty())
			fromPath = QDir::homePath();
	}

	QString filterStr = "All File(*)";
	if (!filter.isEmpty())
		filterStr = filter + ";;" + filterStr;
	QString filePath = QFileDialog::getSaveFileName(this, title, fromPath, filterStr);
	if (filePath.isEmpty())
		return;

	QFile file(filePath);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		file.write(QByteArray());
		editor->fileOpen(filePath, true);
		editor->show();
		projectTreeView->loadFile(filePath);
	}
}


void MainWindow::newHeader(const QString &path, const QString &projectName)
{
	newFile("新建头文件", path, "HeaderFile (*.h)");
}

void MainWindow::newProject()
{
	NewProjectWizard *newProjectWizard = new NewProjectWizard;
	if (newProjectWizard->exec() == QDialog::Accepted)
		projectManager->openProject(newProjectWizard->rootPath());
}


void MainWindow::newSource(const QString &path, const QString &projectName)
{
	newFile("新建源文件", path, "Source File(*.c *.cpp *.cc *.c++)");
}

void MainWindow::newTextFile(const QString &path, const QString &projectName)
{
	newFile("新建文本文件", path, QString());
}

void MainWindow::openFile(const QString &filePath)
{
	if (filePath.isEmpty())
		editor->fileOpenWithPath(projectManager->projectPath(projectManager->currentProjectName()));
	else
		editor->fileOpen(filePath);
	editor->show();
}

void MainWindow::openFile(const QString &filePath, ProjectTreeView::FileType fileType,
						  const QString &projectName)
{
	if (fileType == ProjectTreeView::Text) 
		editor->fileOpen(filePath);
	editor->show();
}

void MainWindow::openProject()
{
	QString fileName = QFileDialog::getOpenFileName(this, QString("选择项目的配置文件"), Global::projectHome(),
													QString("DoSmall Project(project.small)"));
	QFileInfo fileInfo(fileName);
	projectManager->openProject(fileInfo.absolutePath());
}


void MainWindow::projectUpdated(const QString &projectName)
{
	if (projectName == projectManager->currentProjectName()) {
		CProject *project = projectManager->projectObject(projectName);
		QStringList targets = project->targets();
		QStringList executeFiles = project->executeFiles();

		buildCombo->clear();
		runCombo->clear();
		menu_BuildTarget->clear();
		menu_RunTarget->clear();

		buildCombo->addItems(targets);
		runCombo->addItems(executeFiles);
		menu_BuildTarget->addActions(targets);
		menu_RunTarget->addActions(executeFiles);
	}
}


void MainWindow::projectClosed(const QString &projectName)
{
	int index = currentProjectCombo->findText(projectName, Qt::MatchFixedString);
	if (index >= 0) {
		currentProjectCombo->removeItem(index);
	}

	menu_CurrentProject->removeAction(projectName);
}

void MainWindow::projectDeleted(const QString &projectName)
{
	projectClosed(projectName);
}

void MainWindow::projectOpened(const QString &projectName)
{
	int index = currentProjectCombo->findText(projectName, Qt::MatchFixedString);
	if (index < 0) {
		currentProjectCombo->addItem(projectName);
	}

	if (!menu_CurrentProject->hasAction(projectName)) {
		menu_CurrentProject->addAction(projectName);
	}
	if (projectTreeView->isHidden())
		projectTreeView->show();
}

// private slots
void MainWindow::quit()
{
	if (editor->quit() && buildOutput->quit())
		QCoreApplication::quit();
} // quit

void MainWindow::run()
{
	QString projectName = currentProjectCombo->currentText();
	QString executeFile = runCombo->currentText();
	QString args = argsLineEdit->text();
	
	if (projectName.isEmpty() || !projectManager->hasProject(projectName) || executeFile.isEmpty())
		return;

	run(executeFile, args, projectName);
}

void MainWindow::run(const QString &executeFile, const QString &args, const QString &projectName)
{
	midSplitter->show();
	widgetBox->show();
	buildOutput->show();
	terminalView->hide();
	BuilderWidget *builderWidget = getBuilderWidget(projectName);
	if (builderWidget) {
		buildOutput->setCurrentWidget(builderWidget);
		builderWidget->run(executeFile, args);
	}
}

void MainWindow::runCurrentProject(const QString &executeFile)
{
	run(executeFile, QString(), projectManager->currentProjectName());
}

void MainWindow::setUi()
{
	resize(800, 600);

	QIcon icon_Logo;
	icon_Logo.addFile(QString::fromUtf8(":/images/logo.png"),
					  QSize(), QIcon::Normal, QIcon::Off);
	setWindowIcon(icon_Logo);

	projectManager = Global::projectManager();
	editor = new Editor(this);
	projectTreeView = new ProjectTreeView;
	projectTreeView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	buildOutput = new BuildOutput;
	terminalView = new TerminalView;

	widgetBox = new QWidget;
	widgetBox->setMinimumHeight(200);
	QVBoxLayout *vbox = new QVBoxLayout(widgetBox);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(buildOutput);
	vbox->addWidget(terminalView);

	midSplitter = new QSplitter(Qt::Vertical);
	midSplitter->setContentsMargins(0, 0, 0, 0);
	midSplitter->addWidget(editor);
	midSplitter->addWidget(widgetBox);
	midSplitter->setStretchFactor(0, 1);
	
	QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
	mainSplitter->setContentsMargins(0, 0, 0, 0);
	mainSplitter->addWidget(projectTreeView);
	mainSplitter->addWidget(midSplitter);
	mainSplitter->setStretchFactor(1, 1);

	setCentralWidget(mainSplitter);
	terminalView->hide();
	buildOutput->hide();
	widgetBox->hide();

	createMenuBar();
	createToolBar();
	createStatusBar();

	retranslateUi();
	QMetaObject::connectSlotsByName(this);
} // setupUi

void MainWindow::retranslateUi()
{
	setWindowTitle(QApplication::translate("MainWindow", "DoSmall", 0, QApplication::UnicodeUTF8));

	action_New_Source->setText(QApplication::translate("MainWindow", "新建源程序(&F)", 0, QApplication::UnicodeUTF8));
	action_New_Header->setText(QApplication::translate("MainWindow", "新建头文件(&H)", 0, QApplication::UnicodeUTF8));
	action_New_TextFile->setText(QApplication::translate("MainWindow", "新建文本(&T)", 0, QApplication::UnicodeUTF8));
	action_New_Project->setText(QApplication::translate("MainWindow", "新建项目(&P)", 0, QApplication::UnicodeUTF8));
	action_Open_File->setText(QApplication::translate("MainWindow", "打开文件(&E)", 0, QApplication::UnicodeUTF8));
	action_Open_Project->setText(QApplication::translate("MainWindow", "打开项目(&N)", 0, QApplication::UnicodeUTF8));
	action_Save->setText(QApplication::translate("MainWindow", "保存(&S)", 0, QApplication::UnicodeUTF8));
	action_Save_All->setText(QApplication::translate("MainWindow", "保存所有文件(&A)", 0, QApplication::UnicodeUTF8));
	action_Save_As->setText(QApplication::translate("MainWindow", "另存为(&V)", 0, QApplication::UnicodeUTF8));
	action_Close->setText(QApplication::translate("MainWindow", "关闭(&C)", 0, QApplication::UnicodeUTF8));
	action_Quit->setText(QApplication::translate("MainWindow", "退出(&Q)", 0, QApplication::UnicodeUTF8));

	action_Build->setText(QApplication::translate("MainWindow", "编译目标(F9)", 0, QApplication::UnicodeUTF8));
	action_Run->setText(QApplication::translate("MainWindow", "运行程序(F5)", 0, QApplication::UnicodeUTF8));
	action_ConfigProject->setText(QApplication::translate("MainWindow", "项目属性(&P)", 0, QApplication::UnicodeUTF8));

	action_Debug->setText(QApplication::translate("MainWindow", "调试(&D)", 0, QApplication::UnicodeUTF8));

	menu_CurrentProject->setTitle(QApplication::translate("MainWindow", "选择项目(&S)", 0, QApplication::UnicodeUTF8));
	menu_BuildTarget->setTitle(QApplication::translate("MainWindow", "编译目标(&B)", 0, QApplication::UnicodeUTF8));
	menu_RunTarget->setTitle(QApplication::translate("MainWindow", "运行目标(&R)", 0, QApplication::UnicodeUTF8));

	action_EditorView->setText(QApplication::translate("MainWindow", "编辑器(&E)", 0, QApplication::UnicodeUTF8));
	action_ProjectView->setText(QApplication::translate("MainWindow", "项目浏览器(&P)", 0, QApplication::UnicodeUTF8));
	action_BuildOutputView->setText(QApplication::translate("MainWindow", "编译输出(&O)", 0, QApplication::UnicodeUTF8));
	action_TerminalView->setText(QApplication::translate("MainWindow", "控制台(&T)", 0, QApplication::UnicodeUTF8));

	action_Help->setText(QApplication::translate("MainWindow", "帮助(&D)", 0, QApplication::UnicodeUTF8));

	menu_File->setTitle(QApplication::translate("MainWindow", "文件(&F)", 0, QApplication::UnicodeUTF8));
	menu_Build->setTitle(QApplication::translate("MainWindow", "编译运行(&B)", 0, QApplication::UnicodeUTF8));
	menu_Debug->setTitle(QApplication::translate("MainWindow", "调试(&D)", 0, QApplication::UnicodeUTF8));
	menu_Tools->setTitle(QApplication::translate("MainWindow", "工具(&T)", 0, QApplication::UnicodeUTF8));
	menu_Window->setTitle(QApplication::translate("MainWindow", "窗口(&W)", 0, QApplication::UnicodeUTF8));
	menu_Help->setTitle(QApplication::translate("MainWindow", "帮助(&H)", 0, QApplication::UnicodeUTF8));

	file_ToolBar->setWindowTitle(QApplication::translate("MainWindow", "文件工具栏", 0, QApplication::UnicodeUTF8));
	build_ToolBar->setWindowTitle(QApplication::translate("MainWindow", "编译运行工具栏", 0, QApplication::UnicodeUTF8));
} // retranslateUi

void MainWindow::toggleBuildOutputView()
{
	if (widgetBox->isHidden())
		widgetBox->show();
	
	if (buildOutput->isHidden()) {
		terminalView->hide();
		buildOutput->show();
	} else {
		buildOutput->hide();
		widgetBox->hide();
		if (editor->isHidden())
			midSplitter->hide();
	}
}

void MainWindow::toggleEditorView()
{
	if (midSplitter->isHidden())
		midSplitter->show();
	
	if (editor->isHidden()) {
		editor->show();
	} else {
		editor->hide();
		if (widgetBox->isHidden())
			midSplitter->hide();
	}
}

void MainWindow::toggleTerminalView()
{
	if (midSplitter->isHidden())
		midSplitter->show();
	
	if (widgetBox->isHidden())
		widgetBox->show();
	
	if (terminalView->isHidden()) {
		buildOutput->hide();
		terminalView->show();
		if (terminalView->count() <= 0)
			terminalView->newTerm();
	} else {
		terminalView->hide();
		widgetBox->hide();
		if (editor->isHidden())
			midSplitter->hide();
	}
}

void MainWindow::toggleProjectView()
{
	if (midSplitter->isHidden())
		midSplitter->show();
	
	if (projectTreeView->isHidden())
		projectTreeView->show();
	else
		projectTreeView->hide();
}
