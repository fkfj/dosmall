#include <QtGui/QHeaderView>
#include <QtGui/QToolBar>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

#include "filepathwidget.h"

FilePathWidget::FilePathWidget(const QString &labelText, const QString &prefix, bool selectDir, QWidget *parent)
	: QWidget(parent)
{
	this->selectDir = selectDir;
	this->prefix = prefix;
	modified = false;
	model = new QStandardItemModel;
	tableView = new QTableView;
	tableView->setModel(model);
	tableView->verticalHeader()->hide();
	tableView->horizontalHeader()->hide();
	tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	tableView->setDefaultDropAction(Qt::MoveAction);

	QIcon icon_Add;
	icon_Add.addFile(QString::fromUtf8(":/images/add.png"), QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Add = new QAction(this);
	action_Add->setIcon(icon_Add);
	
	QIcon icon_Del;
	icon_Del.addFile(QString::fromUtf8(":/images/out.png"), QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Del = new QAction(this);
	action_Del->setIcon(icon_Del);

	QIcon icon_Edit;
	icon_Edit.addFile(QString::fromUtf8(":/images/edit.png"), QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Edit = new QAction(this);
	action_Edit->setIcon(icon_Edit);
	
	QIcon icon_Up;
	icon_Up.addFile(QString::fromUtf8(":/images/go_up.png"), QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Up = new QAction(this);
	action_Up->setIcon(icon_Up);

	QIcon icon_Down;
	icon_Down.addFile(QString::fromUtf8(":/images/go_down.png"), QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Down = new QAction(this);
	action_Down->setIcon(icon_Down);
	
	connect(action_Add, SIGNAL(triggered()), this, SLOT(add()));
	connect(action_Del, SIGNAL(triggered()), this, SLOT(del()));
	connect(action_Edit, SIGNAL(triggered()), this, SLOT(edit()));
	connect(action_Up, SIGNAL(triggered()), this, SLOT(up()));
	connect(action_Down, SIGNAL(triggered()), this, SLOT(down()));
	
	QToolBar *toolBar = new QToolBar;
	toolBar->addAction(action_Add);
	toolBar->addAction(action_Del);
	toolBar->addAction(action_Edit);
	toolBar->addAction(action_Up);
	toolBar->addAction(action_Down);
	
	headerLabel = new QLabel(tr("%1").arg(labelText));

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addWidget(headerLabel);
	hbox->addWidget(toolBar);
	QGridLayout *layout = new QGridLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(1);
	layout->addLayout(hbox, 0, 0);
	layout->addWidget(tableView, 1, 0);

	filePathDialog = new FilePathDialog;

	setLayout(layout);
	setWindowTitle(tr("Property"));
}

QStringList FilePathWidget::getList()
{
	filePathList.clear();
	int count = model->rowCount();
	if (count <= 0)
		return filePathList;
	
	for (int i = 0; i < count; ++i) {
		filePathList << model->item(i)->text();
	}
	return filePathList;
}

void FilePathWidget::add(const QStringList &list)
{
	QStringList::const_iterator constIterator = list.constBegin();
	QStringList::const_iterator endIterator = list.constEnd();
	QStandardItem *newItem = 0;
	QList<int> rowList; 		// 添加的行列表
	int row = model->rowCount(); // 添加的起始行
	while (constIterator != endIterator) {
		QString str = *constIterator;
		str.remove(QRegExp("^" + prefix)); // 前去除前缀
		if (str.contains(QRegExp("\\s+"))) { // 如果包含空格, 则保证开始和末尾都有双引号
			if (!str.startsWith("\""))
				str = "\"" + str;
			if (!str.endsWith("\""))
				str = str + "\"";
		}
		str = prefix + str;		// 加上前缀
		newItem = new QStandardItem(str); // 创建该项的item
		model->insertRow(row, newItem);	  // 添加该item到model末尾
		rowList << row;			// 将改标记需要选择
		++constIterator;
	}
	qSort(rowList);						  // 将要选择的行排序
	tableView->selectionModel()->clear(); // 选择列表清空
	// 选中新添加的行
	for (int j = 0; j < rowList.count(); ++j) 
		tableView->selectionModel()->select(model->index(rowList.at(j), 0), QItemSelectionModel::Select);
	setModification(true);
}

bool FilePathWidget::isModified()
{
	return modified;
}

void FilePathWidget::setModification(bool b)
{
	modified = b;
}

void FilePathWidget::add()
{
	QStringList list;
	if (selectDir) 
		list = filePathDialog->getDir("添加目录", "");
	else 
		list = filePathDialog->getFiles("添加文件",  "");
	
	if (!list.isEmpty())
		add(list);
}

void FilePathWidget::del()
{
	// 选中行列表
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
	setModification(true);
}

void FilePathWidget::edit()
{
	// 选中列表indexes
	QModelIndexList indexes = tableView->selectionModel()->selectedRows();
	if (indexes.isEmpty())
		return;
	
	QStringList list;

	// 只编辑选中的第一行
	if ( selectDir ) 
		list = filePathDialog->getDir("编辑目录路径", model->data(indexes.at(0)).toString());
	else
		list = filePathDialog->getFiles("编辑文件路径", model->data(indexes.at(0)).toString());

	// 如果返回列表为空, 则返回
	if (list.isEmpty())
		return;

	// 如果返回列表为一项,且该项与原来值一样, 则返回
	if (list.size() == 1 && list.at(0) == model->data(indexes.at(0)).toString())
		return;

	// 如果以上都不是, 则说明编译的内容改变了, 删除原来的编辑的行, 添加心列表
	int row = indexes.at(0).row();
	model->removeRows(row, 1);
	add(list);
}

void FilePathWidget::up()
{
	QModelIndexList indexes = tableView->selectionModel()->selectedRows();
	if (indexes.isEmpty())
		return;

	// 如果选中的第一项是第一行, 则返回
	if (indexes.at(0).row() == 0)
		return;
	// 选中列表清空
	tableView->selectionModel()->clear();
	int size = indexes.size();
	int row = 0;
	QStandardItem *tmpItem;
	for (int i = 0; i < size; ++i) {
		row = indexes.at(i).row();
		// 将row - 1行保存在tmpItem中.
		tmpItem = new QStandardItem(model->item(row - 1)->text());
		// 设置row - 1行为row行的值
		model->setItem(row - 1, model->item(row));
		// 设置row行为tmpItem的值
		model->setItem(row, tmpItem);
		// 选中row-1行
		tableView->selectionModel()->select(model->index(row - 1, 0), QItemSelectionModel::Select);
	}
	setModification(true);
}

void FilePathWidget::down()
{
	QModelIndexList indexes = tableView->selectionModel()->selectedRows();
	if (indexes.isEmpty())
		return;

	// 如果选中的最后一项是最后一行, 则返回
	if (indexes.at(indexes.size() - 1).row() >= model->rowCount() - 1)
		return;
	
	tableView->selectionModel()->clear();
	int size = indexes.size();
	int row = 0;
	QList<int> list;			// 下移后需要选中行列表
	QStandardItem *tmpItem;
	for (int i = size - 1; i >= 0; --i) {
		row = indexes.at(i).row();
		// 将row + 1行保存在tmpItem中.
		tmpItem = new QStandardItem(model->item(row + 1)->text());
		// 设置row + 1行为row行的值
		model->setItem(row + 1, model->item(row));
		// 设置row行为tmpItem的值
		model->setItem(row, tmpItem);
		list << row + 1; 		// 标记改行下移后需选中
	}
	// 为了让选中的行的顺序是从小到大的,采取先排序再加入选中表中
	qSort(list);
	for (int j = 0; j < size; ++j)
		tableView->selectionModel()->select(model->index(list.at(j), 0), QItemSelectionModel::Select);
	setModification(true);
}

FilePathDialog::FilePathDialog(const QString &title, const QString &text, QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(title);
  
	lineEdit = new QLineEdit(text);
	this->selectDir = selectDir;
	fileSystem = new QPushButton("浏览(&F)...");
	ok = new QPushButton("确定(&O)");
	cancel = new QPushButton("取消(&C)");

	connect(fileSystem, SIGNAL(clicked()), this, SLOT(fileSystemSlot()));
	connect(ok, SIGNAL(clicked()), this, SLOT(okSlot()));
	connect(cancel, SIGNAL(clicked()), this, SLOT(cancelSlot()));
	connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(okSlot()));
	
	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->addWidget(cancel);
	hbox->addWidget(fileSystem);
	hbox->addWidget(ok);
	
	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->addWidget(lineEdit);
	vbox->addLayout(hbox);
	setLayout(vbox);	
}

QStringList FilePathDialog::getDir(const QString &title, const QString &text)
{
	setWindowTitle(title);
	lineEdit->setText(text);
	filePathList.clear();
	this->selectDir = true;
	this->exec();
	return filePathList;
}

QStringList FilePathDialog::getFiles(const QString &title, const QString &text)
{
	setWindowTitle(title);
	lineEdit->setText(text);
	filePathList.clear();
	this->selectDir = false;
	this->exec();
	return filePathList;
}

void FilePathDialog::fileSystemSlot()
{
	QString text = lineEdit->text();
	text.remove(QRegExp("\\s*:\\s*$"));
	if ( selectDir ) {
		QString dir =
			QFileDialog::getExistingDirectory(this, QString("选择目录"),
											 QDir::homePath(),
											 QFileDialog::ShowDirsOnly);
		text += " : " + dir;
	} else {
		QStringList fileList =
			QFileDialog::getOpenFileNames(this, tr("选择文件"),
										  QDir::homePath());
		QString files;
		QStringList::const_iterator it;
		for (it = fileList.constBegin(); it != fileList.constEnd();
			 ++it) {
			files += " : " + *it;
		}
		
		text += files;
	}
	
	text.remove(QRegExp("^\\s*:\\s*"));
	lineEdit->setText(text);
}

void FilePathDialog::okSlot()
{
	QString text = lineEdit->text();
	text.remove(QRegExp("^:"));
	text.remove(QRegExp(":$"));
	filePathList.clear();
	filePathList = text.split(QRegExp("\\s*:\\s*"));
	
	int index = filePathList.indexOf(QRegExp("^$"));
	while ( index >= 0 ) {
		filePathList.removeAt(index);
		index = filePathList.indexOf(QRegExp("^$"));
	}
	accept();
}

void FilePathDialog::cancelSlot()
{
	filePathList.clear();
	reject();
}

