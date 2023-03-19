NIST PointCloud Visualization Bundle  
====================================

Author: Kenneth Ko
Date:   06/09/2011


Description
-----------
This NIST PointCloud visualization bundle is designed to allow a
user to visualize a fingerprint image using the NIST-specific 
PointCould file format (*.XYZ) and also to convert a RAW fingerprint
image (*.RAW) into the NIST-specific PointCloud file format.

This NIST PointCloud visualization bundle contains two applications:
1) MeshLab (NIST modified version)
2) PointCloud Creator

The MeshLab (NIST modified version) application is a PointCloud 
visualization tool that reads an input file with NIST-specific PointCould 
format and provides the user with pointcloud editing, filtering and 
rendering capabilities. 

The PointCloud Creator application allows the user to convert a RAW 
fingerprint image to the NIST-specific PointCloud file format. 

For the MeshLab build procedure, please see meshlab/src/README.txt.
For PointCloud Creator build procedure, please see util/README.txt.


NOTE
-----
This tool is based on MeshLab, which is available in its original form at:
http://meshlab.sourceforge.net/
