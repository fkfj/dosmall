#include <QtCore/QRegExp>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <QtGui/QMessageBox>
#include <QtGui/QVBoxLayout>
#include <QtCore/QDebug>

#include "cprojectconfigpages.h"

BasicPage::BasicPage(CProject *project, QWidget *parent)
	: QWidget(parent)
{
	cproject = project;

	typeCombo = new QComboBox;
	typeCombo->addItem("C");
	typeCombo->addItem("C++");
	QLabel *typeLabel = new QLabel("项目类型(&T):");
	typeLabel->setBuddy(typeCombo);
	typeList << "C" << "C++";
	
	ccCompilerLineEdit = new QLineEdit;
	QLabel *ccCompilerLabel = new QLabel("C编译器(&E):");
	ccCompilerLabel->setBuddy(ccCompilerLineEdit);
	ccButton = new QPushButton("浏览...");
	connect(ccButton, SIGNAL(clicked()), this, SLOT(loadCCFilePath()));
	
	cccCompilerLineEdit = new  QLineEdit;
	QLabel *cccCompilerLabel = new QLabel("C++编译器(&F):");
	cccCompilerLabel->setBuddy(cccCompilerLineEdit);
	cccButton = new QPushButton("浏览...");
	connect(cccButton, SIGNAL(clicked()), this, SLOT(loadCCCFilePath()));

	makeLineEdit = new  QLineEdit;
	QLabel *makeLabel = new QLabel("Make(&M):");
	makeLabel->setBuddy(makeLineEdit);
	makeButton = new QPushButton("浏览...");
	connect(makeButton, SIGNAL(clicked()), this, SLOT(loadMakeFilePath()));

	developModeCombo = new QComboBox;
	developModeCombo->addItem(tr("Debug"));
	developModeCombo->addItem(tr("Release"));
	QLabel *developModeLabel = new QLabel("开发模式(&D):");
	developModeLabel->setBuddy(developModeCombo);
	developModeList << "Debug" << "Release";

	QLabel *warningLabel = new QLabel("警告选项:");
	wallCheckBox = new QCheckBox("-Wall");
	wCheckBox = new QCheckBox("-W");
	werrorCheckBox = new QCheckBox("-Werror");

	platformCombo = new QComboBox;
	platformCombo->addItem("默认");
	QLabel *platformLabel = new QLabel("平台(&P):");
	platformLabel->setBuddy(platformCombo);
	platformList << "";

	stdCombo = new QComboBox;
	stdCombo->addItem("默认");
	stdCombo->addItem("-ansi");
	stdCombo->addItem("-ansi -pedantic");
	stdCombo->addItem("-std=iso9899:1990");
	stdCombo->addItem("-std=iso9899:199409");
	stdCombo->addItem("-std=iso9899:1999");
	stdCombo->addItem("-std=gnu89");
	stdCombo->addItem("-std=gnu99");
	QLabel *stdLabel = new QLabel("标准(&S)");
	stdLabel->setBuddy(stdCombo);
	stdList << "" << "-ansi" << "-ansi -pedantic" << "-std=iso9899:1990" << "-std=9899:199409"
		<< "-std=9899:1999" << "-std=gnu89" << "-std=gnu99";

	otherFlagLineEdit = new QLineEdit;
	QLabel *otherFlagLabel = new QLabel("其它编译参数(&O):");
	otherFlagLabel->setBuddy(otherFlagLineEdit);
	
	loadSettings();
	
	QGridLayout *gridLayout = new QGridLayout();
	gridLayout->setContentsMargins(0, 0, 0, 0);
	gridLayout->addWidget(typeLabel, 0, 0);
	gridLayout->addWidget(typeCombo, 0, 1);
	gridLayout->addWidget(ccCompilerLabel, 1, 0);
	gridLayout->addWidget(ccCompilerLineEdit, 1, 1);
	gridLayout->addWidget(ccButton, 1, 2);
	gridLayout->addWidget(cccCompilerLabel, 2, 0);
	gridLayout->addWidget(cccCompilerLineEdit, 2, 1);
	gridLayout->addWidget(cccButton, 2, 2);
	gridLayout->addWidget(makeLabel, 3, 0);
	gridLayout->addWidget(makeLineEdit, 3, 1);
	gridLayout->addWidget(makeButton, 3, 2);
	gridLayout->addWidget(developModeLabel, 4, 0);
	gridLayout->addWidget(developModeCombo, 4, 1);
	gridLayout->addWidget(warningLabel, 5, 0);
	gridLayout->addWidget(wallCheckBox, 6 , 0);
	gridLayout->addWidget(wCheckBox, 7 , 0);
	gridLayout->addWidget(werrorCheckBox, 8 , 0);
	gridLayout->addWidget(platformLabel, 9, 0);
	gridLayout->addWidget(platformCombo, 9, 1);
	gridLayout->addWidget(stdLabel, 10, 0);
	gridLayout->addWidget(stdCombo, 10, 1);
	// gridLayout->addWidget(otherFlagLabel, 11, 0);
	// gridLayout->addWidget(otherFlagLineEdit, 12, 0);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->addLayout(gridLayout);
	mainLayout->addWidget(otherFlagLabel);
	mainLayout->addWidget(otherFlagLineEdit);
	mainLayout->addStretch(1);

	setLayout(mainLayout);
}

void BasicPage::loadSettings()
{
	// load type
	QString type = cproject->type();
	if (typeList.contains(type))
		typeCombo->setCurrentIndex(typeList.indexOf(type));
	else
		typeCombo->setCurrentIndex(1);

	// load cc
	QString cc = cproject->cc();
	ccCompilerLineEdit->setText(cc);

	// load ccc
	QString ccc = cproject->ccc();
	cccCompilerLineEdit->setText(ccc);

	// load make
	QString make = cproject->make();
	makeLineEdit->setText(make);
	
	// load DevelopMode
	QString developMode = cproject->developMode();
	if (developModeList.contains(developMode))
		developModeCombo->setCurrentIndex(developModeList.indexOf(developMode));
	else 
		developModeCombo->setCurrentIndex(0);

	// load Warnings
	QString warnings = cproject->warnings();
	if (warnings.contains("-Werror"))
		werrorCheckBox->setCheckState(Qt::Checked);
	else
		werrorCheckBox->setCheckState(Qt::Unchecked);

	if (warnings.contains("-W"))
		wCheckBox->setCheckState(Qt::Checked);
	else
		wCheckBox->setCheckState(Qt::Unchecked);

	if (warnings.contains("-Wall"))
		wallCheckBox->setCheckState(Qt::Checked);
	else
		wallCheckBox->setCheckState(Qt::Unchecked);

	// load Platform
	QString platform = cproject->platform();
	if (platformList.contains(platform))
		platformCombo->setCurrentIndex(platformList.indexOf(platform));
	else
		platformCombo->setCurrentIndex(0);

	// load Std
	QString std = cproject->std();
	if (stdList.contains(std))
		stdCombo->setCurrentIndex(stdList.indexOf(std));

	otherFlagLineEdit->setText(cproject->otherFlag());
}

void BasicPage::loadCCFilePath()
{
	QString filePath = QFileDialog::getOpenFileName(this, QString("选择C编译器"),
													QString("/usr/bin"));
	if (!filePath.isEmpty())
		ccCompilerLineEdit->setText(filePath);
}

void BasicPage::loadCCCFilePath()
{
	QString filePath = QFileDialog::getOpenFileName(this, QString("选择C++编译器"),
													QString("/usr/bin"));
	if (!filePath.isEmpty())
		ccCompilerLineEdit->setText(filePath);
}

void BasicPage::loadMakeFilePath()
{
	QString filePath = QFileDialog::getOpenFileName(this, QString("选择Make"),
													QString("/usr/bin"));
	if (!filePath.isEmpty())
		ccCompilerLineEdit->setText(filePath);
}

void BasicPage::save()
{
	// save type
	cproject->setType(typeList.at(typeCombo->currentIndex()));

	// save cc
	QString str = QString();
	str = ccCompilerLineEdit->text();
	str.remove(QRegExp("^\\s+"));
	str.remove(QRegExp("\\s+$"));
	cproject->setCC(str);

	// save ccc
	str = cccCompilerLineEdit->text();
	str.remove(QRegExp("^\\s+"));
	str.remove(QRegExp("\\s+$"));
	cproject->setCCC(str);

	// save make
	str = makeLineEdit->text();
	str.remove(QRegExp("^\\s+"));
	str.remove(QRegExp("\\s+$"));
	cproject->setMake(str);

	// save developMode
	cproject->setDevelopMode(developModeList.at(developModeCombo->currentIndex()));
	qDebug() << "developMode = " << developModeList.at(developModeCombo->currentIndex());
	// save warnings
	QString warnings = QString();
	if (wallCheckBox->checkState() == Qt::Checked)
		warnings += "-Wall";
	if (wCheckBox->checkState() == Qt::Checked)
		warnings += "-W";
	if (werrorCheckBox->checkState() == Qt::Checked)
		warnings += "-Werror";
	cproject->setWarnings(warnings);

	// save platform
	cproject->setPlatform(platformList.at(platformCombo->currentIndex()));

	// save std
	cproject->setStd(stdList.at(stdCombo->currentIndex()));

	QString otherFlag = otherFlagLineEdit->text();
	otherFlag.remove(QRegExp("^\\s+"));
	otherFlag.remove(QRegExp("\\s+$"));
	cproject->setOtherFlag(otherFlag);
}


IncludePage::IncludePage(CProject *project, QWidget *parent)
	: QWidget(parent)
{
	cproject = project;

	includePath = new FilePathWidget("Include Paths (-I)", "-I", true);
	includeFile = new FilePathWidget("Include Files (-include)", "-include", false);

	loadSettings();
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(includePath);
	vbox->addWidget(includeFile);
	setLayout(vbox);
}

void IncludePage::loadSettings()
{
	includePath->add(cproject->includePath());
	includeFile->add(cproject->includeFile());
}

void IncludePage::save()
{
	if (includePath->isModified())
		cproject->setIncludePath(includePath->getList());
	
	if (includeFile->isModified())
		cproject->setIncludeFile(includeFile->getList());
} 

LibraryPage::LibraryPage(CProject *project, QWidget *parent)
	: QWidget(parent)
{
	cproject = project;

	libraryPath = new FilePathWidget("Library Path (-L)", "-L", true);
	libraryFile = new FilePathWidget("Library File (-l)", "-l", false);

	loadSettings();
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(libraryPath);
	vbox->addWidget(libraryFile);
	setLayout(vbox);
}

void LibraryPage::loadSettings()
{
	libraryPath->add(cproject->libraryPath());
	libraryFile->add(cproject->libraryFile());
}

void LibraryPage::save()
{
	if (libraryPath->isModified())
		cproject->setLibraryPath(libraryPath->getList());
	
	if (libraryFile->isModified())
		cproject->setLibraryFile(libraryFile->getList());
}

MainSourceDependPage::MainSourceDependPage(CProject *project, QWidget *parent)
	:QWidget(parent)
{
	cproject = project;
	infoList << "\"main文件\" —— 指含main函数且可以编译成可执行文件的源文件。"
			 << "\"源文件\" —— 指以.c/.cpp/.cc/.cxx为后缀的源文件，不包括以.h为后缀的头文件。"
			 << "\"依赖源文件\" —— 指不包含main函数的源文件，同样不包含头文件。"
			 << "项目中允许有多个含main文件对开发和测试都比较方便，为了缩小编译的范围, 可以设置main文件依赖的其它非main源文件，设置时需注意以下几项:"
			 << "1.如果项目中只有一个main文件，可以不设置依赖关系。这种情况下项目编译得到一个以项目名称为名的可执行文件，它依赖所有源文件。"
			 << "2.如果项目中有多个main文件, 必须将main文件添加到main文件列表中, 否则会按照第1条中的规则编译，将出现有多个main函数的错误。"
			 << "3.非main文件不要添加到main文件文件列表中, main文件不要添加到依赖源文件列表中。任何非源文件不要添加到main文件列表和依赖文件列表中。"
			 << "4.不要添加头文件, 头文件也不需要处理, 因为编译时会自动处理源文件对头文件的依赖关系"
			 << "5.\"依赖所有\"表示该main的源文件依赖所有非main源文件，\"指定依赖\"表示依赖指定的源文件，\"不依赖\"表示该main文件不依赖其它任何文件。"
			 << "6.如果一个main文件设置了\"指定依赖\", 则需要将该main文件依赖的源文件添加完整，否则会出现找不到变量/函数/类等情况。"
			 << "7.所有源程序都必须放在项目的根目录或其子目录下，否则会无法编译该文件。";
	infoButton = new QPushButton("显示提示");
	infoButton->setMaximumWidth(120);
	connect(infoButton, SIGNAL(clicked()), this, SLOT(showInfo()));

	contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::ListMode);
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setSpacing(4);
    connect(contentsWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*, QListWidgetItem*)));

    pagesWidget = new QStackedWidget;

	QIcon icon_Add;
	icon_Add.addFile(QString::fromUtf8(":/images/add.png"), QSize(24, 24), QIcon::Normal, QIcon::On);
	action_AddMainSource = new QAction("添加main源文件", this);
	action_AddMainSource->setIcon(icon_Add);

	QIcon icon_Out;
	icon_Out.addFile(QString::fromUtf8(":/images/out.png"), QSize(24, 24), QIcon::Normal, QIcon::On);
	action_OutMainSource = new QAction("从列表中删除", this);
	action_OutMainSource->setIcon(icon_Out);

	connect(action_AddMainSource, SIGNAL(triggered()), this, SLOT(addMainSource()));
	connect(action_OutMainSource, SIGNAL(triggered()), this, SLOT(outMainSource()));

	QToolBar *mainToolBar = new QToolBar;
	mainToolBar->setGeometry(QRect(0, 0, 0, 0));
	mainToolBar->addAction(action_AddMainSource);
	mainToolBar->addAction(action_OutMainSource);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setGeometry(QRect(0, 0, 0, 0));
	mainLayout->addWidget(infoButton);
	mainLayout->addWidget(new QLabel("main源文件:"));
	mainLayout->addWidget(mainToolBar);
	mainLayout->addWidget(contentsWidget);
	mainLayout->addWidget(pagesWidget);
	loadSettings();
}

void MainSourceDependPage::addMainSource()
{
	QString filePath = QFileDialog::getOpenFileName(this, "选择main源文件", cproject->rootPath(),
													QString("C/C++ (*.c *.cc *.cpp *.cxx *.c++)"));
	if (!filePath.isEmpty()) {
		filePath.remove(QRegExp("^" + cproject->rootPath() +"/"));
		QListWidgetItem *button = new QListWidgetItem;
		button->setText(filePath);
		button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		contentsWidget->addItem(button);
		
		DependSourceWidget *depend = new DependSourceWidget(cproject->rootPath(), filePath + "依赖的源文件");
		pagesWidget->addWidget(depend);
		depend->dependAll();
	}
}

void MainSourceDependPage::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}


void MainSourceDependPage::loadSettings()
{
	QStringList mainSourceList = cproject->mainSources();
	QStringList::const_iterator constIterator = mainSourceList.constBegin();
	QStringList::const_iterator endIterator = mainSourceList.constEnd();
	while (constIterator != endIterator) {
		QListWidgetItem *button = new QListWidgetItem;
		button->setText(*constIterator);
		button->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		contentsWidget->addItem(button);
		
		DependSourceWidget *depend = new DependSourceWidget(cproject->rootPath(), *constIterator + "依赖的源文件");
		pagesWidget->addWidget(depend);
		QStringList dependList = cproject->mainSourceDepend(*constIterator);
		if (dependList.at(0).toLower() == "part") {
			depend->dependPart();
			dependList.removeFirst();
			depend->addSource(dependList);
		} else if (dependList.at(0).toLower() == "only") {
			depend->dependOnly();
		} else {
			depend->dependAll();
		}
		++constIterator;
	}
}


void MainSourceDependPage::outMainSource()
{
	int row = contentsWidget->currentRow();
	if (row < 0)
		return;
	pagesWidget->removeWidget(pagesWidget->widget(row));
	contentsWidget->takeItem(row);
}

void MainSourceDependPage::save()
{
	int count = contentsWidget->count();
	if (count <= 0)
		return;
	QStringList mainSourceList;
	QStringList dependList;
	DependSourceWidget *dependWidget = 0;
	for (int i = 0; i < count; ++i) {
		QString mainSource = contentsWidget->item(i)->text();
		mainSourceList << mainSource;
		dependWidget = qobject_cast<DependSourceWidget *>(pagesWidget->widget(i));
		if (!dependWidget) {
			continue;
		} else if (dependWidget->isDependAll()) {
			dependList << "all " + mainSource;
		} else if (dependWidget->isDependOnly()) {
			dependList << "only " + mainSource;
		} else if (dependWidget->isDependPart()) {
			dependList << "part " + mainSource + " " + dependWidget->getList().join(" ");
		}
	}
	cproject->setMainSources(mainSourceList);
	cproject->setMainSourceDepend(dependList);
}

void MainSourceDependPage::showInfo()
{
	QMessageBox::information(0, "关于主文件的依赖关系", infoList.join("\n"), QMessageBox::Ok);
}

DependSourceWidget::DependSourceWidget(const QString &rootPath, const QString &labelText,  QWidget *parent)
	: QWidget(parent)
{
	path = rootPath;
	QLabel *label = new QLabel(labelText);
	label->setWordWrap(true);
	
	QIcon icon_Add;
	icon_Add.addFile(QString::fromUtf8(":/images/add.png"), QSize(24, 24), QIcon::Normal, QIcon::On);
	action_AddSource = new QAction("添加依赖的源文件", this);
	action_AddSource->setIcon(icon_Add);
	
	QIcon icon_Out;
	icon_Out.addFile(QString::fromUtf8(":/images/out.png"), QSize(24, 24), QIcon::Normal, QIcon::On);
	action_OutSource = new QAction("从列表中删除", this);
	action_OutSource->setIcon(icon_Out);
	
	dependAllButton = new QRadioButton("依赖所有");
	dependAllButton->setToolTip("依赖所有非main源文件");
	dependPartButton = new QRadioButton("指定依赖");
	dependPartButton->setToolTip("依赖指定的非main源文件");
	dependOnlyButton = new QRadioButton("不依赖");
	dependOnlyButton->setToolTip("不依赖其它源文件");

	buttonGroup = new QButtonGroup;
	buttonGroup->addButton(dependAllButton);
	buttonGroup->addButton(dependPartButton);
	buttonGroup->addButton(dependOnlyButton);

	connect(action_AddSource, SIGNAL(triggered()), this, SLOT(addSource()));
	connect(action_OutSource, SIGNAL(triggered()), this, SLOT(outSource()));
	connect(dependAllButton, SIGNAL(clicked()), this, SLOT(dependAll()));
	connect(dependPartButton, SIGNAL(clicked()), this, SLOT(dependPart()));
	connect(dependOnlyButton, SIGNAL(clicked()), this, SLOT(dependOnly()));

	QToolBar *dependToolBar = new QToolBar;
	dependToolBar->addAction(action_AddSource);
	dependToolBar->addAction(action_OutSource);
	dependToolBar->addWidget(dependAllButton);
	dependToolBar->addWidget(dependPartButton);
	dependToolBar->addWidget(dependOnlyButton);

	model = new QStandardItemModel;
	tableView = new QTableView;
	tableView->setModel(model);
	tableView->verticalHeader()->hide();
	tableView->horizontalHeader()->hide();
	tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	dependAll();
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(label);
	vbox->addWidget(dependToolBar);
	vbox->addWidget(tableView);
}


void DependSourceWidget::addSource()
{
	QStringList list = QFileDialog::getOpenFileNames(this, QString("选择源文件"), path);
	if (!list.isEmpty())
		addSource(list);
}

void DependSourceWidget::addSource(const QStringList &list)
{
	QStringList tmpList = list;
	tmpList.replaceInStrings(QRegExp("^" + path + "/"), "");
	QStringList::const_iterator constIterator = tmpList.constBegin();
	QStringList::const_iterator endIterator = tmpList.constEnd();
	while (constIterator != endIterator) {
		model->appendRow(new QStandardItem(*constIterator));
		++constIterator;
	}
}

QStringList DependSourceWidget::getList()
{
	QStringList list;
	int count = model->rowCount();
	if (count <= 0)
		return list;
	
	for (int i = 0; i < count; ++i) {
		list << model->item(i)->text();
	}
	return list;
}

bool DependSourceWidget::isDependAll()
{
	return status == DependSourceWidget::All;
}

bool DependSourceWidget::isDependPart()
{
	return status == DependSourceWidget::Part;
}

bool DependSourceWidget::isDependOnly()
{
	return status == DependSourceWidget::Only;
}


void DependSourceWidget::dependAll()
{
	status = DependSourceWidget::All;
	dependAllButton->setChecked(true);
	action_AddSource->setEnabled(false);
	action_OutSource->setEnabled(false);
	tableView->setEnabled(false);
}
 

void DependSourceWidget::dependPart()
{
	status = DependSourceWidget::Part;
	dependPartButton->setChecked(true);
	action_AddSource->setEnabled(true);
	action_OutSource->setEnabled(true);
	tableView->setEnabled(true);
}


void DependSourceWidget::dependOnly()
{
	status = DependSourceWidget::Only;
	dependOnlyButton->setChecked(true);
	action_AddSource->setEnabled(false);
	action_OutSource->setEnabled(false);
	tableView->setEnabled(false);
}

int DependSourceWidget::dependStatus()
{
	return status;
}
 
void DependSourceWidget::outSource()
{
	QModelIndexList indexes = tableView->selectionModel()->selectedRows();
	if (indexes.isEmpty())
		return;

	int size = indexes.size();
	int row = 0;
	// 这里必须从size-1位置删起, 因为删除了0位置的, 原来1位置的就成了0位置...
	for (int i = size - 1; i >= 0 ; --i) { 
		QModelIndex index = indexes.at(i);
		row = index.row();
		model->removeRows(row, 1); // 删除行row
	}
	// 如果row大于model的行数, 则row等于模行
	if (row >= model->rowCount())
		row = model->rowCount() - 1;
	// 选中行row
	tableView->selectionModel()->select(model->index(row, 0), QItemSelectionModel::Select);
}
