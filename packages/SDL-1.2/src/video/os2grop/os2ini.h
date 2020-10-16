#ifndef _OS2INI_H_
#define _OS2INI_H_

void os2iniOpen(void);
void os2iniClose(void);
char* os2iniGetValue(char *pszSwitch);
int os2iniGetBool(char *pszSwitch, int iDefault);
int os2iniIsValue(char *pszSwitch, char *pszNeedValue);

#endif /* _OS2INI_H_ */
