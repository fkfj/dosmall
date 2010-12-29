// #include <QtGui>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>

#include "codeeditor.h"

// public
CodeEditor::CodeEditor(bool readOnly, QWidget *parent)
	: QWidget(parent), readOnly(readOnly)
{
    fileName = QString("noname.txt"); // 应避免重名
	setUi();
	setConnect();
} // CodeEditor

CodeEditor::CodeEditor(const QString &filePath, bool readOnly, QWidget *parent)
	: QWidget(parent), readOnly(readOnly) 
{
	setFilePath(filePath);
	setUi();
	setConnect();
} // CodeEditor

QString CodeEditor::getFileName()
{
    return fileName;
} // getFileName

QString CodeEditor::getPath()
{
    return path;
} // getPath

QString CodeEditor::getFilePath()
{
    return path + "/" + fileName;
} // getFilePath

void CodeEditor::setFileName(const QString &fileName)
{
	this->fileName = fileName;
}

void CodeEditor::setPath(const QString &path)
{
	this->path = path;
}

void CodeEditor::setFilePath(const QString &filePath)
{
	QFileInfo fileInfo(filePath);
	path = fileInfo.absolutePath();
	fileName = fileInfo.fileName();
} // setFilePath

PlainEditor* CodeEditor::getPlainEditor()
{
    return plainEditor;
} // getTextEdit



bool CodeEditor::isModified()			// 是否修改了缓存区
{
	return plainEditor->document()->isModified();
} // isModified

// public slote
void CodeEditor::modificationChanged(bool b)
{
    emit modificationChangedSignal(b, this);
} // changedSlot

void CodeEditor::fileNew()
{
	emit fileNewWithPathSignal(getPath());
}

void CodeEditor::fileOpen()
{
	emit fileOpenWithPathSignal(getPath());
}

void CodeEditor::reload()
{
    QFile file(getFilePath());
    if (!(file.open(QFile::ReadOnly | QIODevice::Text))) {
		QMessageBox::information(this, tr("无法读取该文件"),
								 tr("%1").arg(getFilePath()),
								 QMessageBox::Ok);
		return ;
	} else {
		QTextStream in(&file);
		plainEditor->setPlainText(in.readAll());
		file.close();
	}
}

bool CodeEditor::fileSave()			// 保存缓存区到文件。
{
	QFile file(getFilePath());
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::StandardButton ret;
		ret = QMessageBox::question(this, tr("无法保存该文件"),
									tr("是否选择另存为该文件: %1",
									   "点击【OK】另存为， 点击【Cancel】放弃该文件的修改").arg(getFilePath()),
									QMessageBox::Ok | QMessageBox::Cancel,
									QMessageBox::Ok);
		
		switch (ret) {
		case QMessageBox::Ok:
			while(!fileSaveAs());
			return true;
		case QMessageBox::Cancel:
			return false;
		default:
			return false;
		}
	}
	
	QTextStream out(&file);
	out << plainEditor->toPlainText();
	file.close();
	
	plainEditor->document()->setModified(false);
	return true;	
} // fileSave

bool CodeEditor::fileSaveAll()
{
	emit fileSaveAllSignal();
	return true;
}

bool CodeEditor::fileSaveAs()			// 另存为缓存区。
{
	QString filePath =
		QFileDialog::getSaveFileName(this,
									 tr("另存为..."),
									 getFilePath(),
									 tr("All Files (*)"));
	
    if (filePath.isEmpty()) {
		return false;
	}

	// 检查是否有父母录的写权限
	QFileInfo fileInfo(filePath);
	QFileInfo parentDir(fileInfo.absolutePath());
	if (!parentDir.exists()){
		QMessageBox::information(this, tr("该文件不存在"),
								 tr("%1").arg(parentDir.absoluteFilePath()),
								 QMessageBox::Ok);
		return false;
	}
	
	QMessageBox::StandardButton ret;
	if (!parentDir.isWritable()){
		ret = QMessageBox::warning(this, tr("权限不够"),
								   tr("无法在此创建文件: %1").arg(parentDir.absoluteFilePath()),
								   QMessageBox::Ok | QMessageBox::Discard,
								   QMessageBox::Ok);
		if (ret == QMessageBox::Ok)
			return false;
		else
			return true;
	}

    QFile file(filePath);
    QTextStream out(&file);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		ret = QMessageBox::warning(this, tr("写入文件失败"),
								   tr("%1").arg(filePath),
								   QMessageBox::Ok);
		if (ret == QMessageBox::Ok)
			return false;
		else
			return true;
	}
    out << plainEditor->toPlainText(); // 写入文件
	file.close();
	
	QString oldFilePath = getFilePath(); // 暂存修改前的文件名
    setFilePath(filePath); // 更改文件名
    plainEditor->document()->setModified(false);
	emit renamedSignal(oldFilePath, this);
    return true;
} // fileSaveAs

void CodeEditor::format()
{

}
// protected
void CodeEditor::setUi()
{
	action_New = new QAction(this);
	action_New->setObjectName(QString::fromUtf8("action_New"));
	action_New->setToolTip("新建文件");
	QIcon icon_New;
	icon_New.addFile(QString::fromUtf8(":/images/file_new.png"),
					 QSize(24, 24), QIcon::Normal, QIcon::On);
	action_New->setIcon(icon_New);
	
	action_Open = new QAction(this);
	action_Open->setObjectName(QString::fromUtf8("action_Open"));
	action_Open->setToolTip("打开文件");
	QIcon icon_Open;
	icon_Open.addFile(QString::fromUtf8(":/images/file_open.png"),
					 QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Open->setIcon(icon_Open);
	
	action_Save = new QAction(this);
	action_Save->setObjectName(QString::fromUtf8("action_Save"));
	action_Save->setToolTip("保存");
	QIcon icon_Save;
	icon_Save.addFile(QString::fromUtf8(":/images/file_save.png"),
					  QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Save->setIcon(icon_Save);
	

	action_Save_All = new QAction(this);
	action_Save_All->setObjectName(QString::fromUtf8("action_Save_All"));
	action_Save_All->setToolTip("保存所有");
	QIcon icon_Save_All;
	icon_Save_All.addFile(QString::fromUtf8(":/images/file_save_all.png"),
						  QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Save_All->setIcon(icon_Save_All);

	action_Save_As = new QAction(this);
	action_Save_As->setToolTip("另存为");
	QIcon icon_Save_As;
	icon_Save_As.addFile(QString::fromUtf8(":/images/file_save_as.png"),
						 QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Save_As->setIcon(icon_Save_As);

	action_Undo = new QAction(this);
	action_Undo->setObjectName(QString::fromUtf8("action_Undo"));
	action_Undo->setToolTip("Undo");
	QIcon icon_Undo;
	icon_Undo.addFile(QString::fromUtf8(":/images/edit_undo.png"),
					  QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Undo->setIcon(icon_Undo);
	
	action_Redo = new QAction(this);
	action_Redo->setObjectName(QString::fromUtf8("action_Redo"));
	action_Redo->setToolTip("Redo");
	QIcon icon_Redo;
	icon_Redo.addFile(QString::fromUtf8(":/images/edit_redo.png"),
					  QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Redo->setIcon(icon_Redo);
	
	action_Cut = new QAction(this);
	action_Cut->setObjectName(QString::fromUtf8("action_Cut"));
	action_Cut->setToolTip("剪切");
	QIcon icon_Cut;
	icon_Cut.addFile(QString::fromUtf8(":/images/edit_cut.png"),
					 QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Cut->setIcon(icon_Cut);

	
	action_Copy = new QAction(this);
	action_Copy->setObjectName(QString::fromUtf8("action_Copy"));
	action_Copy->setToolTip("复制");
	QIcon icon_Copy;
	icon_Copy.addFile(QString::fromUtf8(":/images/edit_copy.png"),
					  QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Copy->setIcon(icon_Copy);

	
	action_Paste = new QAction(this);
	action_Paste->setObjectName(QString::fromUtf8("action_Paste"));
	action_Paste->setToolTip("粘贴");
	QIcon icon_Paste;
	icon_Paste.addFile(QString::fromUtf8(":/images/edit_paste.png"),
					   QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Paste->setIcon(icon_Paste);

	action_Format = new QAction(this);
	action_Format->setObjectName(QString::fromUtf8("action_Format"));
	action_Format->setToolTip("格式化代码");
	QIcon icon_Format;
	icon_Format.addFile(QString::fromUtf8(":/images/edit_format.png"),
						QSize(24, 24), QIcon::Normal, QIcon::On);
	action_Format->setIcon(icon_Format);

	toolBar = new QToolBar();
	toolBar->setObjectName(QString::fromUtf8("toolBar"));
	toolBar->setContentsMargins(0, 0, 0, 0);
	toolBar->setFixedHeight(28);

	toolBar->addAction(action_New);
	toolBar->addAction(action_Open);
	toolBar->addAction(action_Save);
	toolBar->addAction(action_Save_All);
	toolBar->addAction(action_Save_As);
	toolBar->addAction(action_Undo);
	toolBar->addAction(action_Redo);
	toolBar->addAction(action_Cut);
	toolBar->addAction(action_Copy);
	toolBar->addAction(action_Paste);
	toolBar->addAction(action_Format);

    frame = new QFrame(this);
	// frame->setContentsMargins(0, 0, 0, 0);
	
    hbox = new QHBoxLayout(frame);
	hbox->setObjectName(QString::fromUtf8("vbox"));
	hbox->setContentsMargins(0, 0, 0, 0);
	hbox->setSpacing(0);
	
    plainEditor = new PlainEditor(this);
	plainEditor->setObjectName(QString::fromUtf8("plainEditor"));
	hbox->addWidget(plainEditor);

    vbox = new QVBoxLayout(this);
	vbox->setObjectName(QString::fromUtf8("hbox"));
	vbox->setContentsMargins(0, 0, 0, 0);
	vbox->setSpacing(0);
    vbox->addWidget(toolBar);
	vbox->addWidget(frame);
} // setUi

void CodeEditor::setConnect() {
	connect(plainEditor->document(), SIGNAL(modificationChanged(bool)), this,
			SLOT(modificationChanged(bool)));
	connect(plainEditor->document(), SIGNAL(modificationChanged(bool)), action_Save,
			SLOT(setEnabled(bool)));

	connect(action_New, SIGNAL(triggered()), this, SLOT(fileNew()));
	connect(action_Open, SIGNAL(triggered()), this, SLOT(fileOpen()));
	connect(action_Save, SIGNAL(triggered()), this, SLOT(fileSave()));
	connect(action_Save_All, SIGNAL(triggered()), this, SLOT(fileSaveAll()));
	connect(action_Save_As, SIGNAL(triggered()), this, SLOT(fileSaveAs()));

	connect(plainEditor->document(), SIGNAL(undoAvailable(bool)),
            action_Undo, SLOT(setEnabled(bool)));
    connect(plainEditor->document(), SIGNAL(redoAvailable(bool)),
            action_Redo, SLOT(setEnabled(bool)));

    action_Save->setEnabled(plainEditor->document()->isModified()); 
    action_Undo->setEnabled(plainEditor->document()->isUndoAvailable());
    action_Redo->setEnabled(plainEditor->document()->isRedoAvailable());

    connect(action_Undo, SIGNAL(triggered()), plainEditor, SLOT(undo()));
    connect(action_Redo, SIGNAL(triggered()), plainEditor, SLOT(redo())); 

    action_Cut->setEnabled(false); 
    action_Copy->setEnabled(false); 

    connect(action_Cut, SIGNAL(triggered()), plainEditor, SLOT(cut()));
    connect(action_Copy, SIGNAL(triggered()), plainEditor, SLOT(copy()));
    connect(action_Paste, SIGNAL(triggered()), plainEditor, SLOT(paste()));

    connect(plainEditor, SIGNAL(copyAvailable(bool)), action_Cut, SLOT(setEnabled(bool)));
    connect(plainEditor, SIGNAL(copyAvailable(bool)), action_Copy, SLOT(setEnabled(bool)));

	connect(action_Format, SIGNAL(triggered()), this, SLOT(format()));
}
