PVIV 0.8.11 - 05/25/2011
----------------------------------------------

1) Description
	
	Pviv converts photovoltaic IV data acquired with GreenEngineering IVStat into ASCII.
	(More into on the IVStat: http://www.greenmountainengineering.com/ivstat).
	Pviv extracts Voc, Jsc, filling factors, and efficiencies from any IV data in ASCII format.

	XMGrace is required to plot ASCII files.
	XMGrace (freeware) is available from http://plasma-gate.weizmann.ac.il/Grace/ .
	Curve plotting can be turned off (from the 'settings' menu) if XMGrace is not installed. However
	some features (fitting) may not be available, since they depend on the XMGrace package.

2) Installation
   
    	1. Install xmgrace from http://plasma-gate.weizmann.ac.il/Grace/. 
	
	2. No binaries are provided for MacOSX. Therefore, we strongly recommend to recompile the binary for your architecture and to have a system-wise installation.

	3. Gcc (included in XScale) is the recommended compiler.
	
		a. From the "src" folder, type "make" to compile the binary.

		b. To install the binary, type from the terminal (you might need to be root, or use sudo): "make install"

		c. From the command line, run pviv.
 
		d. pviv accepts the name of the file to be converted as an argument.
		
		e. To uninstall: from the terminal type: "sudo make uninstall"

4) Test file

	Several examples can be found in the folder "example". 

5) License

	This program (source code and binaries) is free software; 
	you can redistribute it and/or modify it under the terms of the
	GNU General Public License as published by the Free Software 
	Foundation, either in version 3 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You can find a complete copy of the GNU General Public License at:
	http://www.gnu.org/licenses/gpl.txt


6) Contact

	For any suggestion, bug, comments: Nicola Ferralis: feranick@hotmail.com
	An updated version of this program can be found at:

	http://electronsoftware.googlepages.com/pviv

