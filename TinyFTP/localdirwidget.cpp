#include "localdirwidget.h"
#include "tinyftp.h"

LocalDirWidget::LocalDirWidget(QWidget *parent)
	: QWidget(parent)
{
	parentTinyFtp = reinterpret_cast<TinyFTP*>(parent);

	localDirTreeModel = new DirTreeModel(this);
	localDirTreeModel->setRootPath(QDir::currentPath());

	localDirTreeView = new QTreeView(this);
	localDirTreeView->setModel(localDirTreeModel);
	localDirTreeView->header()->setStretchLastSection(true);
	localDirTreeView->resizeColumnToContents(0);
    localDirTreeView->setAlternatingRowColors(true);
	localDirTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
	localDirTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
	localDirTreeView->setSortingEnabled(true);
	localDirTreeView->sortByColumn(0, Qt::AscendingOrder);
    localDirTreeView->setRootIsDecorated(false);
    localDirTreeView->setItemsExpandable(false);

	localDirFileSystemModel = new QFileSystemModel(this);
	localDirFileSystemModel->setFilter(QDir::AllDirs | QDir::Drives | 
		QDir::NoDot | QDir::NoDotDot | QDir::NoDotAndDotDot);
	QModelIndex index = localDirFileSystemModel->setRootPath(QDir::currentPath());
	
	localDirComboTreeView = new QTreeView(this);
	localDirComboTreeView->setModel(localDirFileSystemModel);
    localDirComboTreeView->resizeColumnToContents(0);
    localDirComboTreeView->hideColumn(1);
    localDirComboTreeView->hideColumn(2);
    localDirComboTreeView->hideColumn(3);
    localDirComboTreeView->setHeaderHidden(true);
    localDirComboTreeView->expand(index);
    localDirComboTreeView->scrollTo(index);
    localDirComboTreeView->setCurrentIndex(index);
    localDirComboTreeView->setItemsExpandable(true);
	
	localDirComboBox = new QComboBox(this);
	localDirComboBox->setModel(localDirFileSystemModel);
	localDirComboBox->setView(localDirComboTreeView);

	dotdotDirToolButton = new QToolButton(this);
	dotdotDirToolButton->setText(tr("�ϼ�Ŀ¼"));
	refreshDirToolButton = new QToolButton(this);
	refreshDirToolButton->setText(tr("ˢ��"));

	localDirStatusBar = new QStatusBar(this);

	QHBoxLayout *topHBoxLayout = new QHBoxLayout;
	topHBoxLayout->addWidget(dotdotDirToolButton);
	topHBoxLayout->addWidget(refreshDirToolButton);
	topHBoxLayout->addWidget(localDirComboBox);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(topHBoxLayout);
	mainLayout->addWidget(localDirTreeView);
	mainLayout->addWidget(localDirStatusBar);
	setLayout(mainLayout);

	//*******************************
	// context menu
	contextMenu = new QMenu(this);
	uploadAction = new QAction(tr("�ϴ�"), this);
	queueAction = new QAction(tr("����"), this);
    refreshAction = new QAction(tr("ˢ��"), this);
	editAction = new QAction(tr("�༭"), this);
	readAction = new QAction(tr("�鿴"), this);
	execAction = new QAction(tr("ִ��"), this);
	delAction = new QAction(tr("ɾ��"), this);
	renameAction = new QAction(tr("������"), this);
	propertyAction = new QAction(tr("����"), this);
	contextMenu->addAction(uploadAction);
	contextMenu->addAction(queueAction);
    contextMenu->addAction(refreshAction);
	sendToAction = contextMenu->addMenu(new QMenu(tr("���͵�"), this));
	contextMenu->addAction(editAction);
	contextMenu->addAction(readAction);
	contextMenu->addAction(execAction);
	contextMenu->addAction(delAction);
	contextMenu->addAction(renameAction);
	contextMenu->addAction(propertyAction);

    //*******************************
    // �ؼ� �ź� & ��
	connect(localDirTreeView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(setRootIndex(const QModelIndex &)));
	/*connect(localDirComboTreeView, SIGNAL())*/
    connect(localDirComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(currentIndexChanged(const QString &)));
	connect(localDirTreeView, SIGNAL(pressed(const QModelIndex &)), this, SLOT(showContextMenu(const QModelIndex &)));
    connect(refreshDirToolButton, SIGNAL(clicked()), this, SLOT(refresh()));

	//*******************************
	// context menu slots & signals
	connect(uploadAction, SIGNAL(triggered()), this, SLOT(upload()));
	connect(queueAction, SIGNAL(triggered()), this, SLOT(queue()));
    connect(refreshAction, SIGNAL(triggered()), this, SLOT(refresh()));
	connect(editAction, SIGNAL(triggered()), this, SLOT(edit()));
	connect(readAction, SIGNAL(triggered()), this, SLOT(read()));
	connect(execAction, SIGNAL(triggered()), this, SLOT(exec()));
	connect(delAction, SIGNAL(triggered()), this, SLOT(del()));
	connect(renameAction, SIGNAL(triggered()), this, SLOT(rename()));
	connect(propertyAction, SIGNAL(triggered()), this, SLOT(property()));
}

LocalDirWidget::~LocalDirWidget()
{
}

QDir LocalDirWidget::currentDir(bool *ok/* = 0*/) const
{
	return localDirTreeModel->currentDir(ok);
}

QString LocalDirWidget::currentDirPath() const
{
	return localDirTreeModel->currentDirPath();
}

// void LocalDirWidget::contextMenuEvent(QContextMenuEvent *event)
// {
// 	QModelIndex index = localDirTreeView->indexAt(QCursor::pos());
// 	Node *node = static_cast<Node*>(index.internalPointer());
// 	if (!index.isValid()) {
// 		return ;
// 	}
// 	//*******************************
// 	// tasks:
// 	//	1. �ϴ�
// 	//	2. ����
// 	//	3. ���͵���QMenu��
// 	//	4. �༭
// 	//	5. �鿴
// 	//	6. �ƶ�
// 	//	7. ִ��
// 	//	8. ɾ��
// 	//	9. ������
// 	//	10. ����
// 	contextMenu->exec(QCursor::pos());
// }

void LocalDirWidget::setRootIndex(const QModelIndex &index)
{
	if (!index.isValid()) {
		return ;
	}
	Node *node = static_cast<Node*>(index.internalPointer());
	if (node->isDir) {
        QString path = node->path;
		localDirTreeModel->setRootIndex(index);
		localDirTreeView->resizeColumnToContents(0);
		
        //*******************************
        // ����Ĵ���û��Ч������֪Ϊ��
        QModelIndex curIndex = localDirFileSystemModel->index(path);
        localDirComboTreeView->collapseAll();
        localDirComboTreeView->expand(curIndex);
        localDirComboTreeView->reset();
	}
}

void LocalDirWidget::currentIndexChanged(const QString &text)
{
    QModelIndex curIndex = localDirComboTreeView->currentIndex();
    localDirComboTreeView->collapseAll();
    localDirComboTreeView->expand(curIndex);
    localDirComboTreeView->reset();
/*    localDirComboTreeView->scrollTo(curIndex);*/

    localDirTreeModel->setRootPath(localDirFileSystemModel->filePath(curIndex));
    localDirTreeView->resizeColumnToContents(0);
}

void LocalDirWidget::showContextMenu(const QModelIndex &index)
{
	if (QApplication::mouseButtons() == Qt::RightButton) {
		//*******************************
		// Ĭ�� ʹ�� ���в˵���
		QList<QAction*> actions = contextMenu->actions();
		foreach (QAction* action, actions)
			action->setEnabled(true);

		//*******************************
		// ���� ���͵� �˵�
		sendToAction->menu()->clear();
		TabWidget *remoteDirTabWidget = parentTinyFtp->remoteDirTabWidget;
		int count = remoteDirTabWidget->count();
		for (int i = 0; i < count; i++) {
			RemoteDirWidget *w = static_cast<RemoteDirWidget*>(remoteDirTabWidget->widget(i));
			if (w->isConnected()) {
				QAction *action = new QAction(remoteDirTabWidget->tabText(i), this);
				sendToAction->menu()->addAction(action);
				/*connect(action, SIGNAL(triggered()), this, SLOT(uploadFile()));*/
			}
		}

		//*******************************
		// ���� ���������Ĳ˵��� ʹ�� ״̬
		if (!sendToAction->menu()->actions().count()) {
			sendToAction->setEnabled(false);
		}
		Node *node = static_cast<Node*>(index.internalPointer());
		QFileInfo fileInfo(node->path);
		if (fileInfo.isDir() && node->fileName == tr("..")) {
			foreach (QAction* action, actions)
				action->setEnabled(false);
		} else if (true/*fileInfo.isFile()*/) {
			if (!fileInfo.isWritable()) {
				editAction->setEnabled(false);
				execAction->setEnabled(false);
				delAction->setEnabled(false);
				renameAction->setEnabled(false);
			}
			if (!fileInfo.isReadable()) {
				readAction->setEnabled(false);
				uploadAction->setEnabled(false);
				queueAction->setEnabled(false);
			}
			if (!fileInfo.isExecutable()) {
				execAction->setEnabled(false);
			}
			if (!static_cast<RemoteDirWidget*>(
				remoteDirTabWidget->currentWidget())->isConnected()) {
					uploadAction->setEnabled(false);
			}
		}

		contextMenu->exec(QCursor::pos());
	}
}

void LocalDirWidget::upload()
{

}

void LocalDirWidget::queue()
{

}

void LocalDirWidget::edit()
{

}

void LocalDirWidget::read()
{

}

void LocalDirWidget::exec()
{

}

void LocalDirWidget::del()
{

}

void LocalDirWidget::rename()
{

}

void LocalDirWidget::property()
{

}

void LocalDirWidget::reset()
{
    localDirTreeModel->setRootPath(currentDirPath());
    localDirTreeView->resizeColumnToContents(0);
}

void LocalDirWidget::refresh()
{
    localDirTreeModel->setRootPath(currentDirPath());
    localDirTreeView->resizeColumnToContents(0);
}