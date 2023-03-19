#ifndef DYNAMICQDIALOG_H
#define DYNAMICQDIALOG_H
#include <QDialog>
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

class DynamicQDialog : public QDialog
{
   Q_OBJECT

public:
   DynamicQDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
   ~DynamicQDialog();

public slots:
   void checkWidth(const QString &text);
   void checkHeight(const QString &text);
   int getWidth();
   int getHeight();
   void sl_accept();
   void sl_cancel();

private:
   int width;
   int height;

   int widthFieldStatus; 
   int heightFieldStatus;
   QLabel *labelText;
   QLabel *labelWidth;
   QLabel *labelHeight;
   QLineEdit *lineEditWidth;
   QLineEdit *lineEditHeight;
   QPushButton *okButton;
   QPushButton *closeButton;

signals:
};
#endif // DYNAMICQDIALOG_H

