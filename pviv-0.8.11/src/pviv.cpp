//*****************************************************************************
//                                                         
//		PV-IV						 
//														 
//		v. 0.8.11
//
//		2011 - Nicola Ferralis 					 
//                                                        
//		PV-IV photovoltaic data analisys
//		(Extract data from Green Mountain IV-Stat)
//
//		This program (source code and binaries) is free software; 
//		you can redistribute it and/or modify it under the terms of the
//		GNU General Public License as published by the Free Software 
//		Foundation, in version 3 of the License.
//
//		This program is distributed in the hope that it will be useful,
//		but WITHOUT ANY WARRANTY; without even the implied warranty of
//		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//		GNU General Public License for more details.
//
//		You can find a complete copy of the GNU General Public License at:
//		http://www.gnu.org/licenses/gpl.txt
//												             
//**********************************************************************************
/* To do
1. fix boundaries for Voc and Isc.
2. Add possibility to select either internal data or that calculated from the measured data
3. General cleanup.
*/

//Note for Windows:
// In order to be able to compile, the character settings needs to be set NOT in unicode.
// Project menu -> project properties -> Configuration Properties -> General -> Character Set -> Not Set

//////////////
#if     _MSC_VER
// Use these for MSVC
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <windows.h>
#include <ctime>
#include <process.h>


#define snprintf sprintf_s


using namespace std; 

#define popen _popen 
#define pclose     _pclose   
#define MSini 1   // 1: save config file in C:/ (MS32 only)    0: save it in the same folder 
#include "direct.h"			

#else
// Use these for gcc
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <math.h>
#include <stdio.h>  
#include <errno.h>
#include <grace_np.h>
#include <time.h>
#define MSini 0
using namespace std; 
#endif

#define GPFIFO "./gpio" 

#define PATH_MAX 200
#define CMAXI 200

#if     _MSC_VER
	#if MSini ==1					
	char inipath[]="C:/";					
	char mainpath[200];	
	#endif
#else
char mainpath[PATH_MAX+1];
#endif

int const Max=1000000;

int i,j,npoints,vplot, type;
int pl=0;

int vplot0=0;
double length0=37; // in mm length cell.
double width0=33; // in mm width cell.
double Pin0=100; //Power density in mW/cm^2

int flags=0;
int flagr=0;
int filetype=0;

double Voc, Ioc, Vsc, Isc, Jsc, Pmax, Vmp, Imp, FF, nu, area;
double V[Max], I[Max], P[Max];
double length, width, Pin;
string sample, notes, configuration;

char version[]="0.8.11 - 20110525";
char developer[] = "Nicola Ferralis - feranick@hotmail.com";

#if     _MSC_VER
char ini[]="pviv.cfg";
#else
char ini[]=".pviv";
#endif

char summary[CMAXI];

typedef struct coord{
	double x;
	double y;
	} coord;

typedef struct mcoord{
	float x[Max];
	float y[Max];
	} mcoord;

int operate(char *namein, int s);
int parser(char *namein);

coord Maximum();
coord linear(double x1, double x2, double y1, double y2, double x0, bool t);

#if     _MSC_VER
#else
void IniPlot(char* name);
void Plot(char* name);
void PlotSettings();
void ClosePlot();
void my_error_function(const char *msg);

#endif
void PreferencePanel();

int ReadKey();
float ReadKeyF();
double ReadKeyD();

double extractmeasurements(string text, int a);
string findpath();
string nameonly(char*namein);

#if     _MSC_VER
BOOL get_module_directory(TCHAR *obuf, size_t osize);
#endif


int main(int argc, char *argv[])

{ 	//setup preference file

	#if     _MSC_VER
		#if MSini ==1
		_chdir(_getcwd(mainpath, 200));  
		_chdir(inipath);					
		#endif
	#else
	int rc=0;
	char *pt;	
	pt=getcwd(mainpath, PATH_MAX+1);	
	rc = chdir(getenv("HOME"));	
	#endif

	#if     _MSC_VER
	ifstream	infile(ini);   //MSVC
	#else
	ifstream	infile(ini);    		  //gcc
	#endif
	if(!infile)			// the first time the program runs, it saves the settings 
		{ofstream outfile2(ini);
		outfile2<<length0<<"\n"<<width0<<"\n"<<Pin0<<"\n"<<vplot0<<"\n"; 
		length=length0;
		width=width0;
		Pin=Pin0;
		vplot=vplot0;
		outfile2.close();
		}

	infile>>length>>width>>Pin>>vplot;   // if the config file exists, it reads the settings from it. 
	
	infile.close();

	#if     _MSC_VER
		#if MSini ==1
		_chdir(mainpath);			
		#endif
	#else
		rc = chdir(mainpath);
	#endif	
	
	/////////////////////////////
	// Setup summary file
	/////////////////////////////
	time_t rawtime;
 	struct tm * timeinfo;
 	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	char *today;
	today=(char *)malloc(sizeof(char[CMAXI]));
	strftime (today,CMAXI,"%Y-%m-%d",timeinfo);
	snprintf(summary, CMAXI, "%sSummary_%s.csv", findpath().c_str(), today);
	
	free(today);
	ifstream infile2(summary);

	if(!infile2)
		{	
		ofstream out(summary);
		out<<"\"File\",\"Sample\",\"Configuration\",\"Notes\",\"Voc(V)\",\"Isc(A)\",\"Jsc(A/cm^2)\",\"Pmax(W)\",\"Vmaxp(V)\",\"Imaxp(A)\",\"FF\",\"Efficiency(%)\",\"Area (cm^2)\"\n";		
		out.close();
		}
	infile2.close();
	//////////////////////////////////////////////////////////////////////////////

	filetype=2;
	
	if(argc<2)
		{	
		cout<<"\n_________________________________________________\n";
		cout<<"PV-IV Photovoltaic data analysis\n";
		cout<<"\nPress: \n0) Exit\t\t\t2) Preferences\n1) Convert\t\t3) About\n";
		type=ReadKey();
		}	

	if(argc>=2)
	{	for(int i=1; i<argc; i++)
			{parser(argv[i]);	
			if(flagr==0)
				{operate(argv[i], 1);}	
			}
		return 0;
		//return main(0,0); 
		}


	if((type!=1) & (type!=2) & (type!=3) & (type!=4))
		{cout<<"Bye Bye\n";
		return 0;}	

	if(type==3)
		{cout<<"\n PV-IV  Photovoltaic data analysis\n  v. "<<version;
		cout<<"\n  Extract and analize data acquired with Green Mountain IV-Stat\n";
		cout<<"\n  Suggestions and bugs:  "<<developer<<"\n";
		cout<<"  An updated version of this program can be found at:\n    http://electronsoftware.googlepages.com/ \n";
		cout<<"\n  XmGrace (version 5.1.x or higher) is required to plot ASCII files.\n";
		cout<<"  XmGrace (freeware) can be downloaded at \n    http://plasma-gate.weizmann.ac.il/Grace/ \n\n";
		cout<<"  This program and source code are released under the: \n  Gnu Public License v. 3.0.\n    http://www.gnu.org/licenses/gpl.txt\n\n";
		return main(0,0);}

	if (type==2)
		{	
		int *tmp, *preftype;
		double *tmpd;
		tmp=(int *)malloc(sizeof(int));
		preftype=(int *)malloc(sizeof(int));
		tmpd=(double *)malloc(sizeof(double));
		*preftype=1;
		
		while(*preftype!=0)
			{PreferencePanel();
			cout<<"  Type: (1 to 4: change individual settings)\n\t(10: restore default)   (0: exit)  ";
			*preftype=ReadKey();

			if (*preftype==1)
				{cout<<"\nThe current length of the cell is: "<<length<<" mm";
				cout<<"\n (1: Change)  (other: no change) ";				
				*tmp=ReadKey();
				if(*tmp==1)
					{cout<<" Enter the new length (in mm): ";
					length=ReadKeyD();}
				if(*tmp!=1)
					{cout<<" Value not changed\n\n";}
				}

			if (*preftype==2)
				{cout<<"\nThe current width of the cell is: "<<width<<" mm";
				cout<<"\n (1: Change)  (other: no change) ";				
				*tmp=ReadKey();
				if(*tmp==1)
					{cout<<" Enter the new width (in mm): ";
					width=ReadKeyD();}
				if(*tmp!=1)
					{cout<<" Value not changed\n\n";}
				}

			if (*preftype==3)
				{cout<<"\nThe current power density is: "<<Pin<<" mW/cm^2";
				cout<<"\n (1: Change)  (other: no change) ";				
				*tmp=ReadKey();
				if(*tmp==1)
					{cout<<" Enter the new power density (in mW/cm^2): ";
					Pin=ReadKeyD();}
				if(*tmp!=1)
					{cout<<" Value not changed\n\n";}
				}

			#if     _MSC_VER
			#else

			if (*preftype==4)
				{cout<<"\nDo you want the spectra to be plotted? ";
				cout<<"\n (Grace needs to be installed to take advantage of this feature)";
				
				cout<<"\n (0: NOT Plotted)  (1: plotted)  (other: no change) ";
				*tmp=vplot;
				vplot=ReadKey();
				if(vplot==1)
					{cout<<" Some features (fitting) may not be available if plotting is not enabled. \n";}
				if(vplot!=1 && vplot!=2)
					{cout<<" Value not changed\n\n";
					vplot=*tmp;}
				}

			#endif

			if (*preftype==10)
				{
				length=length0;
				width=width0;
				Pin=Pin0;
				vplot=vplot0;
				cout<<"\n Default parameters succesfully restored!\n\n";}
			}
		free(tmp);	
		free(preftype);	

		#if     _MSC_VER
			#if MSini ==1
			_chdir(inipath);			
			#endif
		#else
		rc = chdir(getenv("HOME"));
		#endif

		
		ofstream outfile2(ini);
		outfile2<<length<<"\n"<<width<<"\n"<<Pin<<"\n"<<vplot<<"\n";
		
		#if     _MSC_VER
			#if MSini ==1
			_chdir(mainpath);			
			#endif
		#else
		rc = chdir(mainpath);
		#endif
		outfile2.close();		
			
		return main(0,0);
		}
	////////////////////	
	// Actual operation
	////////////////////

	if(type==1)
		{
		#if     _MSC_VER
		#else
		ClosePlot();
		#endif		
		
		flagr=0;
		char *namein;
		namein=(char *)malloc(sizeof(char[CMAXI]));
		cout<<"name input file (WITH extension): ";
		cin>>namein;
		parser(namein);
		
		if(flagr==0)		
			{operate(namein, 1);}
		free(namein);
		main(0,0);}

		return 0;
	}

//OPERATE 
int parser(char *namein)
{	
	//ClosePlot();
	ifstream infile(namein);
	if(!infile)
		{
		cout<<"\n file '"<< namein<<"' not found\n";
		flagr=1;		
		return 0;
		}

	char *temp;
	temp=(char *)malloc(sizeof(char[CMAXI]));


	infile>>temp;
	if(strcmp(temp,"<SYSTEM")==0) 
		{filetype=0;}
	if(strcmp(temp,"Vraw,Iraw,Icorrected,Ifit")==0) 
		{filetype=1;}
	if(strcmp(temp,"Vraw,Iraw,Icorrected,Ifit")!=0 && strcmp(temp,"<SYSTEM")!=0)
		{filetype=2;}
		
	char *outname;
	outname=(char *)malloc(sizeof(char[CMAXI]));

	if(filetype==0 || filetype==1)
		{
		string text;
		char *asciiname;
		asciiname=(char *)malloc(sizeof(char[CMAXI]));

		snprintf(asciiname, CMAXI, "%s.dat", namein);
		snprintf(outname, CMAXI, "%s_power.dat", namein);

		
		if(filetype==0)
			{while(getline(infile,text))
				{infile>>text;
				
				// Start reading within the header of the full file.
				//	Collect sample, notes and area.					
				if(text=="<TEST")
					{getline(infile,text);
					getline(infile,sample);
					sample.erase(sample.begin(),sample.begin()+7);			
					sample.erase(sample.end()-1,sample.end());
					getline(infile,text);
					getline(infile,notes);
					notes.erase(notes.begin(),notes.begin()+6);	
					notes.erase(notes.end()-1,notes.end());	
					getline(infile,text);
					getline(infile,text);
					text.erase(text.begin(),text.begin()+22);
					text.erase(text.end()-1,text.end());
					area=atof(text.c_str());
					getline(infile,configuration);	
					configuration.erase(configuration.begin(),configuration.begin()+21);
					configuration.erase(configuration.end()-1,configuration.end());
					}
			
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////			
			// This section retrieves the extracted data in the file. So far we are not using it for anything
			// as we are extracting our own measurements from the data. However, this will be used when the switch
			// will be set up (so one can decide to use either one).				
				
			if(text=="<EXTRACTED")
					{for (i=0;i<4; i++)
						{getline(infile,text);}
					
					Voc=extractmeasurements(text, 4);
					getline(infile,text);
					Isc=extractmeasurements(text, 4);
					getline(infile,text);
					Jsc=extractmeasurements(text, 4);
					getline(infile,text);
					Pmax=extractmeasurements(text, 4);
					getline(infile,text);
					Vmp=extractmeasurements(text, 4);
					getline(infile,text);
					Imp=extractmeasurements(text, 4);
						
					for (i=0;i<4; i++)
						{getline(infile,text);}	
						FF=extractmeasurements(text, 3);

					for (i=0;i<9; i++)
						{getline(infile,text);}	
						cout<<text<<"\n";
						nu=extractmeasurements(text, 5);
						//cout<<Voc<<"\t"<<Isc<<"\t"<<Jsc<<"\t"<<Pmax<<"\t"<<Vmp<<"\t"<<Imp<<"\t"<<FF<<"\t"<<nu<<"\n";
					}				
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
				// Skip to the data.
				if(text=="Vraw,Iraw,Icorrected,Ifit")
					{break;}				
				}
			}
		
		ofstream out(asciiname);
		ofstream outp(outname);

		getline(infile,text);
		j=0;
		stringstream iss;
		string line;
		while(getline(infile,text))		
			{
			// Check for end-of-file and removes additional tags at the end of the file.	
			if(text[text.length()]!='\n') 
				{text.erase(text.end()-1,text.end() );}
			
			if(text=="</IV DATA>")			
				{cout<<"\"textssssssssss\"\n";
				break;}
			
			// Get the actual data
			iss<<text;
			getline(iss,line,',');
			V[j]=atof(line.c_str());
			getline(iss,line,',');
			I[j]=atof(line.c_str());
			P[j]=V[j]*I[j];
			getline(iss,line,',');
			getline(iss,line,',');

			out<<V[j]<<"\t"<<I[j]<<"\n";
			outp<<V[j]<<"\t"<<P[j]<<"\n";
			j++;
			iss.clear();
			}


		npoints = j;
		out.close();
		outp.close();
		cout<<"\n ASCII file saved in: "<<nameonly(asciiname)<<"\n";
		cout<<" Power curve (ASCII) file saved in: "<<nameonly(outname)<<"\n";

		#if     _MSC_VER
		#else
		IniPlot(asciiname);
		#endif
		free(asciiname);		
		}
		
	if(filetype==2)
		{j=0;
		V[0]=atof(temp );
		infile>>I[0];
		P[0]=V[0]*I[0];
		j=1;
		while(!infile.eof())
			{infile>>V[j]>>I[j];
			P[j]=V[j]*I[j];
			j++;}
		npoints = j-1;

		#if     _MSC_VER
		#else
		IniPlot(namein);
		#endif		

		snprintf(outname, CMAXI, "%s_power.dat", namein);	
		ofstream outp(outname);
		for (j=0; j<npoints; j++)
			{outp<<V[j]<<"\t"<<P[j]<<"\n";}
		outp.close();
		}

		#if     _MSC_VER
		#else		
		Plot(outname);
		#endif

		infile.close();
		free(outname);	
		free(temp);
			
	return 0;	
	}


int operate(char *namein, int s)

{	static coord Max, coV, coI;
	char *asciiname, *infoname;
	
	asciiname=(char *)malloc(sizeof(char[CMAXI]));
	infoname=(char *)malloc(sizeof(char[CMAXI]));

	if(filetype==0 || filetype==1)
		{snprintf(asciiname, CMAXI, "%s.dat", namein);}
	else
		{snprintf(asciiname, CMAXI, "%s", namein);}

	
	snprintf(infoname, CMAXI, "%s_info.txt", namein);

	for(j=0; j<npoints; j++)
		{

		if((I[j-1]<0 && I[j]>-1e-5) || (I[j-1]<0 && I[j]>-1e-5) || (I[0]>=0 && I[npoints-1]>=0))
			{coV=linear(V[j],V[j-1],I[j],I[j-1],0, false);
			Voc=coV.x;
			Ioc=coV.y;}
		
		if((V[j-1]>0 && V[j]<0) || (V[j-1]<0 && V[j]>0) || (V[0]>=0 && V[npoints-1]>=0))
			{coI=linear(V[j],V[j-1],I[j],I[j-1],0, true);
			Isc=coI.y;
			Vsc=coI.x;}
		}

	//here things need to be fixed in cases when the boundaries are not reached.
	
	area=width*length/100;
	Jsc=Isc/area;
	Max=Maximum();	
	Vmp=Max.x;
	Pmax=Max.y;
	Imp=Pmax/Vmp;
	FF=Pmax/(Voc*Isc);	
	nu=(Pmax/((Pin*area)/1000))*100; 

	cout<<"\n File: "<<namein<<"\n";	
	cout<<" Sample: "<<sample<<"\n";
	cout<<" Configuration: "<<configuration<<"\n";	
	cout<<" Notes: "<<notes<<"\n";
	cout<<" Voc = "<<Voc<<" V\n";
	cout<<" Isc = "<<Isc<<" A\n";
	cout<<" Jsc = "<<Jsc<<" A/cm^2\n";
	cout<<" Pmax = "<<Pmax<<" W\n";
	cout<<" Vmaxp = "<<Vmp<<" V\n";
	cout<<" Imaxp = "<<Imp<<" A\n";
	cout<<" FF = "<<FF<<"\n";
	cout<<" Efficiency = "<<nu<<" %\n";
	
	ofstream outfile(infoname);

	outfile<<"File: "<<namein<<"\n";
	outfile<<"Sample: "<<sample<<"\n";
	outfile<<"Configuration: "<<configuration<<"\n";
	outfile<<"Notes: "<<notes<<"\n";	
	outfile<<"Voc = "<<Voc<<" V\n";
	outfile<<"Isc = "<<Isc<<" A\n";
	outfile<<"Jsc = "<<Jsc<<" A/cm^2\n";
	outfile<<"Pmax = "<<Pmax<<" W\n";
	outfile<<"Vmp = "<<Vmp<<" V\n";
	outfile<<"Imp = "<<Imp<<" A\n";
	outfile<<"FF = "<<FF<<"\n";
	outfile<<"Efficiency = "<<nu<<" %\n";
	outfile<<"----------------------------\n";
	
	outfile.close();

	if(s==1)
		{ofstream out(summary, ios::app);
		if(filetype==0)
			out<<"\""<<nameonly(namein)<<"\",\""<<sample<<"\",\""<<configuration<<"\",\""<<notes<<"\","<<Voc<<","<<Isc<<","<<Jsc<<","<<Pmax<<","<<Vmp<<","<<Imp<<","<<FF<<","<<nu<<","<<area<<"\n";
		else
		out<<"\""<<nameonly(namein)<<"\",\"-\",\"-\",\"-\","<<Voc<<","<<Isc<<","<<Jsc<<","<<Pmax<<","<<Vmp<<","<<Imp<<","<<FF<<","<<nu<<","<<area<<"\n";
		out.close();
		
	}
			
	cout<<"\n Info file saved in: "<<nameonly(infoname)<<"\n\n";

	free(infoname);	
	free(asciiname);
	return 0;}


/////////////////////////////
// Max finder
/////////////////////////////


coord Maximum()
	{
	static coord max;
	max.y=0;
	for(i=0; i<npoints;i++)
		{
		if(P[i]>=max.y)

			{max.y= P[i];
			max.x=V[i];}
		}
	return max;
	}

//////////////////////////////////////////////
// Extract line fit parameter from two points
//////////////////////////////////////////////


coord linear(double x2, double x1, double y2, double y1, double x0, bool t)
	{double m, a;
	static coord zero;
	m=(y2-y1)/(x2-x1);
	a=y1-m*x1;
	if(t==true)	
		{zero.x=x0;
		zero.y=m*x0+a;}
	if(t==false)
		{zero.x=-a/m;
		zero.y=0.0;}
	return zero;}
	 
	

/////////////////////////////
// Plotting routines
/////////////////////////////


#if     _MSC_VER
#else
void IniPlot(char* name)
{	pl=0;
	if(vplot==1)
	{
	GraceRegisterErrorFunction(my_error_function);
	if (GraceOpen(2048) == -1) {
	        fprintf(stderr, "Can't run Grace: make sure it's installed. \n");
	        exit(-1);
	  	  }
	PlotSettings();
	GracePrintf("title \"%s\"", name);
	GracePrintf("read \"%s\"", name);
	}
	else
	{}
}


void Plot(char* nameout)
{
	if(vplot==1)	
		{GracePrintf("read \"%s\"", nameout);
		GracePrintf("redraw");}
	else
		{}
	}

void PlotSettings()
	{
	
	GracePrintf("xaxis label \"Voltage [V]\"");
	GracePrintf("yaxis label \"Current [A]\"");
	//GracePrintf("yaxis alt label \"Power [W]\"");
	}	

void ClosePlot()
	{
	if(vplot==1)	
		{GraceClose();}
	else
		{}
}
#endif


/////////////////////////////
// Keyboard I/O
/////////////////////////////

int ReadKey()
{	char tkc[10];
	int tk;
	cin>>tkc;
		
	tk=(int) atof(tkc);
	if(tk<0)
		{return 10;}
	else
		{}

	return tk;
}

float ReadKeyF()
{	char tkc[10];
	float tk=0.0;
	cin>>tkc;
	if (strcmp(tkc,"m")==0 || strcmp(tkc,"m2")==0)
		{if (strcmp(tkc,"m")==0)
			{tk=(float) Maximum().y;}
		if (strcmp(tkc,"m2")==0)
			{tk=(float) Maximum().y/2;}
		}

	else
		{	
		tk= (float) atof(tkc);
		if(tk<0)
			{return 10;}
		else
			{}
	}

	return tk;
}

double ReadKeyD()
{	char tkc[10];
	double tk=0.0;
	cin>>tkc;
	tk= (double) atof(tkc);

	return tk;
}

#if     _MSC_VER
#else
void my_error_function(const char *msg)
{
    fprintf(stderr, "library message: \"%s\"\n", msg);
}
#endif

//////////////////////////////////
// Visualize the Preference Panel
//////////////////////////////////

void PreferencePanel()

{	cout<<"\n****************************************************************************\n \"Preferences\" \n";

	cout<<"\n1) The length of the solar cell is: "<<length<<" mm\n";

	cout<<"\n2) The width of the solar cell is: "<<width<<" mm\n";

	cout<<"\n3) The power density of the solar cell is: "<<Pin<<" W/cm^2\n";
	
	#if     _MSC_VER
	#else
	cout<<"\n4) Spectra are currently: ";
	if(vplot ==0)
		{cout <<" NOT plotted. \n";}
	if (vplot ==1)
		{cout<<" plotted.\n";}
	#endif
	cout<<"\n*****************************************************************************\n";
}

////////////////////////////////////////////////////////////////////////////////////////////
// remove tag from string in header file (leaves only the data and converts it into double)
////////////////////////////////////////////////////////////////////////////////////////////

double extractmeasurements(string text, int a)
	{	
	text.erase(text.begin(),text.begin()+a);
	return atof(text.c_str());
	}

/////////////////////////////
// Strip path from file.
/////////////////////////////

string nameonly(char*namein)
	{string test;
	test=namein;
	#if     _MSC_VER
	test=test.substr( test.rfind("\\")+1, test.length());
	#else
	test=test.substr( test.rfind("/")+1, test.length());
	#endif	
	return test;
	}

/////////////////////////////////
// Find absolute path of a file
/////////////////////////////////

string findpath()
	{		
	string fullFileName = "";
	string path = "";
	#if     _MSC_VER
	
	TCHAR buf[MAX_PATH];
  
	get_module_directory(buf, sizeof(buf)) ;
	path=buf;
	fullFileName = path + string("\\");
	#else

	pid_t pid = getpid();
	char buf[20] = {0};
	sprintf(buf,"%d",pid);
	string _link = "/proc/";
	_link.append( buf );
	_link.append( "/exe");
	char proc[512];
	int ch = readlink(_link.c_str(),proc,512);
	if (ch != -1) {
        	proc[ch] = 0;
        	path = proc;
       		string::size_type t = path.find_last_of("/");
        	path = path.substr(0,t);
    		}

    	fullFileName = path + string("/");

	#endif	
		//cout<<"fullFileName: "<<fullFileName<<"\n";
	return fullFileName;	
	}

/////////////////////////////////////////
// Get working directory (Windows only)
////////////////////////////////////////

#if     _MSC_VER
BOOL get_module_directory(TCHAR *obuf, size_t osize)
{
    if ( ! GetModuleFileName(0, obuf, osize) )
    {
        *obuf = '\0';// insure it's NUL terminated
        return FALSE;
    }
  
    // run through looking for the *last* slash in this path.
    // if we find it, NUL it out to truncate the following
    // filename part.
  
    TCHAR*lastslash = 0;
     for ( ; *obuf; obuf ++)
    {
        if ( *obuf == '\\' || *obuf == '/' )
            lastslash = obuf;
    }
  
    if ( lastslash ) *lastslash = '\0';
	   return TRUE;
}
#endif

