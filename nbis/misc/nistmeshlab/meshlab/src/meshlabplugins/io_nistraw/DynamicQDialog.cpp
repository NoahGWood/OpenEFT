#include <QtGui>
#include "DynamicQDialog.h"

DynamicQDialog::DynamicQDialog(QWidget* parent, Qt::WindowFlags flags): QDialog(parent, flags)
{
   width = 0;
   height = 0;
   widthFieldStatus = 0;
   heightFieldStatus = 0;

   labelText = new QLabel(tr("Please input the RAW image dimension."));

   labelWidth = new QLabel(tr("&Width:"));
   lineEditWidth = new QLineEdit;
   labelWidth->setBuddy(lineEditWidth);

   labelHeight = new QLabel(tr("&Height:"));
   lineEditHeight = new QLineEdit;
   labelHeight->setBuddy(lineEditHeight);

   okButton = new QPushButton(tr("&OK"));
   okButton->setDefault(true);
   okButton->setEnabled(false);

   closeButton = new QPushButton(tr("Close"));

   connect(lineEditWidth, SIGNAL(textChanged(const QString &)), this, SLOT(checkWidth(const QString &)));
   connect(lineEditHeight, SIGNAL(textChanged(const QString &)), this, SLOT(checkHeight(const QString &)));
   connect(okButton, SIGNAL(clicked()), this, SLOT(sl_accept()));
   connect(closeButton, SIGNAL(clicked()), this, SLOT(sl_cancel()));

   QHBoxLayout *topLeftLayout1 = new QHBoxLayout;
   topLeftLayout1->addWidget(labelText);

   QHBoxLayout *topLeftLayout2 = new QHBoxLayout;
   topLeftLayout2->addWidget(labelWidth);
   topLeftLayout2->addWidget(lineEditWidth);

   QHBoxLayout *topLeftLayout3 = new QHBoxLayout;
   topLeftLayout3->addWidget(labelHeight);
   topLeftLayout3->addWidget(lineEditHeight);

   QHBoxLayout *topLeftLayout4 = new QHBoxLayout;
   topLeftLayout4->addWidget(okButton);
   topLeftLayout4->addWidget(closeButton);
   
   QVBoxLayout *leftLayout = new QVBoxLayout;
   leftLayout->addLayout(topLeftLayout1);
   leftLayout->addLayout(topLeftLayout2);
   leftLayout->addLayout(topLeftLayout3);
   leftLayout->addLayout(topLeftLayout4);
   

   QHBoxLayout *mainLayout = new QHBoxLayout;
   mainLayout->addLayout(leftLayout);
   setLayout(mainLayout);

   setWindowTitle(tr("Get Width and Height"));
   setFixedHeight(sizeHint().height());

}

DynamicQDialog::~DynamicQDialog(){}

void DynamicQDialog::checkWidth(const QString &text)
{
   widthFieldStatus = 1;

   if (widthFieldStatus == 1 && heightFieldStatus == 1)
   {
      okButton->setEnabled(!text.isEmpty());      
   }
}

void DynamicQDialog::checkHeight(const QString &text)
{
   heightFieldStatus = 1;

   if (widthFieldStatus == 1 && heightFieldStatus == 1)
   {
      okButton->setEnabled(!text.isEmpty());
   }
}

int DynamicQDialog::getWidth()
{
  return width;
}

int DynamicQDialog::getHeight()
{
  return height;
}

void DynamicQDialog::sl_accept()
{
   bool ok;
   
   QString textWidth = lineEditWidth->text();
   QString textHeight = lineEditHeight->text();
   
   width = textWidth.toInt(&ok, 10);
   height = textHeight.toInt(&ok, 10);
   
   close();
}

void DynamicQDialog::sl_cancel()
{
   close();
}

