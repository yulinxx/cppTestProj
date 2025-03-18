#include "viewwrapper.h"
#include <QVBoxLayout>

ViewWrapper::ViewWrapper(QWidget *parent) : QWidget(parent)
{
    m_marchView = new MarchView(this);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_marchView);
    setLayout(layout);
}

ViewWrapper::~ViewWrapper()
{
}