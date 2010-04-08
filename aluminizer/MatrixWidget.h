#pragma once

#include <QObject>
#include <QLabel>
#include <QAction>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QTableWidget>

#include <tnt.h>
#include <jama_lu.h>

#include "my_matrix.h"

//typedef std::vector<std::vector<double> > matrix_t;

//! Add some extra features to QDoubleSpinBox
/* Page up/down: increase/decrease in 10 x increment steps
 */

/*
   class CustomDSB : public QDoubleSpinBox
   {
   Q_OBJECT
   public:

   protected:
   void keyPressEvent(QKeyEvent *);
   };
 */
typedef my_matrix matrix_t;

void CopyArray2D(const matrix_t& src, TNT::Array2D<double>* dst);

class MatrixWidget : public QTableWidget
{
Q_OBJECT

public:
MatrixWidget(QWidget* parent = 0, const std::string& name = "", unsigned nR = 0, unsigned nC = 0, const std::string& sIni = "");
~MatrixWidget();

void setName(const std::string&);

void setRowLabel(unsigned r, const std::string&);
void setColLabel(unsigned c, const std::string&);
void updateLabels();

void setValue(const matrix_t&);
const matrix_t& getValue();
double getValue(unsigned r, unsigned c);

void setReadOnly(bool b);

signals:
void valueChanged();

protected slots:
void slot_itemActivated(QTableWidgetItem*);
void slot_cellEntered(int, int);
void slot_valueChanged();
void slot_lock();
void slot_unlock();

protected:
QDoubleSpinBox* input(unsigned i, unsigned j);
unsigned nR, nC;

std::string name;

matrix_t M;

//	QLabel label;

std::vector<std::string> rLabels, cLabels;
//   std::vector<std::vector<QDoubleSpinBox*> > inputs;
QAction action_lock, action_unlock;
};
