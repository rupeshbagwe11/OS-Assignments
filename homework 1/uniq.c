#include "types.h"
#include "user.h"

#define UNIQ_MAX_BUFFER_SIZE 512
#define UNIQ_MAX_LINE_CHARACTERS 8192

char gcarBuffer[UNIQ_MAX_BUFFER_SIZE];
char gcarLine[UNIQ_MAX_LINE_CHARACTERS];

int gbISC = 0, gbISD = 0, gbISCI = 0, gbStandardInput = 1;

void printToNextLine()
{
	printf(1,"\n");
}

int isAlphabet(char pcChar)
{
	if( ( pcChar >= 'a' && pcChar <= 'z' ) || ( pcChar >='A' && pcChar <='Z' ))
	{
		return 1;
	}
	return 0;
}

char toLowerCase(char pcChar)
{
	if( pcChar >='A' && pcChar <='Z' )
	{
		return pcChar + 32;
	}
	return pcChar;
}

 char* toLowerString(char * psString)
{
	int i;

	char *lstempstring = malloc( strlen(psString) + 1 );
	strcpy( lstempstring, psString );

	for( i = 0 ; i < strlen(lstempstring) ; i++)
	{
		if(isAlphabet(lstempstring[i]) == 1)
		{
			lstempstring[i] = toLowerCase(lstempstring[i]);
		}
	}
	return lstempstring;
}

//Compares two string passed
//piTgnoreCase = 0 - Case Sensitive Comparison, 1 - Case Insensitve Comparison
int equalsWithWithoutCase( char *psString1 , char *psString2, int piIgnoreCase )
{
	if( piIgnoreCase == 1)
	{
		if( strcmp( toLowerString( psString1 ) , toLowerString( psString2 ) ) == 0 )
		{
			return 1;
		}
		return 0;
	}
	else
	{
		if( strcmp( psString1 , psString2 ) == 0 )
		{
			return 1;
		}
		return 0;
	}
}

void displayMessage(int no)
{
	switch(no)
	{
		case 1:{
				printf(1," uniq : Invalid Command");
				printf(1,"\n Try 'uniq --help' for more information.");
				printToNextLine();
			break;
		}
		case 2:{
				printf(1," uniq : Invalid Option ");
				printf(1,"\n Try 'uniq --help' for more information.");
				printToNextLine();
			break;
		}
       	default: {
       			printf(1," Usage: uniq [OPTION]... [INPUT] ");
				printf(1,"\n Filter adjacent matching lines from File or Standard Input, writing to Standard Output.");
				printToNextLine();
				printf(1,"\n -c, --count           prefix lines by the number of occurrences ");
				printf(1,"\n -d, --repeated        only print duplicate lines ");
				printf(1,"\n -i, --ignore-case     ignore differences in case when comparing ");
				printToNextLine();
       		break;
       	}        
	}
	        exit();  
}

int isOption(char * psString)
{
	if(psString[0] == '-')
	{
		return 1;
	}
	return 0;
}

void initializeOptions(int piArgc, char *psarArgv[])
{
	int i, j;
	for(i = 1; i < piArgc; i++)
	{
		if(isOption(psarArgv[i]) == 1)
		{
			char *lsoption = psarArgv[i];

			if( equalsWithWithoutCase( lsoption , "--help", 1 ) == 1)
			{
				displayMessage(0);
				break;
			}

			for( j = 1; j < strlen(lsoption); j++)
			{
				if( toLowerCase( lsoption[j] ) == 'i' )
				{
					gbISCI = 1;
				}
				else if( toLowerCase( lsoption[j] ) == 'c' )
				{
					gbISC = 1;
				}
				else if( toLowerCase( lsoption[j] ) == 'd' )
				{
					gbISD = 1;
				}
				else
				{
					displayMessage(2);	
				}
			}
		}
	}
}		
	
int giDuplicateCount = 0;

void uniqOutput(char *psString1 , char * psString2 )
{
	if( equalsWithWithoutCase( psString1 , psString2, gbISCI ) == 0 )
	{
		if( ( gbISD == 1 && giDuplicateCount > 0 )  || gbISD == 0)
		{
			if( gbStandardInput == 1)
			{
				printf(1,"      ");
			}
			if( gbISC == 1 )
			{
				printf( 1,"  %d   ",giDuplicateCount+1 );	
			}
			printf( 1,"%s\n",psString1);
		}
		giDuplicateCount = 0;
	}
	else
	{
		giDuplicateCount++;
	}
}	

void uniq(int piFileData, char *psFileName)
{
	int i,n;
	int liLineCharCounter = 0;
	int lbISFirst = 1;

	char *lsString1  = "";
	while( (n = read(piFileData, gcarBuffer, sizeof(gcarBuffer))) > 0 )
	{		
		//READ File DATA
		for(i = 0; i < n; i++)
		{
			if(gcarBuffer[i] == '\n')
			{
				gcarLine[liLineCharCounter] = '\0';
				liLineCharCounter = 0;	

				char *lsString2 = malloc( strlen(gcarLine) + 1 );
				strcpy( lsString2, gcarLine );

				if(lbISFirst == 0)
				{
					uniqOutput(lsString1 , lsString2);
				}
				
				lsString1 = lsString2;
				lbISFirst = 0;
			}
			else
			{
				gcarLine[liLineCharCounter] = gcarBuffer[i];
				liLineCharCounter++;
			}
		}	
	}

	gcarLine[liLineCharCounter] = '\0';
	char *lsString2 = malloc( strlen(gcarLine) + 1 );
	strcpy( lsString2, gcarLine );

	uniqOutput(lsString1 , lsString2);

	if( n < 0 )
	{
		printf(1, "\nuniq : Cannot Read File %s\n ", psFileName);
		printToNextLine();
		exit();
	}
}

int	main(int piArgc, char *psarArgv[])
{
	int i,liFileData;
	char *lsFileName;

	initializeOptions( piArgc , psarArgv );	

	//twice bcoz : uniq abc xyz lmn -c -i  : should also execute
	for(i = 1; i < piArgc; i++)
	{
		if(isOption(psarArgv[i]) == 1)
		{
			continue;
		}

		lsFileName = psarArgv[i];
		//filename exists so no standard input
		gbStandardInput = 0;

		//Print the filename
		printf(1,"---------  %s  ----------\n",lsFileName);
		
		liFileData = open(lsFileName,0);
			
		//If file doesn't open show error and continue to nextfile
		if(liFileData < 0)
		{			
			printf(1, "\n uniq : Cannot Open File %s", lsFileName);
			continue;
		}
		//call uniq for each file
		uniq(liFileData,lsFileName); 
	
		//close open file
		close(liFileData);
	}

	if( gbStandardInput == 1)
	{
		uniq(0,"Standard Input"); 
	}

	exit();

}
