#ifndef IAbouttWidget_H
#define IAbouttWidget_H

#include "LibExports.h"

#include <QtWidgets/QDialog>

class EXPORT_API IAboutWidget : public QDialog
{
    //Q_OBJECT
public:
    IAboutWidget(QWidget* parent = nullptr);
    virtual ~IAboutWidget();

private:
    virtual void initWidget() = 0;

};

#endif //IAbouttWidget_H
