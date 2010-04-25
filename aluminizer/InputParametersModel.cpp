#include "InputParametersModel.h"

TxtParametersModel::TxtParametersModel(const std::string& sTxtFileName) : 
 TxtParameters(sTxtFileName), 
 QAbstractTableModel() 
{
}


 TxtParametersModel::~TxtParametersModel()
 {
 }


int TxtParametersModel::rowCount(const QModelIndex & m) const
{
/*
Returns the number of rows under the given parent. 
When the parent is valid it means that rowCount is returning the number of children of parent.
Note: When implementing a table based model, rowCount() should return 0 when the parent is valid.
*/
	if(m.isValid())
		return 0;
	else
		return size();
}

int TxtParametersModel::columnCount(const QModelIndex & m) const
{
	if(m.isValid())
		return 0;
	else
		return 4;
}

QVariant TxtParametersModel::data(const QModelIndex &index, int role) const
{
	if(index.isValid() && index.row() < size() && role == Qt::DisplayRole)
	{
		const_iterator cit = begin();

		for(int i=0; i<index.row(); i++)
			cit++;

		if(index.column() == 0)
		{
			std::string s = cit->second.value.c_str();
			return QObject::tr(s.c_str());
		}
		
		if(index.column() == 1)
			return QVariant(asctime(localtime( &(cit->second.tChanged) )));

		if(index.column() == 2)
			return QVariant(cit->second.revision);

		if(index.column() == 3)
			return QVariant(cit->second.was_touched);
	}
	
	return QVariant();
}


QVariant TxtParametersModel::headerData ( int section, Qt::Orientation orientation, int role) const
{
	if(role == Qt::DisplayRole)
	{
		if(orientation == Qt::Horizontal)
		{
			if(section == 0)
				return QObject::tr("Value");

			if(section == 1)
				return QObject::tr("Last changed");

			if(section == 2)
				return QObject::tr("Revision");

			if(section == 3)
				return QObject::tr("Touched");

		}
		else
		{
			if(orientation == Qt::Vertical)
			{
				const_iterator cit = begin();

				for(int i=0; i<section; i++)
					cit++;

				return QObject::tr(cit->first.c_str());
			}
		}
	}

	return QAbstractTableModel::headerData(section, orientation, role);
}
