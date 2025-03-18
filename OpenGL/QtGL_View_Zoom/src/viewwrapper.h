#ifndef VIEWWRAPPER_H
#define VIEWWRAPPER_H

#include <QWidget>
#include "marchview.h"

class ViewWrapper : public QWidget
{
    Q_OBJECT
public:
    explicit ViewWrapper(QWidget *parent = nullptr);
    ~ViewWrapper();

private:
    MarchView *m_marchView;
};

#endif // VIEWWRAPPER_H