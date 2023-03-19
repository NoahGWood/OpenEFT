/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2007                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
/****************************************************************************
  History

 $Log: FilterCreateIso.h,v $
 Revision 1.1  2007/12/02 07:57:48  cignoni
 Added the new sample filter plugin that removes border faces


 *****************************************************************************/
#ifndef FILTERCREATEISO_PLUGIN_H
#define FILTERCREATEISO_PLUGIN_H

#include <QObject>
#include <common/interfaces.h>


class FilterCreateIso : public QObject, public MeshFilterInterface
{
	Q_OBJECT
	Q_INTERFACES(MeshFilterInterface)

		public:
	/* naming convention :
		 - FP -> Filter Plugin
		 - name of the plugin separated by _
	*/
	enum { 
    FP_CREATEISO, 
  } ;
	
	/* default values for standard parameters' values of the plugin actions */
	FilterCreateIso();
	~FilterCreateIso();

	virtual QString filterName(FilterIDType filter) const;
	virtual QString filterInfo(FilterIDType filter) const;

  virtual FilterClass getClass(QAction *);
  virtual int getRequirements(QAction *);
    virtual void initParameterSet(QAction *,MeshModel &/*m*/, RichParameterSet & /*parent*/);

    virtual bool applyFilter(QAction *filter, MeshDocument &md, RichParameterSet & /*parent*/, vcg::CallBackPos * cb) ;

	protected:
	
};


#endif
