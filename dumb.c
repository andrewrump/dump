/*
** DUMB - Dos/*nix/Mac ASCII conversion tool
** Usage: dumb {-[d|u|m|b]} {<file>}
**
** Convert the carriage return newline convention used on whatever
** platform to the format used on the platform specified by the user.
**
** By: Leif Andrew Engels Rump, andrew-nospam@rump.dk, Copyright: 1995, 2003, 2006
** Version: 1.0 Original version
** Version: 2.0 Rewritten because source code was lost
*/

/*
** Known problems:
** - RETURNVALUE NOT LOGICAL!!!???!!!
** - Should default selection depending on platform
** - Does not keep filename casing, ownership, rights, etc. Should truncate original file instead and add
*/

#define VERSION "2.0"
#define COPYRIGHT "1995, 2003"
//#define DEBUG 2

#include <stdlib.h>
#include <limits.h>

#ifndef _MAX_PATH
#define _MAX_PATH PATH_MAX
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define CARRIAGE  13
#define LINEFEED  10
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE (~FALSE)
#endif

typedef enum {eUndefined = '?', eDOS = 'd', eUNIX = 'u', eMAC = 'm', eBACKWARDS = 'b'} dumb_t;

/*
**
*/

int PutDumb(dumb_t eDUMB, FILE *fOut)
{
  int iPut = 0;

  switch (eDUMB)
  {
  case eDOS :
    iPut = fputc(CARRIAGE, fOut);
    if (iPut != EOF)
      iPut = fputc(LINEFEED, fOut);
    break;
  case eUNIX :
    iPut = fputc(LINEFEED, fOut);
    break;
  case eMAC :
    iPut = fputc(CARRIAGE, fOut);
    break;
  case eBACKWARDS :
    iPut = fputc(LINEFEED, fOut);
    if (iPut != EOF)
      iPut = fputc(CARRIAGE, fOut);
    break;
  }
  return iPut;
}

/*
**
*/

int Detected(dumb_t eDUMB, FILE *fOut, int *bDetected, int *bTesting)
{
  int iPut = 0;

  if (*bDetected)
  {
    if (*bTesting)
    {
      iPut = PutDumb(eDUMB, fOut);
      *bTesting = FALSE;
      *bDetected = FALSE;
    }
    else
      iPut = PutDumb(eDUMB, fOut);
  }
  else
  {
    if (*bTesting)
    {
      iPut = PutDumb(eDUMB, fOut);
      *bTesting = FALSE;
    }
    else
      *bDetected = TRUE;
  }

  return iPut;
}

/*
**
*/

int dumb(dumb_t eDUMB, FILE *fIn, FILE *fOut)
{
  int   bCarriage = FALSE;
  int   bLineFeed = FALSE;
  int   iCharacter;
  int   iPut = 0;

  while (!feof(fIn) && iPut != EOF)
  {
    iCharacter = fgetc(fIn);
    if (!feof(fIn))
    {
      switch (iCharacter)
      {
      case CARRIAGE :
        iPut = Detected(eDUMB, fOut, &bCarriage, &bLineFeed);
        break;
      case LINEFEED :
        iPut = Detected(eDUMB, fOut, &bLineFeed, &bCarriage);
        break;
      default :
        if (bCarriage || bLineFeed)
        {
          iPut = PutDumb(eDUMB, fOut);
          bCarriage = FALSE;
          bLineFeed = FALSE;
        }
        if (iPut != EOF)
          iPut = fputc(iCharacter, fOut);
        break;
      }
    }
  }

  if (iPut != EOF && (bCarriage || bLineFeed))
    iPut = PutDumb(eDUMB, fOut);

  return iPut;
}

/*
**
*

void xxx(char *argv)
{
  int   ifIn;
  _stat sIn;

    //_access
  ifIn = _open(argv, _O_BINARY | _O_CREAT | _O_EXCL | _O_RDWR | _O_SEQUENTIAL);
  if (ifIn == -1)
    ;
  else
  {
    _fstat
  _chmod
  //_chown
  _close(ifIn);
}

/*
**
*/

int main(int argc, char *argv[])
{
  dumb_t  eDUMB = eUndefined;
  int     bDUMB = FALSE;

  // Decode argument
  if (argc != 1)
  {
    if(argv[1][0] == '-' && argv[1][1] != '\0' && argv[1][2] == '\0')
    {
      bDUMB = TRUE;
      switch (argv[1][1])
      {
      case eDOS :
        eDUMB = eDOS;
        break;
      case eUNIX :
      case '*' :  // hidden
        eDUMB = eUNIX;
        break;
      case eMAC :
        eDUMB = eMAC;
        break;
      case eBACKWARDS :
        eDUMB = eBACKWARDS;
        break;
      case eUndefined :
        eDUMB = eUndefined;
        break;
      }
    }
    else
    {
#if defined(_WIN32)
      eDUMB = eDOS;
#elif defined(_WIN16)
      eDUMB = eDOS;
#elif defined(_DOS)
      eDUMB = eDOS;
#elif defined(_MAC)
      eDUMB = eMAC;
#else
      eDUMB = eUNIX;
#endif
    }
  }

  if (eDUMB == eUndefined)
  {
    fprintf(stderr, "\nUsage: %s -[%c|%c|%c|%c] {<files>}\n\n", argv[0], eDOS, eUNIX, eMAC, eBACKWARDS);
    //              "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
    fprintf(stderr, "DOS/*nix/Mac ASCII carriage return and line feed conversion tool \n\n"
                    "It takes whatever ASCII from whatever platform and convert it to the platform\n"
                    "specified on the command line: -%c for DOS, -%c for *nux, -%c for Mac or\n"
                    "-%c for backwards, i.e., line feed before carriage return!\n\n"
                    "WARNING: Do not use this tool to convert anything else but ASCII files!\n\n"
                    "Version %s, Copyright %s Andrew Engels Rump (andrew-nospam@rump.dk)\n",
                    eDOS, eUNIX, eMAC, eBACKWARDS, VERSION, COPYRIGHT);
    return 1;
  }
  else
  {
    if (argc == (bDUMB ? 2 : 1)) // Convert stdin. Deliver to stdout
      return dumb(eDUMB, stdin, stdout);
    else
    {
      int   iCount, iResult = 0;
      FILE  *fIn, *fOut;
      char  *caTmpName;
      char  caOutName[_MAX_PATH];  // PATH_MAX

      // Convert one file at a time
      for (iCount = (bDUMB ? 2 : 1); iCount < argc && iResult == 0; iCount++)
      {
        // Open input file
        fIn = fopen(argv[iCount], "rb");
        if (fIn)
        {
#ifdef  DEBUG
          fprintf(stderr, "Opened in: \"%s\"\n", argv[iCount]);
#if DEBUG == 2
          (void)fgetc(stdin);
#endif
#endif
          // Get name of temporary output file
          caTmpName = tmpnam(NULL); // Linux suggests mkstemp!
          if (caTmpName != NULL)
          {
#ifdef  DEBUG
            fprintf(stderr, "Temporary out: \"%s\"\n", caTmpName);
#if DEBUG == 2
            (void)fgetc(stdin);
#endif
#endif
            // Create output file
            strcpy(caOutName, caTmpName);
            fOut = fopen(caOutName, "wb");
            if (fOut)
            {
#ifdef  DEBUG
              fprintf(stderr, "Temporary out created.\n");
#if DEBUG == 2
              (void)fgetc(stdin);
#endif
#endif
              // Convert file
              iResult = dumb(eDUMB, fIn, fOut);

              if (iResult != EOF)
                iResult = fclose(fOut);
              else
              {
                fprintf(stderr, "Error while converting file \"%s\"\n", argv[iCount]);
                (void)fclose(fOut);
              }
            }
            else
            {
              fprintf(stderr, "Could not open temporary file \"%s\" for writing\n", caTmpName);
              return 1;
            }
          }
          else
          {
            fprintf(stderr, "Could not create temporary file \"%s\"\n", caTmpName);
            return 1;
          }

          if (iResult != EOF)
            iResult = fclose(fIn);
          else
            (void)fclose(fIn);

          if (iResult != EOF) // Replace old file with converted file
          {
            // Move the original file to a temporary file
            caTmpName = tmpnam(NULL);
#ifdef  DEBUG
            fprintf(stderr, "Temporary in \"%s\"\n", caTmpName);
#if DEBUG == 2
            (void)fgetc(stdin);
#endif
#endif
            iResult = rename(argv[iCount], caTmpName);
#ifdef  DEBUG
            fprintf(stderr, "Rename \"%s\" to \"%s\" returned %d\n", argv[iCount], caTmpName,
                    iResult);
#if DEBUG == 2
            (void)fgetc(stdin);
#endif
#endif

            if (iResult == 0)
            {
              // Move the converted file to the old filename
              iResult = rename(caOutName, argv[iCount]);
#ifdef  DEBUG
              fprintf(stderr, "Rename \"%s\" to \"%s\" returned %d\n", caOutName, argv[iCount],
                      iResult);
#if DEBUG == 2
              (void)fgetc(stdin);
#endif
#endif
              if (iResult == 0)
              {
                iResult = unlink(caTmpName);
#ifdef  DEBUG
                fprintf(stderr, "Unlinking \"%s\" return %d\n", caTmpName, iResult);
#if DEBUG == 2
                (void)fgetc(stdin);
#endif
#endif
                if (iResult != 0)
                {
                  fprintf(stderr, "Could not remove the temporary file \"%s\"\n", caTmpName);
                }
              }
              else
              {
                (void)unlink(caTmpName);
#ifdef  DEBUG
                fprintf(stderr, "Unlinking \"%s\"\n", caTmpName);
#if DEBUG == 2
                (void)fgetc(stdin);
#endif
#endif
              }
            }
            else
            {
#ifdef  DEBUG
              fprintf(stderr, "Could not rename \"%s\" to \"%s\" returned %d with code %d\n",
                      argv[iCount], caTmpName, iResult, errno);
              fprintf(stderr, "Unlinking \"%s\"\n", caOutName);
#if DEBUG == 2
              (void)fgetc(stdin);
#endif
#else
              fprintf(stderr, "Could not convert \"%s\"\n", argv[iCount]);
#endif
              (void)unlink(caOutName);
            }
          }
#ifdef  DEBUG
          else
            fprintf(stderr, "Bailing out.\n");
#endif
        }
        else
        {
          fprintf(stderr, "Could not open \"%s\" for reading\n", argv[iCount]);
          return 1;
        }
      }
      return iResult;
    }
  }
}
