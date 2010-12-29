#include <QtGui/QGridLayout>
#include <QtGui/QMessageBox>

#include "newprojectwizard.h"
#include "../tools/filetools.h"

NewProjectWizard::NewProjectWizard(QWidget *parent)
	: QWizard(parent)
{
	addPage(new ProjectInfoPage);
	setWindowTitle(tr("新建项目"));
}

void NewProjectWizard::accept()
{
	int type = field("projectType").toInt();
	QString projectName = field("projectName").toString();
	QString projectLocation = field("projectLocation").toString();
	QString projectPath = field("projectPath").toString();
	valid = false;
	QString projectType;
	switch (type) {
	case 0:
		projectType = "C";
		break;
	case 1:
		projectType = "C++";
		break;
	default:
		projectType = "C++";
		break;
	}
	
	QRegExp rx1("^\\s*");
	QRegExp rx2("\\s*$");
	QRegExp rx3("/$");
	projectName.remove(rx1);
	projectName.remove(rx2);
	projectLocation.remove(rx1);
	projectLocation.remove(rx2);
	projectLocation.remove(rx3);

	FileTools fileTools;
	if (!fileTools.mkdir(projectPath)) {
		QMessageBox::warning(0, QObject::tr("创建目录失败"),
							 QObject::tr("%1")
							 .arg(fileTools.errorString()));
		return;
	}
	
	if (!QFile::copy(Global::projectConfigTemplate(), projectPath + "/project.small")) {
		QMessageBox::warning(0, QObject::tr("创建项目配置文件失败"),
							 QObject::tr("copy from:%1 \ncopy to:%2")
							 .arg(Global::projectConfigTemplate())
							 .arg(projectPath + "/project.small"));
		return;
	}

	QSettings *settings = new QSettings(projectPath + "/project.small", QSettings::IniFormat);
	settings->setValue("Type", projectType);
	settings->setValue("Name", projectName);
	settings->sync();
	delete settings;
	valid = true;
	path = projectPath;
    QDialog::accept();
}

QString NewProjectWizard::rootPath()
{
	return path;
}

bool NewProjectWizard::success()
{
	return valid;
}


ProjectInfoPage::ProjectInfoPage(QWidget *parent)
    : QWizardPage(parent)

{
	setTitle(tr("项目信息"));

	existsProjectName = Global::projectManager()->allProjectNames();
	projectLocation = Global::projectHome();
	projectPath = projectLocation;

	// 项目类型选择
	projectTypeComboBox = new QComboBox;
	projectTypeComboBox->addItem("C");
	projectTypeComboBox->addItem("C++");
	projectTypeLabel  = new QLabel(tr("项目类型(&T):"));
	projectTypeLabel->setBuddy(projectTypeComboBox);

	// 项目名称
	projectNameLineEdit = new QLineEdit;
	projectNameLabel = new QLabel(tr("项目名称(&N):"));
	projectNameLabel->setBuddy(projectNameLineEdit);

	// 项目位置
	projectLocationLineEdit = new QLineEdit(projectLocation);
	projectLocationLabel = new QLabel(tr("项目位置(&L):"));
	projectLocationLabel->setBuddy(projectLocationLineEdit);

	// 项目文件夹
	projectPathLineEdit = new QLineEdit(projectPath);
	projectPathLabel = new QLabel(tr("项目文件夹:"));
	projectPathLabel->setBuddy(projectPathLineEdit);

	// 警告栏
	warningLabel = new QLabel;
	warningLabel->setWordWrap(true);
	warningLineEdit = new QLineEdit();
	
	connect(projectNameLineEdit, SIGNAL(textChanged(const QString &)),
			this, SLOT(checkName(const QString &)));
	connect(projectLocationLineEdit, SIGNAL(textChanged(const QString &)),
			this, SLOT(checkLocation(const QString &)));

	registerField("projectType", projectTypeComboBox);
	registerField("projectName*", projectNameLineEdit);
	registerField("projectLocation", projectLocationLineEdit);
    registerField("projectPath", projectPathLineEdit);
    registerField("warning*", warningLineEdit);

    QGridLayout *layout = new QGridLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(projectTypeLabel, 0, 0);
	layout->addWidget(projectTypeComboBox, 0, 1);
	layout->addWidget(projectNameLabel, 1, 0);
	layout->addWidget(projectNameLineEdit, 1, 1);
	layout->addWidget(projectLocationLabel, 2, 0);
	layout->addWidget(projectLocationLineEdit, 2, 1);
	layout->addWidget(projectPathLabel, 3, 0);
	layout->addWidget(projectPathLineEdit, 3, 1);
	layout->addWidget(warningLabel, 4, 1);
	setLayout(layout);
}

bool ProjectInfoPage::checkName(const QString &text)
{

	projectName = text;
	projectName.remove(QRegExp("^\\s*"));
	projectName.remove(QRegExp("\\s*$"));
	projectPathLineEdit->setText(projectLocation + "/" + projectName);

	if (!checkName())
		return false;

	if (!checkLocation())
		return false;

	if (!checkPath())
		return false;

	warningLabel->clear();
	warningLineEdit->setText(tr("t"));
	return true;
}

bool ProjectInfoPage::checkName()
{
	// 检查是否全是空格
	if (projectName.isEmpty()) {
		warningLabel->setText(tr("<font color=red size=3>项目名称不能为空。</font>"));
		warningLineEdit->clear();
		return false;
	}

	if (existsProjectName.contains(projectName)) {
		warningLabel->setText(tr("<font color=red size=3>项目名称\"%1\"已被使用。</font>").arg(projectName));
		warningLineEdit->clear();
		return false;
	}
	
	// 检查文件夹名称是否有效
	if ( projectName.contains("/") || projectName.contains("\\")
		 || projectName.contains("<") || projectName.contains(">")
		 || projectName.contains("*") ) {
		// warningLabel->setText(tr("项目名称不是有效的文件夹名称: 不能包含/\\<>*"));
		warningLabel->setText("项目名称不是有效的文件夹名称: 不能包含/\\*<>");
		warningLineEdit->clear();
		return false;
	}
	return true;
}

bool ProjectInfoPage::checkLocation(const QString &text)
{
	projectLocation = text;
	projectLocation.remove(QRegExp("^\\s*"));
	projectLocation.remove(QRegExp("\\s*$"));
	projectLocation.remove(QRegExp("/$"));
	projectPathLineEdit->setText(projectLocation + "/" + projectName);
	
	if (!checkLocation())
		return false;
	if (!checkName())
		return false;
	if (!checkPath())
		return false;
	
	warningLabel->clear();
	warningLineEdit->setText(tr("t"));
	return true;
}

bool ProjectInfoPage::checkLocation()
{
	QString currentPath = projectLocation;
	QStringList nodes = currentPath.split("/", QString::SkipEmptyParts);

	while (nodes.count() > 0) {
		QFileInfo fileInfo(currentPath);
		if (fileInfo.exists()) { // 如果存在该目录.
			if (!fileInfo.isDir()) { // 如果不是有效目录, 则返回false.
				warningLabel->setText(tr("<font color=red size=3>项目路径不是有效目录: %1</font>").arg(currentPath));
				warningLineEdit->clear();
				return false;
			}
			if (!fileInfo.isWritable()) { // 如果该目录可写, 则返回false
				warningLabel->setText(tr("<font color=red size=3>项目路径为只读: %1</font>").arg(currentPath));
				warningLineEdit->clear();
				return false;
			} else				// 如果存在该目录且可写, 则返回true
				return true;
		}
		// 如果不存在该目录, 则检查该路经的父目录
		currentPath.remove(QRegExp(nodes.last() + "$"));
		nodes.removeLast();
	}
	return false;
}

bool ProjectInfoPage::checkPath()
{
	projectPath = projectPathLineEdit->text();

	QFileInfo fileInfo(projectPath);
	if (!fileInfo.exists()) {
		return true;
	}

	if (!fileInfo.isDir()) {
		warningLabel->setText(tr("<font color=red size=3>%1 已经存在且不是目录。</font>").arg(projectPath));
		warningLineEdit->clear();
		return false;
	}

	if (QFile::exists(projectPath + "/project.small")) {
		warningLabel->setText(tr("%1 已经有一个项目。").arg(projectPath));
		warningLineEdit->clear();
		return false;
	}
	
	QDir dir(projectPath);
	if (dir.count() > 2) 
		warningLabel->setText(tr("<font color=red size=3>提示: %1 已经存在且不为空。</font>").arg(projectPath));
	return true;
}
