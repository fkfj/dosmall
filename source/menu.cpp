#include "menu.h"

Menu::Menu(bool exclusive, QWidget *parent)
	:QMenu(parent)
{
	connect(this, SIGNAL(triggered(QAction *)), this, SLOT(triggeredSlot(QAction *)));
	this->exclusive = exclusive;
	if (exclusive) {
		actionGroup = new QActionGroup(this);
		actionGroup->setExclusive(true);
	} else {
		actionGroup = 0;
	}
}

void Menu::addAction(const QString &text)
{
	if (textActionMap.contains(text))
		return;

	QAction *action = new QAction(text, this);
	textActionMap[text] = action;
	if (exclusive) {
		action->setCheckable(true);
		actionGroup->addAction(action);
	}
	QMenu::addAction(action);
}

void Menu::addActions(const QStringList &list)
{
	QStringList::const_iterator constIterator = list.constBegin();
	QStringList::const_iterator endIterator = list.constEnd();
	while(constIterator != endIterator) {
		QAction *action = new QAction(*constIterator, this);
		if (exclusive) {
			action->setCheckable(true);
			actionGroup->addAction(action);
		}
		QMenu::addAction(action);
		textActionMap[*constIterator] = action;
		++constIterator;
	}
}

void Menu::addActions(const QList<QAction *> &actions)
{
	QList<QAction *>::const_iterator constIterator = actions.constBegin();
	QList<QAction *>::const_iterator endIterator = actions.constEnd();
	while(constIterator != endIterator) {
		if (exclusive) {
			(*constIterator)->setCheckable(true);
			actionGroup->addAction(*constIterator);
		}
		QMenu::addAction(*constIterator);
		textActionMap[(*constIterator)->text()] = *constIterator;
		++constIterator;
	}
}

QString Menu::checkedText()
{
	if (actionGroup) {
		QAction *action = actionGroup->checkedAction();
		if (action)
			return action->text();
		else
			return QString();
	} else {
		return QString();
	}
}

bool Menu::hasAction(const QString &text)
{
	return textActionMap.contains(text);
}

	
void Menu::removeAction(const QString &text)
{
	if (!textActionMap.contains(text))
		return;

	if (exclusive)
		actionGroup->removeAction(textActionMap[text]);
	QMenu::removeAction(textActionMap[text]);
	textActionMap.remove(text);
}

void Menu::setCheckedAction(const QString &text)
{
	if (!exclusive || !actionGroup || !textActionMap.contains(text))
		return;

	if (text.isEmpty()) {
		if (actionGroup->checkedAction())
			actionGroup->checkedAction()->setChecked(false);
	} else {
		textActionMap[text]->setChecked(true);
	}
}

void Menu::clear()
{
	if (exclusive || actionGroup) {
		QMap<QString, QAction*>::const_iterator i = textActionMap.constBegin();
		while (i != textActionMap.constEnd()) {
			actionGroup->removeAction(i.value());
			++i;
		}
	}
	textActionMap.clear();
	QMenu::clear();
}

void Menu::triggeredSlot(QAction *action)
{
	emit text(action->text());
}
