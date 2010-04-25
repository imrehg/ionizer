#ifndef INPUT_PARAMETERS_MODEL_H
#define INPUT_PARAMETERS_MODEL_H

#include "InputParameters.h"
#include <QAbstractTableModel>
#include <QModelIndex>
#include <string>

class TxtParametersModel : public TxtParameters, public QAbstractTableModel
{
public:
	TxtParametersModel(const std::string& sTxtFileName);
	virtual ~TxtParametersModel();

	int rowCount(const QModelIndex &) const;
	int columnCount(const QModelIndex &) const;
	QVariant data(const QModelIndex &,int) const;
	QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
};



#endif //INPUT_PARAMETERS_MODEL_H