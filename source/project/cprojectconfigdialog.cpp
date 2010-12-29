#include <QtGui/QDialogButtonBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

#include "cprojectconfigdialog.h"

CProjectConfigDialog::CProjectConfigDialog(CProject *project, QWidget *parent)
	: QDialog(parent)
{
	cproject = project;
	setUi();
}

void CProjectConfigDialog::ok()
{
	basicPage->save();
	includePage->save();
	libraryPage->save();
	mainSourceDependPage->save();
	accept();
}

void CProjectConfigDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

void CProjectConfigDialog::setUi()
{
    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::ListMode);
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(128);
    contentsWidget->setSpacing(4);

	QListWidgetItem *basicButton = new QListWidgetItem(contentsWidget);
    basicButton->setText(tr("基本设置"));
    basicButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *includeButton = new QListWidgetItem(contentsWidget);
    includeButton->setText(tr("Include"));
    includeButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *libraryButton = new QListWidgetItem(contentsWidget);
    libraryButton->setText(tr("Library"));
    libraryButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);


    QListWidgetItem *mainSourceDependButton = new QListWidgetItem(contentsWidget);
    mainSourceDependButton->setText("主文件依赖");
    mainSourceDependButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	
    connect(contentsWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

	basicPage = new BasicPage(cproject);
	includePage = new IncludePage(cproject);
	libraryPage = new LibraryPage(cproject);
	mainSourceDependPage = new MainSourceDependPage(cproject);
    pagesWidget = new QStackedWidget;
	pagesWidget->addWidget(basicPage);
    pagesWidget->addWidget(includePage);
    pagesWidget->addWidget(libraryPage);
    pagesWidget->addWidget(mainSourceDependPage);

	
    contentsWidget->setCurrentRow(0);

    QHBoxLayout *hbox = new QHBoxLayout;
	hbox->setContentsMargins(0, 0, 0, 0);
    hbox->addWidget(contentsWidget);
    hbox->addWidget(pagesWidget, 1);

	QDialogButtonBox *buttonBox =
		new QDialogButtonBox(QDialogButtonBox::Ok
							 |QDialogButtonBox::Cancel);
	buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(ok()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addLayout(hbox);
    mainLayout->addSpacing(4);
	mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Project Config Dialog"));
}
