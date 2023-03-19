/****************************************************************************
* MeshLab                                                           o o     *
* A versatile mesh processing toolbox                             o     o   *
*                                                                _   O  _   *
* Copyright(C) 2005                                                \/)\/    *
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

/****************************************** 
* This file is modified by NIST - 06/2011 *
******************************************/

#include <QApplication>
#include <QMessageBox>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QLocale::setDefault(QLocale::C);
  QCoreApplication::setOrganizationName("VCG");
  QCoreApplication::setApplicationName("MeshLab(NIST Modified Version)");
	
		if(argc>1)	
		{
			QString helpOpt1="-h";
			QString helpOpt2="--help";
			if( (helpOpt1==argv[1]) || (helpOpt2==argv[1]) )
			{
				printf("\n\n"
							 "    MeshLab: an open source mesh processing system\n"
							 "          Paolo Cignoni (and many many others) \n"
							 "              Visual Computing Lab\n"
							 "                  ISTI - CNR \n\n"
							 "usage:\n\n"
							 "    meshlab [meshfile] \n\n"
							 "Look at --- http://meshlab.sourceforge.net/wiki --- for a longer documentation\n\n"
							 );
			exit(-1);
			}
		}
	
  MainWindow window;
  window.showMaximized();

  // This filter is installed to intercept the open events sent directly by the Operative System. 
	FileOpenEater *filterObj=new FileOpenEater();
  filterObj->mainWindow=&window;
	app.installEventFilter(filterObj);
	app.processEvents();
	if(argc>1)	
	{
		QString helpOpt1="-h";
		QString helpOpt2="--help";
		if( (helpOpt1==argv[1]) || (helpOpt2==argv[1]) )
		printf(
			"usage:\n"
			"meshlab <meshfile>\n"
			"Look at http://meshlab.sourceforge.net/wiki\n"
			"for a longer documentation\n"
		);
		window.open(argv[1]);
	}
	else 	if(filterObj->noEvent) window.open();
 
	return app.exec();
}
