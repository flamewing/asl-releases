/* deco87c800.c */
/*****************************************************************************/
/*                                                                           */
/* DissectorTLCS-870                                                         */
/*                                                                           */
/*****************************************************************************/
/* $Id: deco87c800.c,v 1.4 2015/02/07 16:31:51 alfred Exp $                  */
/*****************************************************************************
 * $Log: deco87c800.c,v $
 * Revision 1.4  2015/02/07 16:31:51  alfred
 * - complete TLCs-870 instruction set
 *
 * Revision 1.3  2015/02/01 21:09:37  alfred
 * - one more...
 *
 * Revision 1.2  2015/02/01 20:44:31  alfred
 * - added some more instructions
 *
 * Revision 1.1  2015/01/26 22:19:09  alfred
 * - add more instructions to decoder
 *
 *****************************************************************************/

#include "stdinc.h" 
#include <ctype.h>

#include "dasmdef.h"
#include "codechunks.h"
#include "strutil.h"

static const char Reg8Names[] = "awcbedlh";
static const char *Reg16Names[] = { "wa", "bc", "de", "hl" };
static const char *RelNames[8] =
{
  "z", "nz", "cs", "cc", "le", "gt", "t", "f"
};
static const char *ALUInstr[8] =
{
  "addc", "add", "subb", "sub", "and" , "xor", "or", "cmp"
};

static unsigned nData;

static char *ZeroHexString(char *pBuffer, size_t BufferSize, LargeWord Num)
{
  HexString(pBuffer + 1, BufferSize - 1, Num, 0);
  if (isdigit(pBuffer[1]))
    return pBuffer + 1;
  else
  {
    pBuffer[0] = '0';
    return pBuffer;
  }
}

static Boolean RetrieveData(LargeWord Address, Byte *pBuffer, unsigned Count)
{
  LargeWord Trans;

  while (Count > 0)
  {
    Trans = 0x10000 - Address;
    if (Count < Trans)
      Trans = Count;
    if (!RetrieveCodeFromChunkList(&CodeChunks, Address, pBuffer, Trans))
    {
      fprintf(stderr, "cannot retrieve instruction arg @ 0x%x\n", (Word)(Address + 1));
      return FALSE;
    }
    pBuffer += Trans;
    Count -= Trans;
    Address = (Address + Trans) & 0xffff;
    nData += Trans;
  }
  return TRUE;
}

static void SimpleNextAddress(tDisassInfo *pInfo, LargeWord ThisAddress)
{
  pInfo->NextAddressCount = 1;
  pInfo->NextAddresses[0] = (ThisAddress + pInfo->CodeLen) % 0xffff;
}

static void PrintData(char *pDest, size_t DestSize, const Byte *pData, unsigned DataLen)
{
  char *pRun, NumBuf[40];
  unsigned z;

  pRun = pDest;
  pRun += snprintf(pRun, DestSize, "\tdb\t");
  for (z = 0; z < DataLen; z++)
  {
    if (z)
      pRun += sprintf(pRun, ",");
    pRun += sprintf(pRun, "%sh", ZeroHexString(NumBuf, sizeof(NumBuf), pData[z]));
  }
}

static void RegPrefix(LargeWord Address, tDisassInfo *pInfo, unsigned PrefixLen, unsigned SrcRegIndex)
{
  Byte Opcode, Data[5];
  char NumBuf[40];

  if (!RetrieveData(Address + PrefixLen, &Opcode, 1))
    return;

  switch (Opcode)
  {
    case 0x01:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tswap\t%c",
               Reg8Names[SrcRegIndex]);
      break;
    case 0x02:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tmul\t%c,%c",
               Reg8Names[SrcRegIndex * 2 + 1], Reg8Names[SrcRegIndex * 2]);
      break;
    case 0x03:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdiv\t%s,c",
               Reg16Names[SrcRegIndex]);
      break;
    case 0x04:
      if (SrcRegIndex > 0)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tretn");
      break;
    case 0x06:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;   
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tpop\t%s",
               Reg16Names[SrcRegIndex]);
      break;
    case 0x07:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;   
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tpush\t%s",
               Reg16Names[SrcRegIndex]);
      break;
    case 0x0a:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdaa\t%c",
               Reg8Names[SrcRegIndex]);
      break;
    case 0x0b:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdas\t%c",
               Reg8Names[SrcRegIndex]);
      break;
    case 0x10: case 0x11: case 0x12: case 0x13:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\txch\t%s,%s",
               Reg16Names[Opcode & 3], Reg16Names[SrcRegIndex]);
      break;
    case 0x14: case 0x15: case 0x16: case 0x17:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%s,%s",
               Reg16Names[Opcode & 3], Reg16Names[SrcRegIndex]);
      break;
    case 0x1c:
      pInfo->CodeLen = PrefixLen + 1;   
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tshlc\t%c",
               Reg8Names[SrcRegIndex]);
      break;
    case 0x1d:
      pInfo->CodeLen = PrefixLen + 1;   
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tshrc\t%c",
               Reg8Names[SrcRegIndex]);
      break;
    case 0x1e:
      pInfo->CodeLen = PrefixLen + 1;   
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\trolc\t%c",
               Reg8Names[SrcRegIndex]);
      break;
    case 0x1f:
      pInfo->CodeLen = PrefixLen + 1;   
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\trorc\t%c",
               Reg8Names[SrcRegIndex]);
      break;
    case 0x30: case 0x31: case 0x32: case 0x33:
    case 0x34: case 0x35: case 0x36: case 0x37:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\t%s\twa,%s",
               ALUInstr[Opcode & 7], Reg16Names[SrcRegIndex]);
      break;
    case 0x38: case 0x39: case 0x3a: case 0x3b:
    case 0x3c: case 0x3d: case 0x3e: case 0x3f:
      if (SrcRegIndex > 3)
        goto inv16;
      if (!RetrieveData(Address + PrefixLen + 1, Data, 2))
        return;
      pInfo->CodeLen = PrefixLen + 1 + 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\t%s\t%s,%sh",
               ALUInstr[Opcode & 7], Reg16Names[SrcRegIndex],
               ZeroHexString(NumBuf, sizeof(NumBuf), (((Word)Data[1]) << 8) | Data[0]));
      break;
    case 0x40: case 0x41: case 0x42: case 0x43:
    case 0x44: case 0x45: case 0x46: case 0x47:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tset\t%c.%u",
               Reg8Names[SrcRegIndex], Opcode & 7);
      break;
    case 0x48: case 0x49: case 0x4a: case 0x4b:
    case 0x4c: case 0x4d: case 0x4e: case 0x4f:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tclr\t%c.%u",
               Reg8Names[SrcRegIndex], Opcode & 7);
      break;
    case 0x58: case 0x59: case 0x5a: case 0x5b:
    case 0x5c: case 0x5d: case 0x5e: case 0x5f:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%c,%c",
               Reg8Names[Opcode & 7], Reg8Names[SrcRegIndex]);
      break;
    case 0x60: case 0x61: case 0x62: case 0x63:
    case 0x64: case 0x65: case 0x66: case 0x67:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address); 
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\t%s\ta,%c",
               ALUInstr[Opcode & 7], Reg8Names[SrcRegIndex]);
      break;
    case 0x68: case 0x69: case 0x6a: case 0x6b:
    case 0x6c: case 0x6d: case 0x6e: case 0x6f:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address); 
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\t%s\t%c,a",
               ALUInstr[Opcode & 7], Reg8Names[SrcRegIndex]);
      break;
    case 0x70: case 0x71: case 0x72: case 0x73:
    case 0x74: case 0x75: case 0x76: case 0x77:
      if (!RetrieveData(Address + PrefixLen + 1, Data, 1))
        return;
      pInfo->CodeLen = PrefixLen + 1 + 1;
      SimpleNextAddress(pInfo, Address); 
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\t%s\t%c,%s",
               ALUInstr[Opcode & 7], Reg8Names[SrcRegIndex],
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x82: case 0x83:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tset\t(%s).%c",
               Reg16Names[Opcode & 3], Reg8Names[SrcRegIndex]);
      break;
    case 0x8a: case 0x8b:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tclr\t(%s).%c",
               Reg16Names[Opcode & 3], Reg8Names[SrcRegIndex]);
      break;
    case 0x92: case 0x93:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tcpl\t(%s).%c",
               Reg16Names[Opcode & 3], Reg8Names[SrcRegIndex]);
      break;
    case 0x9a: case 0x9b:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t(%s).%c,cf",
               Reg16Names[Opcode & 3], Reg8Names[SrcRegIndex]);
      break;
    case 0x9e: case 0x9f:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\tcf,(%s).%c",
               Reg16Names[Opcode & 3], Reg8Names[SrcRegIndex]);
      break;
    case 0xa8: case 0xa9: case 0xaa: case 0xab:
    case 0xac: case 0xad: case 0xae: case 0xaf:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\txch\t%c,%c",
               Reg8Names[Opcode & 7], Reg8Names[SrcRegIndex]);
      break;
    case 0xc0: case 0xc1: case 0xc2: case 0xc3:
    case 0xc4: case 0xc5: case 0xc6: case 0xc7:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tcpl\t%c.%u",
               Reg8Names[SrcRegIndex], Opcode & 7);
      break;
    case 0xc8: case 0xc9: case 0xca: case 0xcb:
    case 0xcc: case 0xcd: case 0xce: case 0xcf:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%c.%u,cf",
               Reg8Names[SrcRegIndex], Opcode & 7);
      break;
    case 0xd0: case 0xd1: case 0xd2: case 0xd3:
    case 0xd4: case 0xd5: case 0xd6: case 0xd7:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\txor\tcf,%c.%u",
               Reg8Names[SrcRegIndex], Opcode & 7);
      break;
    case 0xd8: case 0xd9: case 0xda: case 0xdb:
    case 0xdc: case 0xdd: case 0xde: case 0xdf:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\tcf,%c.%u",
               Reg8Names[SrcRegIndex], Opcode & 7);
      break;
    inv16:
    case 0xfa:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\tsp,%s",
               Reg16Names[Opcode & 3]);
      break;
    case 0xfb:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%s,sp",
               Reg16Names[Opcode & 3]);
      break;
    case 0xfc:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tcall\t%s ; need help",
               Reg16Names[Opcode & 3]);
      break;
    case 0xfe:
      if (SrcRegIndex > 3)
        goto inv16;
      pInfo->CodeLen = PrefixLen + 1;
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tjp\t%s ; need help",
               Reg16Names[Opcode & 3]);
      break;
    default:
      printf("unknown reg prefix opcode 0x%02x @ %x\n", Opcode, (Word)Address);
      pInfo->CodeLen = PrefixLen + 1;
      pInfo->NextAddressCount = 0;
      RetrieveData(Address, Data, pInfo->CodeLen);
      nData -= pInfo->CodeLen;
      PrintData(pInfo->SrcLine, sizeof(pInfo->SrcLine), Data, pInfo->CodeLen);
      break;
  }
}

static void MemPrefix(LargeWord Address, tDisassInfo *pInfo, unsigned PrefixLen, const char *pPrefixString)
{
  Byte Opcode, Data[5];
  char NumBuf[40];

  if (!RetrieveData(Address + PrefixLen, &Opcode, 1))
    return;

  switch (Opcode)
  {
    case 0x08:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\trold\ta,%s",
               pPrefixString);
      break;
    case 0x09:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\trord\ta,%s",
               pPrefixString);
      break;
    case 0x10: case 0x11: case 0x12: case 0x13:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%s,%s",
               pPrefixString, Reg16Names[Opcode & 3]);
      break;
    case 0x14: case 0x15: case 0x16: case 0x17:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%s,%s",
               Reg16Names[Opcode & 3], pPrefixString);
      break;
    case 0x20:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tinc\t%s",
               pPrefixString);
      break;
    case 0x26:
      if (!RetrieveData(Address + PrefixLen + 1, Data, 1))
        return;
      pInfo->CodeLen = PrefixLen + 1 + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t(%sh),%s",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]), pPrefixString);
      break;
    case 0x27:
      pInfo->CodeLen = PrefixLen + 1;
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t(hl),%s",
               pPrefixString);
      break;
    case 0x28:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdec\t%s",
               pPrefixString);
      break;
    case 0x2c:
      if (!RetrieveData(Address + PrefixLen + 1, Data, 1))
        return;
      pInfo->CodeLen = PrefixLen + 1 + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%s,%sh",
               pPrefixString, ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x2f:
      if (!RetrieveData(Address + PrefixLen + 1, Data, 1))
        return;
      pInfo->CodeLen = PrefixLen + 1 + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tmcmp\t%s,%sh",
               pPrefixString, ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x40: case 0x41: case 0x42: case 0x43:
    case 0x44: case 0x45: case 0x46: case 0x47:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tset\t%s.%u",
               pPrefixString, Opcode & 7);
      break;
    case 0x48: case 0x49: case 0x4a: case 0x4b:
    case 0x4c: case 0x4d: case 0x4e: case 0x4f:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tclr\t%s.%u",
               pPrefixString, Opcode & 7);
      break;
    case 0x50: case 0x51: case 0x52: case 0x53:
    case 0x54: case 0x55: case 0x56: case 0x57:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%s,%c",
               pPrefixString, Reg8Names[Opcode & 7]);
      break;
    case 0x58: case 0x59: case 0x5a: case 0x5b:
    case 0x5c: case 0x5d: case 0x5e: case 0x5f:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%c,%s",
               Reg8Names[Opcode & 7], pPrefixString);
      break;
    case 0x60: case 0x61: case 0x62: case 0x63:
    case 0x64: case 0x65: case 0x66: case 0x67:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\t%s\t%s,(hl)",
               ALUInstr[Opcode & 7], pPrefixString);
      break;
    case 0x70: case 0x71: case 0x72: case 0x73:
    case 0x74: case 0x75: case 0x76: case 0x77:
      if (!RetrieveData(Address + PrefixLen + 1, Data, 1))
        return;
      pInfo->CodeLen = PrefixLen + 1 + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\t%s\t%s,%sh",
               ALUInstr[Opcode & 7], pPrefixString,
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x78: case 0x79: case 0x7a: case 0x7b:
    case 0x7c: case 0x7d: case 0x7e: case 0x7f:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\t%s\ta,%s",
               ALUInstr[Opcode & 7], pPrefixString);
      break;
    case 0xa8: case 0xa9: case 0xaa: case 0xab:
    case 0xac: case 0xad: case 0xae: case 0xaf:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address); 
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\txch\t%c,%s",
               Reg8Names[Opcode & 7], pPrefixString);
      break;
    case 0xc0: case 0xc1: case 0xc2: case 0xc3:   
    case 0xc4: case 0xc5: case 0xc6: case 0xc7:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address); 
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tcpl\t%s.%u",
               pPrefixString, Opcode & 7);
      break;
    case 0xc8: case 0xc9: case 0xca: case 0xcb:   
    case 0xcc: case 0xcd: case 0xce: case 0xcf:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address); 
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%s.%u,cf",
               pPrefixString, Opcode & 7);
      break;
    case 0xd0: case 0xd1: case 0xd2: case 0xd3:   
    case 0xd4: case 0xd5: case 0xd6: case 0xd7:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address); 
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\txor\tcf,%s.%u",
               pPrefixString, Opcode & 7);
      break;
    case 0xd8: case 0xd9: case 0xda: case 0xdb:   
    case 0xdc: case 0xdd: case 0xde: case 0xdf:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address); 
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\tcf,%s.%u",
               pPrefixString, Opcode & 7);
      break;
    case 0xfc:
      pInfo->CodeLen = PrefixLen + 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tcall\t%s ; need help",
               pPrefixString);
      break;
    case 0xfe:
      pInfo->CodeLen = PrefixLen + 1;
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tjp\t%s ; need help",
               pPrefixString);
      break;
    default:
      printf("unknown mem opcode 0x%02x @ %x\n", Opcode, (Word)Address);
      pInfo->CodeLen = PrefixLen + 1;
      pInfo->NextAddressCount = 0;
      RetrieveData(Address, Data, pInfo->CodeLen);
      nData -= pInfo->CodeLen;
      PrintData(pInfo->SrcLine, sizeof(pInfo->SrcLine), Data, pInfo->CodeLen);
      break;
  }
}

void Disassemble(LargeWord Address, tDisassInfo *pInfo)
{
  Byte Opcode, Data[10];
  Word Arg;
  char NumBuf[40], NumBuf2[40];
  LargeInt Dist;
  unsigned Vector;

  pInfo->CodeLen = 0;
  pInfo->NextAddressCount = 0;
  pInfo->SrcLine[0] = '\0';

  if (!RetrieveCodeFromChunkList(&CodeChunks, Address, &Opcode, 1))
  {
    fprintf(stderr, "cannot retrieve code @ 0x%x\n", (Word)Address);
    return;
  }
  nData = 1;

  switch (Opcode)
  {
    case 0x00:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tnop");
      break;
    case 0x01:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tswap\ta");
      break;
    case 0x02:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tmul\tw,a");
      break;
    case 0x03:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdiv\twa,c");
      break;
    case 0x04:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\treti");
      break;
    case 0x05:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tret");
      break;
    case 0x06:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tpop\tpsw");
      break;
    case 0x07:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tpush\tpsw");
      break;
    case 0x0a:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdaa\ta");
      break;
    case 0x0b:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdas\ta");
      break;
    case 0x0c:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tclr\tcf");
      break;
    case 0x0d:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tset\tcf");
      break;
    case 0x0e:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tcpl\tcf");
      break;
    case 0x0f:
      if (!RetrieveData(Address + 1, Data, 1)) 
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\trbs,%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x10: case 0x11: case 0x12: case 0x13:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tinc\t%s",
               Reg16Names[Opcode & 3]);
      break;
    case 0x14: case 0x15: case 0x16: case 0x17:
      if (!RetrieveData(Address + 1, Data, 2))
        return;
      pInfo->CodeLen = 3;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%s,%sh",
               Reg16Names[Opcode & 3],
               ZeroHexString(NumBuf, sizeof(NumBuf), (((Word)Data[1]) << 8) | Data[0]));
      break;
    case 0x18: case 0x19: case 0x1a: case 0x1b:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdec\t%s",
               Reg16Names[Opcode & 3]);
      break;
    case 0x1c:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tshlc\ta");
      break;
    case 0x1d:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tshrc\ta");
      break;
    case 0x1e:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\trolc\ta");
      break;
    case 0x1f:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\trorc\ta");
      break;
    case 0x20:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tinc\t(%sh)",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x21:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tinc\t(hl)");
      break;
    case 0x22:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\ta,(%sh)",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x23:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\ta,(hl)");
      break;
    case 0x24:
      if (!RetrieveData(Address + 1, Data, 3))
        return;
      pInfo->CodeLen = 4;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tldw\t(%sh),%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]),
               ZeroHexString(NumBuf, sizeof(NumBuf), (((Word)Data[2]) << 8) | Data[1]));
      break;
    case 0x25:
      if (!RetrieveData(Address + 1, Data, 2))
        return;
      pInfo->CodeLen = 3;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tldw\t(hl),%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), (((Word)Data[1]) << 8) | Data[0]));
      break;
    case 0x26:
      if (!RetrieveData(Address + 1, Data, 2))
        return;
      pInfo->CodeLen = 3;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t(%sh),(%sh)",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[1]),   
               ZeroHexString(NumBuf2, sizeof(NumBuf2), Data[0]));
      break;
    case 0x28:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdec\t(%sh)",
               ZeroHexString(NumBuf2, sizeof(NumBuf2), Data[0]));
      break;
    case 0x29:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdec\t(hl)");
      break;
    case 0x2a:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t(%sh),a",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x2b:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t(hl),a");
      break;
    case 0x2c:
      if (!RetrieveData(Address + 1, Data, 2))
        return;
      pInfo->CodeLen = 3;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t(%sh),%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]),
               ZeroHexString(NumBuf2, sizeof(NumBuf2), Data[1]));
      break;  
    case 0x2d:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t(hl),%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;    
    case 0x2e:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tclr\t(%sh)",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x2f:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tclr\t(hl)");
      break;
    case 0x30: case 0x31: case 0x32: case 0x33:
    case 0x34: case 0x35: case 0x36: case 0x37:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%c,%sh",
               Reg8Names[Opcode & 7], ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x40: case 0x41: case 0x42: case 0x43:
    case 0x44: case 0x45: case 0x46: case 0x47:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tset\t(%sh).%u",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]), Opcode & 7);
      break;
    case 0x48: case 0x49: case 0x4a: case 0x4b:
    case 0x4c: case 0x4d: case 0x4e: case 0x4f:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tclr\t(%sh).%u",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]), Opcode & 7);
      break;
    case 0x50: case 0x51: case 0x52: case 0x53:
    case 0x54: case 0x55: case 0x56: case 0x57:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\ta,%c",
               Reg8Names[Opcode & 7]);
      break;
    case 0x58: case 0x59: case 0x5a: case 0x5b:
    case 0x5c: case 0x5d: case 0x5e: case 0x5f:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\t%c,a",
               Reg8Names[Opcode & 7]);
      break;
    case 0x60: case 0x61: case 0x62: case 0x63:
    case 0x64: case 0x65: case 0x66: case 0x67:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tinc\t%c",
               Reg8Names[Opcode & 7]);
      break;
    case 0x68: case 0x69: case 0x6a: case 0x6b:
    case 0x6c: case 0x6d: case 0x6e: case 0x6f:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdec\t%c",
               Reg8Names[Opcode & 7]);
      break;
    case 0x70: case 0x71: case 0x72: case 0x73:
    case 0x74: case 0x75: case 0x76: case 0x77:
      if (!RetrieveData(Address + 1, Data, 1)) 
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\t%s\ta,%sh",
               ALUInstr[Opcode & 7], ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x78: case 0x79: case 0x7a: case 0x7b:
    case 0x7c: case 0x7d: case 0x7e: case 0x7f:
      if (!RetrieveData(Address + 1, Data, 1)) 
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\t%s\ta,(%sh)",
               ALUInstr[Opcode & 7],
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      break;
    case 0x80: case 0x81: case 0x82: case 0x83:
    case 0x84: case 0x85: case 0x86: case 0x87:
    case 0x88: case 0x89: case 0x8a: case 0x8b:
    case 0x8c: case 0x8d: case 0x8e: case 0x8f:
    case 0x90: case 0x91: case 0x92: case 0x93:
    case 0x94: case 0x95: case 0x96: case 0x97:
    case 0x98: case 0x99: case 0x9a: case 0x9b:
    case 0x9c: case 0x9d: case 0x9e: case 0x9f:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      Dist = Opcode & 0x1f;
      if (Dist & 0x10)
        Dist = Dist - 32;
      pInfo->NextAddresses[pInfo->NextAddressCount++] = (Address + 2 + Dist) & 0xffff;
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tjrs\tt,%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), pInfo->NextAddresses[1]));
      break;
    case 0xa0: case 0xa1: case 0xa2: case 0xa3:
    case 0xa4: case 0xa5: case 0xa6: case 0xa7:
    case 0xa8: case 0xa9: case 0xaa: case 0xab:
    case 0xac: case 0xad: case 0xae: case 0xaf:
    case 0xb0: case 0xb1: case 0xb2: case 0xb3:
    case 0xb4: case 0xb5: case 0xb6: case 0xb7:
    case 0xb8: case 0xb9: case 0xba: case 0xbb:
    case 0xbc: case 0xbd: case 0xbe: case 0xbf:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      Dist = Opcode & 0x1f;
      if (Dist & 0x10)
        Dist = Dist - 32;
      pInfo->NextAddresses[pInfo->NextAddressCount++] = (Address + 2 + Dist) & 0xffff;
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tjrs\tf,%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), pInfo->NextAddresses[1]));
      break;
    case 0xc0: case 0xc1: case 0xc2: case 0xc3:
    case 0xc4: case 0xc5: case 0xc6: case 0xc7:
    case 0xc8: case 0xc9: case 0xca: case 0xcb:
    case 0xcc: case 0xcd: case 0xce: case 0xcf:
      Vector = Opcode & 15;
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      if (RetrieveData(0xffc0 + (Vector << 1), Data, 2))
        pInfo->NextAddresses[pInfo->NextAddressCount++] = (((Word)Data[1]) << 8) | Data[0];
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tcallv\t%u\t ; %sh", Vector,
               ZeroHexString(NumBuf, sizeof(NumBuf), pInfo->NextAddresses[1]));
      nData -= 2;
      break;
    case 0xd0: case 0xd1: case 0xd2: case 0xd3:
    case 0xd4: case 0xd5: case 0xd6: case 0xd7:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      Dist = Data[0];
      if (Dist & 0x80)
        Dist -= 256;
      pInfo->NextAddresses[pInfo->NextAddressCount++] = (Address + 2 + Dist) & 0xffff;
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tjr\t%s,%sh",
               RelNames[Opcode & 7],
               ZeroHexString(NumBuf, sizeof(NumBuf), pInfo->NextAddresses[1]));
      break;
    case 0xd8: case 0xd9: case 0xda: case 0xdb:
    case 0xdc: case 0xdd: case 0xde: case 0xdf:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\ttest\t(%sh).%u",
               ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]), Opcode & 7);
      break;
    case 0xe0:
    case 0xf0:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      snprintf(NumBuf2, sizeof(NumBuf2), "(%sh)", ZeroHexString(NumBuf, sizeof(NumBuf), Data[0]));
      MemPrefix(Address, pInfo, 2, NumBuf2);
      break;
    case 0xe1:
    case 0xf1:
      MemPrefix(Address, pInfo, 1, "(pc+a)");
      break;
    case 0xe2:
    case 0xf2:
      MemPrefix(Address, pInfo, 1, "(de)");
      break;
    case 0xe3:
    case 0xf3:
      MemPrefix(Address, pInfo, 1, "(hl)");
      break;
    case 0xe4:
    case 0xf4:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      Dist = Data[0];
      if (Dist & 0x80)
        Dist -= 256;
      snprintf(NumBuf2, sizeof(NumBuf2), "(hl%s%d)",
               Dist < 0 ? "" : "+", (int)Dist);
      MemPrefix(Address, pInfo, 2, NumBuf2);
      break;
    case 0xe5:
    case 0xf5:
      MemPrefix(Address, pInfo, 1, "(hl+c)");
      break;
    case 0xe6:
    case 0xf6:
      MemPrefix(Address, pInfo, 1, "(hl+)");
      break;
    case 0xe7:
    case 0xf7:
      MemPrefix(Address, pInfo, 1, "(-hl)");
      break;
    case 0xe8: case 0xe9: case 0xea: case 0xeb:
    case 0xec: case 0xed: case 0xee: case 0xef:
      RegPrefix(Address, pInfo, 1, Opcode & 7);
      break;
    case 0xfa:
      if (!RetrieveData(Address + 1, Data, 2))
        return;
      pInfo->CodeLen = 3;
      SimpleNextAddress(pInfo, Address);
      Arg = (((Word)Data[1]) << 8) | Data[0];
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tld\tsp,%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), Arg));
      break;
    case 0xfb:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      Dist = Data[0];
      if (Dist & 0x80)
        Dist -= 256;
      pInfo->NextAddresses[pInfo->NextAddressCount++] = (Address + 2 + Dist) & 0xffff;
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tjr\t%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), pInfo->NextAddresses[0]));
      break;
    case 0xfc:
      if (!RetrieveData(Address + 1, Data, 2))
        return;
      pInfo->CodeLen = 3;
      SimpleNextAddress(pInfo, Address);
      pInfo->NextAddresses[pInfo->NextAddressCount++] = (((Word)Data[1]) << 8) | Data[0];
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tcall\t%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), pInfo->NextAddresses[1]));
      break;
    case 0xfd:
      if (!RetrieveData(Address + 1, Data, 1))
        return;
      pInfo->CodeLen = 2;
      SimpleNextAddress(pInfo, Address);
      pInfo->NextAddresses[pInfo->NextAddressCount++] = 0xff00 + Data[0];
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tcallp\t%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), pInfo->NextAddresses[1]));
      break;
    case 0xfe:
      if (!RetrieveData(Address + 1, Data, 2))
        return;
      pInfo->CodeLen = 3;
      pInfo->NextAddressCount = 1;
      pInfo->NextAddresses[0] = (((Word)Data[1]) << 8) | Data[0];
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tjp\t%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), pInfo->NextAddresses[0]));
      break;
    case 0xff:
      pInfo->CodeLen = 1;
      SimpleNextAddress(pInfo, Address);
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tswi");
      break;
    default:
      printf("unknown opcode 0x%02x @ %x\n", Opcode, (Word)Address);
      pInfo->CodeLen = 1;
      pInfo->NextAddressCount = 0;
      snprintf(pInfo->SrcLine, sizeof(pInfo->SrcLine), "\tdb\t%sh",
               ZeroHexString(NumBuf, sizeof(NumBuf), Opcode));
  }

  if (nData != pInfo->CodeLen)
    sprintf(pInfo->SrcLine + strlen(pInfo->SrcLine), " ; ouch %u != %u", nData, pInfo->CodeLen);
}
