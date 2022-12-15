#ifndef ADDONE
#define ADDONE

#include "define.h"




int addone_RunAddoneAsCommand(LinkLib *head, const char *AddoneName, const char *FuncName, char *argv);
Gp addone_NoAddone(Res resbonse);

LinkLib *addone_MakeFuncLink(LinkLib **, const char dir[128]);
Gp addone_RunAddoneAsApplication(LinkLib *head, Res *resbonse, const char *AddoneName, const char *FuncName);
void addone_ConsoleControl(char *cmd, LinkLib **head);

#endif
