#include <SDL_config.h>
#include <SDL_stdinc.h>
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#include <os2.h>
#include "debug.h"
#include "os2ini.h"

typedef struct _INIITEM {
  PSZ		pszSwitch;
  PSZ		pszValue;
} INIITEM, *PINIITEM;

static BOOL		fOpened = FALSE;
static PINIITEM		pIniItems = NULL;
static ULONG		cIniItems = 0;

#define _MAX_VALUES	( sizeof(aValues) / sizeof(INIITEM) )

#define _STR_LTrim(s) do { while( SDL_isspace(*s) || *s==',' ) s++; } while( 0 )
#define _STR_RTrim(s,e) while( e>(s) && ( SDL_isspace(*(e-1)) ) ) e--
#define _STR_Trim(s,e) _STR_LTrim(s,e); _STR_RTrim(s,e)
#define _STR_skipSeparators(ps,cs) while( ( *ps == cs ) || SDL_isspace(*ps) ) ps++
#define _STR_skipNotSeparator(ps,cs) while( ( *ps != '\0' ) && ( *ps != cs ) ) ps++
#define _STR_skipNotSpace(ps) while( ( *ps != '\0' ) && !SDL_isspace(*ps) ) ps++

// _isMatch() compares the string to pattern (with '*', '?')

static BOOL _isMatch(PCHAR pcStr, ULONG cbStr, PCHAR pcPtrn, ULONG cbPtrn)
{
  PCHAR		pcStarStr = NULL;
  ULONG		cbStarStr = 0;
  PCHAR		pcStartPtrn = NULL;
  ULONG		cbStartPtrn = 0;
  CHAR		chStr, chPtrn;

  if ( cbStr == 0 && cbPtrn == 0 )
    return TRUE;

  while( cbStr != 0 )
  {
    chStr = SDL_toupper( *pcStr );

    if ( cbPtrn != 0 )
    {
      chPtrn = SDL_toupper( *pcPtrn );

      if ( chPtrn == '?' || chPtrn == chStr )
      {
        pcPtrn++;
        cbPtrn--;
        pcStr++;
        cbStr--;
        continue;
      }

      if ( chPtrn == '*' )
      {
        //skip all continuous '*'
        do
        {
          cbPtrn--;
          if ( cbPtrn == 0 )		// if end with '*', its match.
            return TRUE;
          pcPtrn++;
        }
        while( *pcPtrn == '*' );

        pcStartPtrn = pcPtrn;		// store '*' pos for string and pattern
        cbStartPtrn = cbPtrn;
        pcStarStr = pcStr;
        cbStarStr = cbStr;
        continue;
      }
    }

    if ( ( cbPtrn == 0 || chPtrn != chStr ) && ( pcStartPtrn != NULL ) )
    {
      pcStarStr++;		// Skip non-match char of string, regard it
      cbStarStr--;		// matched in '*'.
      pcStr = pcStarStr;
      cbStr = cbStarStr;

      pcPtrn = pcStartPtrn;	// Pattern backtrace to later char of '*'.
      cbPtrn = cbStartPtrn;
    }
    else
      return FALSE;
  }

  // Check if later part of ptrn are all '*'
  while( cbPtrn != 0 )
  {
    if ( *pcPtrn != '*' )
      return FALSE;

    pcPtrn++;
    cbPtrn--;
  }

  return TRUE;
}

// _readIni() read ini-file pszIniFName, search record for program pszProgFName
// and store given switches to the global list pIniItems

static VOID _readIni(PSZ pszIniFName, PSZ pszProgFName)
{
  FILE		*fdIni = fopen( pszIniFName, "r" );
  CHAR		acBuf[1024];
  PCHAR		pcSwBegin, pcSwEnd, pcValBegin, pcValEnd, pcNext;
  PSZ		pszProgName;
  BOOL		fQuotes;

  if ( fdIni == NULL )
  {
    debug( "Cannot open ini-file: %s", pszIniFName );
    return;
  }
  debug( "Read ini-file: %s", pszIniFName );

  while( fgets( acBuf, sizeof(acBuf), fdIni ) != NULL )
  {
    pcSwBegin = acBuf;
    _STR_LTrim( pcSwBegin );

    // Skip comments and empty lines
    if ( *pcSwBegin == '#' || *pcSwBegin == ';' || *pcSwBegin == '\0' )
      continue;

    // Read file name from ini-file's line

    fQuotes = *pcSwBegin == '"';
    if ( fQuotes )
    {
      pcSwBegin++;
      _STR_LTrim( pcSwBegin );
      if ( *pcSwBegin == '\0' )
        break;
      pcSwEnd = pcSwBegin;
      _STR_skipNotSeparator( pcSwEnd, '"' );
      if ( *pcSwEnd == '\0' )
        break;
      pcNext = pcSwEnd + 1;
      _STR_RTrim( pcSwBegin, pcSwEnd );
      if ( pcSwBegin == pcSwEnd )
        continue;
    }
    else
    {
      pcSwEnd = pcSwBegin;
      _STR_skipNotSpace( pcSwEnd );
      pcNext = pcSwEnd + 1;
      if ( pcSwEnd == NULL )
        continue;
    }

    *pcSwEnd = '\0';
    if ( SDL_strchr( pcSwBegin, '\\' ) == NULL )
    {
      // Only file name without path given.
      pszProgName = SDL_strrchr( pszProgFName, '\\' );
      if ( pszProgName != NULL )
        pszProgName++;
    }
    else
      pszProgName = pszProgFName;

    if ( !_isMatch( pszProgName, SDL_strlen( pszProgName ),
                    pcSwBegin, pcSwEnd - pcSwBegin ) )
    {
      // Not this program...
      continue;
    }
    *pcSwEnd = '\0';
    debug( "Program's record found: %s", pcSwBegin );

    // Read switches

    pcSwBegin = pcNext;
    _STR_LTrim( pcSwBegin );
    while( *pcSwBegin != '\0' )
    {
      // Parse SWITCH:VALUE[,] part

      pcSwEnd = pcSwBegin;
      _STR_skipNotSeparator( pcSwEnd, ':' );
      if ( *pcSwEnd == '\0' )
        break;
      pcValBegin = pcSwEnd + 1;
      _STR_RTrim( pcSwBegin, pcSwEnd );

      _STR_LTrim( pcValBegin );
      if ( *pcValBegin == '\0' )
        break;
      pcValEnd = pcValBegin;
      _STR_skipNotSeparator( pcValEnd, ',' );
      if ( *pcValEnd != '\0' )
      {
        pcNext = pcValEnd + 1;
        _STR_LTrim( pcNext );
      }
      else
        pcNext = NULL;

      _STR_RTrim( pcValBegin, pcValEnd );
 
      // Switch and value were read.
      // pcSwBegin - switch name,  pcValBegin - value
      if ( pcSwBegin != pcSwEnd && pcValBegin != pcValEnd )
      {
        ULONG		ulIdx;

        *pcSwEnd = '\0';
        *pcValEnd = '\0';

        // Search key in the existing list
        for( ulIdx = 0; ulIdx < cIniItems; ulIdx++ )
        {
          if ( SDL_strcasecmp( pIniItems[ulIdx].pszSwitch, pcSwBegin ) == 0 )
          {
            // Switch found - update value
            debug( "Update switch \"%s\". Old value: \"%s\", new value: \"%s\"",
                   pcSwBegin, pIniItems[ulIdx].pszValue, pcValBegin );

            SDL_free( pIniItems[ulIdx].pszValue );
            pIniItems[ulIdx].pszValue = SDL_strdup( pcValBegin );
            break;
          }
        }

        if ( ulIdx == cIniItems )
        {
          // Switch not listed

          debug( "New switch \"%s\". Value: \"%s\"", pcSwBegin, pcValBegin );

          if ( (cIniItems & 0x07) == 0 )
          {
            // Expand list every 8 items
            PINIITEM	pNewList = SDL_realloc( pIniItems, sizeof(INIITEM) *
                                                ((cIniItems & ~0x07) + 8) );
            if ( pNewList == NULL )
            {
              debug( "Not enough memory" );
              return;
            }
            pIniItems = pNewList;
          }
          // Store new switch/value
          pIniItems[cIniItems].pszSwitch = SDL_strdup( pcSwBegin );
          pIniItems[cIniItems].pszValue = SDL_strdup( pcValBegin );
          cIniItems++;
        }
      }

      if ( pcNext == NULL )
        break;
      pcSwBegin = pcNext;
    }
    break;
  } 
  fclose( fdIni ); 
}


void os2iniOpen()
{
  PTIB			tib;
  PPIB			pib;
  PSZ			pszHome = SDL_getenv( "HOME" );
  PCHAR			pcSlash;
  ULONG			cbPath;
  CHAR			acProgFName[CCHMAXPATH + 1];
  CHAR			acIniFName[CCHMAXPATH + 1];

  if ( fOpened )      // Already opened
    return;

  DosGetInfoBlocks( &tib, &pib );
  _fullpath( acProgFName, pib->pib_pchcmd, sizeof(acProgFName) );
  debug( "Program: %s", &acProgFName );

  // Read global ini-file

  if ( pszHome != NULL )
  {
    SDL_snprintf( acIniFName, sizeof(acIniFName), "%s\\sdl.ini", pszHome );
    _readIni( acIniFName, acProgFName );
  }

  // Read local ini-file

  pcSlash = SDL_strrchr( acProgFName, '\\' );
  if ( pcSlash != NULL )
  {
    cbPath = pcSlash - acProgFName + 1;
    SDL_memcpy( acIniFName, acProgFName, cbPath );
  }
  else
    cbPath = 0;
  SDL_strlcpy( &acIniFName[cbPath], "sdl.ini", sizeof( &acIniFName ) - cbPath );

  _readIni( acIniFName, acProgFName );
  fOpened = TRUE;
}

void os2iniClose()
{
  if ( !fOpened )      // Was not opened
    return;

  if ( pIniItems != NULL )
  {
    while( cIniItems != 0 )
    {
      cIniItems--;
      SDL_free( pIniItems[cIniItems].pszSwitch );
      SDL_free( pIniItems[cIniItems].pszValue );
    }

    SDL_free( pIniItems );
    pIniItems = NULL;
  }
  fOpened = FALSE;
}

char* os2iniGetValue(char *pszSwitch)
{
  ULONG		ulIdx;

  if ( !fOpened )
    return NULL;

  for( ulIdx = 0; ulIdx < cIniItems; ulIdx++ )
  {
    if ( SDL_strcasecmp( pIniItems[ulIdx].pszSwitch, pszSwitch ) == 0 )
      return pIniItems[ulIdx].pszValue;
  }
  return NULL;
}

int os2iniGetBool(char *pszSwitch, int iDefault)
{
  PSZ		pszValue = os2iniGetValue( pszSwitch );

  debug( "Switch %s = %s", pszSwitch, pszValue == NULL ? "NULL" : pszValue );
  return pszValue == NULL ? iDefault : ( *pszValue == '1' ||
                                         SDL_toupper( *pszValue ) == 'Y' ||
                                         SDL_strcasecmp( pszValue, "on" ) == 0 );
}

int os2iniIsValue(char *pszSwitch, char *pszNeedValue)
{
  PSZ		pszValue = os2iniGetValue( pszSwitch );

  return pszValue != NULL && ( SDL_strcasecmp( pszValue, pszNeedValue ) == 0 );
}
