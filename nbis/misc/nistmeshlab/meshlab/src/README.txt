MeshLab (NIST modified version)
===============================

Author: Kenneth Ko
Date:   06/09/2011


Description
-----------
The MeshLab (NIST modified version) application is a PointCloud 
visualization tool that reads an input file with NIST-specific
PointCould format (*.XYZ) and RAW fingerprint image (*.RAW) to 
provide the user with pointcloud editing, filtering and rendering
capabilities. 

Note: This tool is based on MeshLab, which is available in its 
original form at:
http://meshlab.sourceforge.net/


Installation
------------
Please see INSTALL.txt for detailed information on how to 
build MeshLab (NIST modified version).


Input File Support in MeshLab (NIST modified version)
-----------------------------------------------------
The MeshLab (NIST modified version) is modifed to support NIST-
specific PointCould format (*.XYZ) and RAW fingerprint image (*.RAW).

	How to Create an XYZ Input File
	-------------------------------
	For detailed information on creating an *.XYZ fingerprint 
	image from a *.RAW fingerprint image and to view the 
	NIST-specific 3D PointCloud Image Format Specification,
	please see util/README.txt

	File naming Requirements For RAW Input Files
	--------------------------------------------
	In order for MeshLab (NIST modified version) to process 
	a RAW fingerprint image, the filename MUST follow the 
	format as described below:

	<RAW fingerprint image>_<width>_<height>.raw

	(NOTE: Please see util/example for fingerprint image
	examples for both the *.XYZ and *.RAW formats.)
