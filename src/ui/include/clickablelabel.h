#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent);
    explicit ClickableLabel(const QString &text = "", QWidget *parent = nullptr);

signals:
    void clicked();

public slots:

protected:
    void mousePressEvent(QMouseEvent *);
};

#endif // CLICKABLELABEL_H
