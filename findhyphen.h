/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
#ifndef _FINDHYPHEN_H
#define _FINDHYPHEN_H

extern void BuildTree(char **Patterns);

extern void AddException(char *Name);

extern void DoHyphens(char *word, int **posis, int *posicnt);

extern void DestroyTree(void);
#endif /* _FINDHYPHEN_H */
