#ifndef _FINDHYPHEN_H
#define _FINDHYPHEN_H
extern char mytolower(char ch);

extern void BuildTree(char **Patterns);

extern void AddException(char *Name);

extern void DoHyphens(char *word, int **posis, int *posicnt);

extern void DestroyTree(void);
#endif /* _FINDHYPHEN_H */
