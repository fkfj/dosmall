#ifndef MENU_H
#define MENU_H

#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtGui/QMenu>

class Menu : public QMenu
{
	Q_OBJECT;
	
public:
	Menu(bool exclusive = false, QWidget *parent = 0);

	void addAction(const QString &text);
	void addActions(const QStringList &list);
	void addActions(const QList<QAction *> &actions);
	bool hasAction(const QString &text);
	QString checkedText();
	void removeAction(const QString &text);
	void setCheckedAction(const QString &text);
	void clear();
				
private slots:
	void triggeredSlot(QAction *action);
	
signals:
	void text(const QString &str);

private:
	bool exclusive;
	QActionGroup *actionGroup;
	QMap<QString, QAction*> textActionMap;
};

#endif
