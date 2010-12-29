#ifndef BUILDERWIDGET_H
#define BUILDERWIDGET_H

#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QToolBar>
#include <QtGui/QComboBox>

#include "cbuilder.h"
#include "processaction.h"
#include "../project/cproject.h"
#include "../editor/plaineditor.h"

class BuilderWidget : public QWidget
{
	Q_OBJECT;
	
public:
	BuilderWidget(const QString &projectName, CProject *cproject, QWidget *parent = 0);
	
	bool isBuilding();
	bool isValid();
	void exitAllRunProcess();
	bool hasRunningProcess();
	QString projectName();

public slots:
	void appendText(const QString &text); // 添加text 到textEdit 末尾行的下一行
	void buildAll();
	void build(const QString &target);
	void buildClean();
	void cleanProject();
	void breakBuild();
	void run(const QString &execFile, const QString &args = QString());
	void clearText();
	bool close();

private slots:
	void addProcessAction(ProcessAction *processAction);
	void remvoeProcessAction(ProcessAction *processAction);
	
signals:
	void gotoError();		// 跳转到错误代码位置
	void deleteThis(BuilderWidget *builderWidget);

private:
	QString checkExecFile(const QString &execFile);

	bool valid;
	QString name;
	QToolBar *toolBar;
	CBuilder *cbuilder;
	CProject *cprojectObject;
	PlainEditor *textEdit;
	QAction *action_Clear;
	QList<ProcessAction*> processActionList;
};

#endif
