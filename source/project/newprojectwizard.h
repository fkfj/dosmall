#ifndef NEWPROJECTWIZARD_H
#define NEWPROJECTWIZARD_H

#include <QtCore/QStringList>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QWizard>

#include "../global.h"

class NewProjectWizard : public QWizard
{
	Q_OBJECT;
	
public:
	NewProjectWizard(QWidget *parent = 0);

	void accept();
	QString rootPath();
	bool success();
	
private:
	QString path;
	bool valid;
};

class ProjectInfoPage : public QWizardPage
{
	Q_OBJECT;
	
public:
	ProjectInfoPage(QWidget *parent = 0);

private slots:
	bool checkName(const QString &text);
	bool checkLocation(const QString &text);

private:
	bool checkName();
	bool checkLocation();
	bool checkPath();

	QStringList existsProjectName;
	QLabel *projectTypeLabel; 
	QComboBox *projectTypeComboBox; // 对象类型选择
	QLabel *projectNameLabel;		
	QLineEdit *projectNameLineEdit;	
	QLabel *projectLocationLabel;	
	QLineEdit *projectLocationLineEdit;
	QLabel *projectPathLabel;
	QLineEdit *projectPathLineEdit;
	QLabel *warningLabel;		   // 项目路径警告
	QLineEdit *warningLineEdit;    // 用与提供警告的lineEdit, 不显示在page中
	QString projectName;
	QString projectLocation;
	QString projectPath;
};

#endif
