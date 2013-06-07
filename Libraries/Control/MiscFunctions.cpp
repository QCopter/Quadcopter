/**
  *  Brandon Yue
  *
  *  This file will hold functions that I think might be useful somewhere.
  *
  */


/**
SILENT ERROR FLAGGER.
    Every once in a while, there's a spot in the code which we never want the runtime to get to, but
	if it did, the runtime wouldn't crash. In this case, we'd want to have a note of this saved to some
	external file for review. This function would be hard coded into the program to flag these errors.
    
        INPUT
	Takes optional parameters:
	FILE_ID: Default -1 (unspecified) Name or corresponding file ID where the error occured.
	LINE_NO: Default -1 (unspecified) Line which the error occured. Due to the tedious nature of hard coding this,
	         the value entered here will usually be an approximation.
	CODE:    Default -1 (unspecified) Error code. Probably not important for our purposes.
    
        OUTPUT
    Writes an error log to an external file named ERR_LOG.csv
    Err messages take the form:
    UTC_TIME, FILE_ID, LINE_NO, CODE\n
**/

void SILENT_ERR (int FILE_ID = -1, int LINE_NO = -1, int CODE = -1)
{
    //Write code here.
}
