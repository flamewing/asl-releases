/* headids.h */
/*****************************************************************************/
/* Makroassembler AS                                                         */
/*                                                                           */
/* Hier sind alle Prozessor-IDs mit ihren Eigenschaften gesammelt            */
/*                                                                           */
/* Historie: 29. 8.1998 angelegt                                             */
/*                                                                           */
/*****************************************************************************/

/* Hex-Formate */

typedef enum {Default,MotoS,
              IntHex,IntHex16,IntHex32,
              MOSHex,TekHex,TiDSK,Atmel} THexFormat;

typedef struct
         {
          char *Name;
          Word Id;
          THexFormat HexFormat;
         } TFamilyDescr,*PFamilyDescr;

extern PFamilyDescr FindFamilyByName(char *Name);

extern PFamilyDescr FindFamilyById(Word Id);

extern void headids_init(void);
