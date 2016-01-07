#include "layerpanel.h"
#include "ui_layerpanel.h"

#include <QStyle>

#include "layer.h"
#include "mainwindow.h"

LayerPanel::LayerPanel(MainWindow* parent, QList<Layer *>* layers) :
    QDockWidget(parent),
    ui(new Ui::LayerPanel),
    _layers(layers)
{
    ui->setupUi(this);
    _listWidget = new ListWidget(this);
    ui->horizontalLayout->addWidget(_listWidget);
    ui->layerLabel->setVisible(false);
    connect(_listWidget, &ListWidget::orderChanged, [=](int oldIndex, int newIndex) {
        emit reorderEvent(oldIndex, newIndex);
    });
    connect(_listWidget, &ListWidget::itemChanged, [=](QListWidgetItem* item) {
        if(item->checkState() == Qt::Checked) {
            item->data(1).value<Layer*>()->setVisible(true);
        }
        if(item->checkState() == Qt::Unchecked) {
            item->data(1).value<Layer*>()->setVisible(false);
        }
    });
    connect(_listWidget, &ListWidget::currentItemChanged, [=](QListWidgetItem * current, QListWidgetItem * previous) {
        Q_UNUSED(previous)
        ui->layerLabel->setVisible(true);
        ui->layerLabel->setText(current->data(1).value<Layer*>()->getInformation());
    });
}

LayerPanel::~LayerPanel()
{
    delete ui;
}

void LayerPanel::addLayer(Layer *layer)
{
    // adds a layer to the layer panel
    QListWidgetItem* item = new QListWidgetItem(layer->getName(), _listWidget);
    item->setData(1, QVariant::fromValue<Layer*>(layer));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
    item->setCheckState(Qt::Unchecked); // AND initialize check state
}

