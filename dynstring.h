#ifndef _DYNSTRING_H
#define _DYNSTRING_H

#define DYNSTRING_DEFLEN 240

typedef struct
        {
          LongInt Length, MaxLength;
          char DefContent[240], *pContent;
        } tDynString;

#endif /* _DYNSTRING_H */