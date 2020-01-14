#include "types.h"
#include "user.h"
#include "stat.h"
#include "fcntl.h"

#define TAIL_MAX_BUFFER_SIZE 512

char gcarBuffer[TAIL_MAX_BUFFER_SIZE];


int mNoOfLines = 10,miNoOfFiles = 0;
char mFileNames[5][20];


void tail(int pFD)
{
	int n,i;
	int liLineCounter = 0, liStartPrintLineNo = 0, liTempLineCounter = 0;

	int liTempFile;
  	liTempFile = open ("TailTempFile", O_CREATE | O_RDWR);

	while( (n = read(pFD, gcarBuffer, sizeof(gcarBuffer))) > 0 )
	{		
		write(liTempFile, gcarBuffer, n);
		//READ File DATA
		for(i = 0; i < n; i++)
		{
			if(gcarBuffer[i] == '\n')
			{
				liLineCounter++;
			}
		}	
	}

	if(gcarBuffer[i-1] != '\n')
	{
		liLineCounter++;	
	}		
	close (liTempFile);

	liStartPrintLineNo = liLineCounter - mNoOfLines;
	liTempFile = open ("TailTempFile", 0);

	//READ TEMP FILE DATA
	while( (n = read(liTempFile, gcarBuffer, sizeof(gcarBuffer))) > 0 )
	{		
		//READ File DATA
		for(i = 0; i < n; i++)
		{
			if(liStartPrintLineNo <= liTempLineCounter)
			{
				printf(1,"%c",gcarBuffer[i]);			
			}
			if(gcarBuffer[i] == '\n')
			{
				liTempLineCounter++;
			}
		}	
	}

	close (liTempFile);
	unlink("TailTempFile");
}



int length(char s[]) 
{
   int c = 0;
   
   while (s[c] != '\0') 
   {
      c++;
   }
   return c;
}

void printToNextLine()
{
	printf(1,"\n");
}

char toLowerCase(char pcChar)
{
	if( pcChar >='A' && pcChar <='Z' )
	{
		return pcChar + 32;
	}
	return pcChar;
}

void displayMessage(int no)
{
	switch(no)
	{
		case 1:{
				printf(1," tail : Invalid Command ");
				printf(1," Usage: tail -NoOfLines Filename");
				printToNextLine();
			break;
		}  
	}
	exit();  
}


void substr(char *pString, char *pDest, int s )
{		
	int counter = 0,i;
	for(i = s; i < length(pString); i++ )
	{
		pDest[counter] = pString[i];
		counter++;
	}
	pDest[counter] = '\0';
}


void initializeOptions(int piArgc, char *psarArgv[])
{
	int i;
	for(i = 1; i < piArgc; i++)
	{
		char *lsoption = psarArgv[i];

		if( lsoption[0] == '-' )
		{
			if(length(lsoption) == 1)
			{
				displayMessage(1);
				break;
			}	

			char lstrSub[length(lsoption) - 1];
			substr(lsoption,lstrSub,1);	
			mNoOfLines = atoi(lstrSub);
			if(mNoOfLines == 0)
			{
				displayMessage(1);
			}
		}
		else
		{		
	 		strcpy(	mFileNames[miNoOfFiles], lsoption);
	 		miNoOfFiles++;
		}
	}
}			


int	main(int piArgc, char *psarArgv[])
{
	initializeOptions( piArgc , psarArgv );	

	int i,fd; 
	for( i = 0; i < miNoOfFiles; i++)
	{
		if(miNoOfFiles != 1)
		{
			printf(1,"-----------%s ----------------\n",mFileNames[i]);	
		}
		if( (fd = open(mFileNames[i], 0) ) < 0 )
		{
    		printf(1, "tail: cannot open %s for reading : No such file or directory\n", mFileNames[i]);
    		break;
    	}
    	tail(fd);
  	}

  	if(miNoOfFiles == 0)
  	{
  		tail(0);
  	}


	exit();
}