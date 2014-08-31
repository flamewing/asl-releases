#include <stdio.h>
#include <iostream>
#include <vector>
#include <string.h>

enum { ModReg, ModIReg, ModDisp16, ModDisp32, ModImm, ModAbs16, ModAbs32, ModPCRel16, ModPCRel32,
       ModPop, ModPush, ModRegChain, ModPCChain, ModAbsChain, ModeCnt };
#define MModReg      (1 << ModReg)
#define MModIReg     (1 << ModIReg)
#define MModDisp16   (1 << ModDisp16)
#define MModDisp32   (1 << ModDisp32)
#define MModImm      (1 << ModImm)
#define MModAbs16    (1 << ModAbs16)
#define MModAbs32    (1 << ModAbs32)
#define MModPCRel16  (1 << ModPCRel16)
#define MModPCRel32  (1 << ModPCRel32)
#define MModPop      (1 << ModPop)
#define MModPush     (1 << ModPush)
#define MModRegChain (1 << ModRegChain)
#define MModPCChain  (1 << ModPCChain)
#define MModAbsChain (1 << ModAbsChain)

#define Mask_RegOnly    (MModReg)
#define Mask_AllShort   (MModReg+MModIReg+MModDisp16+MModImm+MModAbs16+MModAbs32+MModPCRel16+MModPCRel32+MModPop+MModPush+MModPCChain+MModAbsChain)
#define Mask_AllGen     (Mask_AllShort+MModDisp32+MModRegChain)
#define Mask_NoImmShort (Mask_AllShort-MModImm)
#define Mask_NoImmGen   (Mask_AllGen-MModImm)
#define Mask_MemShort   (Mask_NoImmShort-MModReg)
#define Mask_MemGen     (Mask_NoImmGen-MModReg)

#define Mask_Source     (Mask_AllGen-MModPush)
#define Mask_Dest       (Mask_NoImmGen-MModPop)
#define Mask_PureDest   (Mask_NoImmGen-MModPush-MModPop)
#define Mask_PureMem    (Mask_MemGen-MModPush-MModPop)

enum { Size8, Size16, Size32, SizeCnt };
#define Mask_Size8 (1 << Size8)
#define Mask_Size16 (1 << Size16)
#define Mask_Size32 (1 << Size32)
#define Mask_AllSize (Mask_Size8 | Mask_Size16 | Mask_Size32)

using namespace std;

/* ------------------------------------ */

class cSizeMask
{
public:
  cSizeMask(unsigned SizeMask);
  ~cSizeMask(void);

  cSizeMask operator++(int);
  friend ostream& operator<< (ostream &stream, const cSizeMask &Mask);

  unsigned GetCurrSize(void) const { return m_Size; }
  bool End(void) const { return m_End; }

protected:
  unsigned m_SizeMask;
  unsigned m_Size;
  bool m_End;
};

cSizeMask::cSizeMask(unsigned SizeMask)
         : m_SizeMask(SizeMask), m_End(false)
{
  m_Size = Size8;
  while ((!(1 << m_Size & m_SizeMask)) && (m_Size < SizeCnt))
    m_Size++;
  if (m_Size >= SizeCnt)
    m_End = true;
}

cSizeMask::~cSizeMask(void)
{
}

cSizeMask cSizeMask::operator++(int)
{
  if (m_End)
    return *this;

  do
  { 
    m_Size++;
  }
  while ((m_Size < SizeCnt) && (!((1 << m_Size) & m_SizeMask)));
  if (m_Size >= SizeCnt)
    m_End = true;

  return *this;
}

ostream& operator<< (ostream &stream, const cSizeMask &Mask)
{
  switch (Mask.m_Size)
  {
    case Size8: stream << ".b"; break;
    case Size16: stream << ".h"; break;
    case Size32: stream << ".w"; break;
    default: stream << ".?";
  }
  return stream;
}

/* ------------------------------------- */

class cGenMask : public cSizeMask
{
public:
  cGenMask(unsigned ModeMask, unsigned SizeMask);
  cGenMask(unsigned ModeMask);
  ~cGenMask(void);

  cGenMask operator++(int);
  friend ostream& operator<< (ostream &stream, const cGenMask &Mask);
  unsigned GetCurrMode(void) const { return m_Mode; }

private:
  bool m_NoSizeMask;
  unsigned m_ModeMask;
  unsigned m_Mode, m_FirstMode;
};

cGenMask::cGenMask(unsigned ModeMask, unsigned SizeMask)
        : m_NoSizeMask(false), cSizeMask(SizeMask), m_ModeMask(ModeMask)
{
  if (!m_End)
  {
    m_Mode = 0;
    while ((!(1 << m_Mode & m_ModeMask)) && (m_Mode < ModeCnt))
      m_Mode++;
    if (m_Mode >= ModeCnt)
      m_End = true;
    else
      m_FirstMode = m_Mode;
  }
}

cGenMask::cGenMask(unsigned ModeMask)
        : cGenMask(ModeMask, 1)
{
  m_NoSizeMask = true;
}

cGenMask::~cGenMask(void)
{
}

cGenMask cGenMask::operator++(int)
{
  if (m_End)
    return *this;

  do
  {
    m_Mode++;
  }
  while ((m_Mode < ModeCnt) && (!((1 << m_Mode) & m_ModeMask)));
  if (m_Mode >= ModeCnt)
  {
    m_Mode = m_FirstMode;
    (*(reinterpret_cast<cSizeMask*>(this)))++;
  }
  return *this;
}

ostream& operator<< (ostream &stream, const cGenMask &Mask)
{
  switch (Mask.m_Mode)
  {
    case ModReg: stream << "R1"; break;
    case ModIReg: stream << "@R2"; break;
    case ModDisp16: stream << "@(1234,R3)"; break;
    case ModDisp32: stream << "@(12345678,R4)"; break;
    case ModImm: stream << "#42"; break;
    case ModAbs16: stream << "@1234"; break;
    case ModAbs32: stream << "@12345678"; break;
    case ModPCRel16: stream << "@($+4,PC)"; break;
    case ModPCRel32: stream << "@($+100000,PC)"; break;
    case ModPop: stream << "@SP+"; break;
    case ModPush: stream << "@-SP"; break;
    case ModRegChain: stream << "@@(R6,10)"; break;
    case ModPCChain: stream << "@@(PC,$)"; break;
    case ModAbsChain: stream << "@@(1234)"; break;
    default: stream << "<" << Mask.m_Mode << ">";
  }
  if (!Mask.m_NoSizeMask)
    stream << *(reinterpret_cast<const cSizeMask*>(&Mask));

  return stream;
}

/* ------------------------------------- */

class cDispMask : public cSizeMask
{
public:
  cDispMask(unsigned SizeMask);
  ~cDispMask(void);

  friend ostream& operator<< (ostream&, const cDispMask &Mask);
};

cDispMask::cDispMask(unsigned SizeMask)
         : cSizeMask(SizeMask)
{
}

cDispMask::~cDispMask(void)
{
}

ostream& operator<< (ostream &stream, const cDispMask &Mask)
{
  switch (Mask.m_Size)
  {
    case Size8: stream << "$+10"; break;
    case Size16: stream << "$+300"; break;
    case Size32: stream << "$+3000000"; break;
  }
  stream << *(reinterpret_cast<const cSizeMask*>(&Mask));
  return stream;
}

/* ------------------------------------- */

class cInstrList
{
public:
  cInstrList(vector<const char*> &InstList);
  ~cInstrList(void);

  cInstrList operator++(int) { ++m_it; return *this; }
  bool End(void) const { return m_it == m_InstList.end(); }
  friend ostream& operator<< (ostream &stream, const cInstrList &List);

  const char *GetCurr(void) const { return *m_it; };

private:
  vector<const char*> m_InstList;
  vector<const char*>::iterator m_it;
};

cInstrList::cInstrList(vector<const char*> &InstList)
          : m_InstList(InstList), m_it(m_InstList.begin())
{
}

cInstrList::~cInstrList(void)
{
}

ostream& operator<< (ostream &stream, const cInstrList &List)
{
  stream << *List.m_it;
  return stream;
}

/* ------------------------------------- */

int main(int argc, char **argv)
{
  cout << "\tcpu\tm16" << endl << endl;
  cout << "\tpage\t0" << endl << endl;

  {
    vector<const char*> Instructions({"nop", "pib", "rie", "rrng", "rts", "stctx", "reit", "stop", "sleep"});

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      cout << "\t" << InstrList << endl;
    cout << endl;
  }

  for (cGenMask SrcMask(Mask_Source, Mask_AllSize); !SrcMask.End(); SrcMask++)
    for (cGenMask DestMask(Mask_AllGen & ~MModPop, Mask_AllSize); !DestMask.End(); DestMask++)
      cout << "\tmov:g\t" << SrcMask << "," << DestMask << endl;
  for (cGenMask DestMask(Mask_AllGen & ~MModPop, Mask_AllSize); !DestMask.End(); DestMask++)
    cout << "\tmov:e\t#55," << DestMask << endl;
  for (cGenMask SrcMask(Mask_AllShort & ~MModPush, Mask_AllSize); !SrcMask.End(); SrcMask++)
    cout << "\tmov:l\t" << SrcMask << ",r4.w" << endl;
  for (cGenMask DestMask(Mask_AllShort & ~MModPop, Mask_AllSize); !DestMask.End(); DestMask++)
    cout << "\tmov:s\tr5.w," << DestMask << endl;
  for (cGenMask DestMask(Mask_AllGen & ~MModPop, Mask_AllSize); !DestMask.End(); DestMask++)
    cout << "\tmov:z\t#0," << DestMask << endl;
  for (cGenMask DestMask(Mask_AllShort & ~MModPop, Mask_AllSize); !DestMask.End(); DestMask++)
    cout << "\tmov:q\t#4," << DestMask << endl;
  for (cGenMask DestMask(Mask_AllShort & ~MModPop, Mask_AllSize); !DestMask.End(); DestMask++)
    cout << "\tmov:i\t#55h," << DestMask << endl;
  cout << endl;

  {
    vector<const char*> Instructions({"acs", "jmp", "jsr", "pusha" });

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cGenMask StepMask(Mask_PureMem); !StepMask.End(); StepMask++)
        cout << "\t" << InstrList << ":g\t" << StepMask << endl;
  }
  {
    vector<const char*> Instructions({"neg", "not", "pop" });
   
    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cGenMask StepMask(Mask_PureDest); !StepMask.End(); StepMask++)
        cout << "\t" << InstrList << ":g\t" << StepMask << endl;
  }
  {
    vector<const char*> Instructions({"ldctx" });
  
    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cGenMask StepMask(MModIReg | MModDisp16 | MModDisp32 |
                             MModAbs16 | MModAbs32 | MModPCRel16 | MModPCRel32); !StepMask.End(); StepMask++)
        cout << "\t" << InstrList << ":g\t" << StepMask << endl;
  }
  {
    vector<const char*> Instructions({"ldpsb", "ldpsm" });
   
    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cGenMask StepMask(Mask_Source); !StepMask.End(); StepMask++)
        cout << "\t" << InstrList << ":g\t" << StepMask << endl;
  }
  {
    vector<const char*> Instructions({"push" });
   
    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cGenMask StepMask(Mask_Source & ~MModPop); !StepMask.End(); StepMask++)
        cout << "\t" << InstrList << ":g\t" << StepMask << endl;
  }
  {
    vector<const char*> Instructions({"stpsb", "stpsm" });
   
    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cGenMask StepMask(Mask_Dest); !StepMask.End(); StepMask++)
        cout << "\t" << InstrList << ":g\t" << StepMask << endl;
  }
  cout << endl;

  {
    vector<const char*> Instructions({"add","sub" });

    for (cGenMask SrcMask(Mask_Source, Mask_AllSize); !SrcMask.End(); SrcMask++)
      for (cGenMask DestMask(Mask_PureDest, Mask_AllSize); !DestMask.End(); DestMask++)
        for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
          cout << "\t" << InstrList << ":g\t" << SrcMask << "," << DestMask << endl;

    for (cGenMask DestMask(Mask_PureDest, Mask_AllSize); !DestMask.End(); DestMask++)
      for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
        cout << "\t" << InstrList << ":e\t#10," << DestMask << endl;

    for (cGenMask SrcMask(Mask_AllShort & ~MModPush, Mask_Size32); !SrcMask.End(); SrcMask++)
      for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
        cout << "\t" << InstrList << ":l\t" << SrcMask << ",r6.w" << endl;

    for (cGenMask DestMask(Mask_AllShort & ~(MModPop | MModPush | MModImm), Mask_AllSize); !DestMask.End(); DestMask++)
      for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
        cout << "\t" << InstrList << ":q\t#5," << DestMask << endl;

    for (cGenMask DestMask(Mask_AllShort & ~(MModPop | MModPush | MModImm), Mask_AllSize); !DestMask.End(); DestMask++)
      for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
        cout << "\t" << InstrList << ":i\t#20," << DestMask << endl;
  }
  cout << endl;

  for (cGenMask SrcMask(Mask_Source, Mask_AllSize); !SrcMask.End(); SrcMask++)
    for (cGenMask DestMask(Mask_NoImmGen & ~MModPush, Mask_AllSize); !DestMask.End(); DestMask++)
      cout << "\tcmp:g\t" << SrcMask << "," << DestMask << endl;
  for (cGenMask DestMask(Mask_NoImmGen & ~MModPush, Mask_AllSize); !DestMask.End(); DestMask++)
    cout << "\tcmp:e\t#55," << DestMask << endl;
  for (cGenMask SrcMask(Mask_AllShort & ~MModPush, Mask_AllSize); !SrcMask.End(); SrcMask++)
    cout << "\tcmp:l\t" << SrcMask << ",r6.w" << endl;
  for (cGenMask DestMask(Mask_Source & ~MModImm, Mask_AllSize); !DestMask.End(); DestMask++)
    cout << "\tcmp:z\t#0," << DestMask << endl;
  for (cGenMask DestMask(Mask_AllShort & ~(MModPush|MModImm), Mask_AllSize); !DestMask.End(); DestMask++)
    cout << "\tcmp:q\t#4," << DestMask << endl;
  for (cGenMask DestMask(Mask_AllShort & ~(MModPush|MModImm), Mask_AllSize); !DestMask.End(); DestMask++)
    cout << "\tcmp:i\t#87," << DestMask << endl;
  cout << endl;

  {
    vector<const char*> Instructions({"addu", "addx", "subu", "subx", "cmpu", "ldc", "ldp", "movu", "rem", "remu", "rot"});

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
    {
      unsigned SrcSizeMask = strcmp(InstrList.GetCurr(), "rot") ? Mask_AllSize : Mask_Size8;
      unsigned DestSizeMask = strcmp(InstrList.GetCurr(), "ldc") ? Mask_AllSize : Mask_Size32;
      unsigned DestAdrMask;

      if (!strcmp(InstrList.GetCurr(), "ldp"))
        DestAdrMask = Mask_PureMem;
      else if (!strcmp(InstrList.GetCurr(), "movu"))
        DestAdrMask = Mask_Dest;
      else if (!strcmp(InstrList.GetCurr(), "cmpu"))
        DestAdrMask = Mask_PureDest | MModPop;
      else
        DestAdrMask = Mask_PureDest;

      for (cGenMask SrcMask(Mask_Source, SrcSizeMask); !SrcMask.End(); SrcMask++)
      {
        for (cGenMask DestMask(DestAdrMask, DestSizeMask); !DestMask.End(); DestMask++)
          cout << "\t" << InstrList << ":g\t" << SrcMask << "," << DestMask << endl;
      }
      for (cGenMask DestMask(DestAdrMask, DestSizeMask); !DestMask.End(); DestMask++)
        cout << "\t" << InstrList << ":e\t#45," << DestMask << endl;
    }
  }
  cout << endl;

  {
    vector<const char*> Instructions({"and", "or", "xor"});

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
    {
      for (cGenMask SrcMask(Mask_Source, Mask_AllSize); !SrcMask.End(); SrcMask++)
        for (cGenMask DestMask(Mask_Dest & ~MModPush, Mask_AllSize); !DestMask.End(); DestMask++)
          if (SrcMask.GetCurrSize() <= DestMask.GetCurrSize())
            cout << "\t" << InstrList << ":g\t" << SrcMask << "," << DestMask << endl;

      for (cGenMask DestMask(Mask_Dest & ~MModPush, Mask_AllSize); !DestMask.End(); DestMask++)
        cout << "\t" << InstrList << ":e\t" << "#42," << DestMask << endl;

      for (cGenMask DestMask(Mask_NoImmShort & ~(MModPush|MModPop)); !DestMask.End(); DestMask++)
        cout << "\t" << InstrList << ":i\t" << "#42," << DestMask << endl;
    }
  }
  cout << endl;

  {
    vector<const char*> Instructions({"mul", "mulu", "div", "divu"});

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
    {
      for (cGenMask SrcMask(Mask_Source, Mask_AllSize); !SrcMask.End(); SrcMask++)
        for (cGenMask DestMask(Mask_PureDest, Mask_AllSize); !DestMask.End(); DestMask++)
          cout << "\t" << InstrList << ":g\t" << SrcMask << "," << DestMask << endl;

      for (cGenMask DestMask(Mask_Dest & ~MModPush, Mask_AllSize); !DestMask.End(); DestMask++)
        cout << "\t" << InstrList << ":e\t" << "#120," << DestMask << endl;

      if (strlen(InstrList.GetCurr()) < 4)
        cout << "\t" << InstrList << ":r\tr5.w,r12.w" << endl;
    }
  }
  cout << endl;

  {
    vector<const char*> Instructions({"getb0","getb1","getb2","geth0"});

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cGenMask DestMask(Mask_Dest); !DestMask.End(); DestMask++)
        cout << "\t" << InstrList << "\tr14," << DestMask << endl;
  }
  {
    vector<const char*> Instructions({"putb0","putb1","putb2","puth0"});
   
    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cGenMask SrcMask(Mask_Source); !SrcMask.End(); SrcMask++)
        cout << "\t" << InstrList << "\t" << SrcMask << ",r14" << endl;
  }
  cout << endl;

  for (cGenMask SrcMask(Mask_PureMem); !SrcMask.End(); SrcMask++)
    for (cGenMask DestMask(Mask_Dest, Mask_Size32); !DestMask.End(); DestMask++)
      cout << "\tmova:g\t" << SrcMask << "," << DestMask << endl;
  for (cGenMask SrcMask(MModDisp16); !SrcMask.End(); SrcMask++)
    cout << "\tmova:r\t" << SrcMask << ",r10.w" << endl;
  cout << endl;

  {
    vector<const char*> Instructions({"qdel", "qins" });

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
    {
      for (cGenMask SrcMask(Mask_PureMem); !SrcMask.End(); SrcMask++)
      {
        unsigned DestOpMask = Mask_PureMem | (strcmp(InstrList.GetCurr(), "qins") ? MModReg : 0);

        for (cGenMask DestMask(DestOpMask); !DestMask.End(); DestMask++)
          cout << "\t" << InstrList << "\t" << SrcMask << "," << DestMask << endl;
      }
    }
  }
  cout << endl;

  for (cGenMask SrcMask(Mask_Source, Mask_AllSize); !SrcMask.End(); SrcMask++)
    for (cGenMask DestMask(Mask_Dest, Mask_AllSize); !DestMask.End(); DestMask++)
      cout << "\trvby\t" << SrcMask << "," << DestMask << endl;
  cout << endl;

  {
    vector<const char*> Instructions({"sha","shl"});

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
    {
      for (cGenMask SrcMask(Mask_Source, Mask_Size8); !SrcMask.End(); SrcMask++)
        for (cGenMask DestMask(Mask_PureMem, Mask_AllSize); !DestMask.End(); DestMask++)
          cout << "\t" << InstrList << ":g\t" << SrcMask << "," << DestMask << endl;

      for (cGenMask DestMask(Mask_PureMem, Mask_AllSize); !DestMask.End(); DestMask++)
        cout << "\t" << InstrList << ":e\t#13," << DestMask << endl;

      for (cGenMask DestMask(Mask_AllShort & ~(MModImm|MModPush|MModPop)); !DestMask.End(); DestMask++)
        cout << "\t" << InstrList << ":q\t#-3," << DestMask << endl;
    }
  }
  cout << endl;

  {
    vector<const char*> Instructions({"shxl", "shxr"});

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cGenMask DestMask(Mask_PureDest, Mask_Size32); !DestMask.End(); DestMask++)
        cout << "\t" << InstrList << ":g\t" << DestMask << endl;
  }
  cout << endl;

  {
    vector<const char*> Instructions({"chk","chk/n","chk/s"});

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cSizeMask SizeMask(Mask_AllSize); !SizeMask.End(); SizeMask++)
        for (cGenMask BoundMask(Mask_MemGen-MModPop-MModPush); !BoundMask.End(); BoundMask++)
          for (cGenMask SrcMask(Mask_Source); !SrcMask.End(); SrcMask++)
            cout << "\t" << InstrList << ":g\t" << BoundMask << SizeMask << "," << SrcMask << SizeMask << ",r14" << SizeMask << endl;
  }
  cout << endl;

  for (cSizeMask SizeMask(Mask_AllSize); !SizeMask.End(); SizeMask++)
    for (cGenMask SrcMask(Mask_Source); !SrcMask.End(); SrcMask++)
      for (cGenMask DestMask(Mask_PureMem); !DestMask.End(); DestMask++)
        cout << "\tcsi\tr7" << SizeMask << "," << SrcMask << SizeMask << "," << DestMask << SizeMask << endl;
  cout << endl;

  {
    vector<const char*> Instructions({ "divx", "mulx" });

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cSizeMask SizeMask(Mask_Size32); !SizeMask.End(); SizeMask++)
        for (cGenMask SrcMask(Mask_Source); !SrcMask.End(); SrcMask++)
          for (cGenMask DestMask(Mask_PureDest); !DestMask.End(); DestMask++)
            cout << "\t" << InstrList << ":g\t" << SrcMask << SizeMask << "," << DestMask << SizeMask << "," << "r7" << SizeMask << endl;
  }
  cout << endl;

  {
    vector<const char*> Instructions({"bclr","bclri","bnot","bset","bseti","btst"});

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
    {
      unsigned BaseSizeMask;

      if (!strcmp(InstrList.GetCurr(), "bclri"))
        BaseSizeMask = Mask_Size8;
      else if (!strcmp(InstrList.GetCurr(), "bseti"))
        BaseSizeMask = Mask_Size8;
      else
        BaseSizeMask = Mask_AllSize;

      for (cGenMask OffsetMask(Mask_Source, Mask_AllSize); !OffsetMask.End(); OffsetMask++)
        for (cGenMask BaseMask(Mask_PureDest, BaseSizeMask); !BaseMask.End(); BaseMask++)
        {
          if ((BaseMask.GetCurrSize() == 0) || (BaseMask.GetCurrMode() == ModReg))
            cout << "\t" << InstrList << ":g\t" << OffsetMask << "," << BaseMask << endl;
        }

      for (cGenMask BaseMask(Mask_PureDest, BaseSizeMask); !BaseMask.End(); BaseMask++)
      {
        if ((BaseMask.GetCurrSize() == 0) || (BaseMask.GetCurrMode() == ModReg))
          cout << "\t" << InstrList << ":e\t#4," << BaseMask << endl;
      }

      if ((strcmp(InstrList.GetCurr(), "bclri"))
       && (strcmp(InstrList.GetCurr(), "bnot")))
      {
        for (cGenMask BaseMask(Mask_AllShort & ~(MModImm | MModPop | MModPush), Mask_Size8); !BaseMask.End(); BaseMask++)
          cout << "\t" << InstrList << ":q\t#4," << BaseMask << endl;
      }
    }
  }
  cout << endl;

  {
    vector<const char*> Instructions({"bfcmp", "bfcmpu", "bfins", "bfinsu" });

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
    {
      for (cGenMask SrcMask(MModImm|MModReg, Mask_Size32); !SrcMask.End(); SrcMask++)
        for (cGenMask WidthMask(MModReg, Mask_Size32); !WidthMask.End(); WidthMask++)
          for (cGenMask OffsetMask(Mask_Source, Mask_AllSize); !OffsetMask.End(); OffsetMask++)
            for (cGenMask BaseMask(Mask_PureMem, Mask_Size32); !BaseMask.End(); BaseMask++)
              cout << "\t" << InstrList << ":g:" << ((SrcMask.GetCurrMode() == ModReg) ? "r" : "i")
                   << "\t" << SrcMask << "," << OffsetMask << "," << WidthMask << "," << BaseMask << endl;

      for (cGenMask SrcMask(MModImm|MModReg, Mask_Size32); !SrcMask.End(); SrcMask++)
        for (cGenMask BaseMask(Mask_PureMem, Mask_Size32); !BaseMask.End(); BaseMask++)
          cout << "\t" << InstrList << ":e:" << ((SrcMask.GetCurrMode() == ModReg) ? "r" : "i")
               << "\t" << SrcMask << ",#22,#7," << BaseMask << endl;
    }
  }
  cout << endl;
 
  {
    vector<const char*> Instructions({"bfext", "bfextu" });

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
    {
      for (cGenMask DestMask(MModReg, Mask_Size32); !DestMask.End(); DestMask++)
        for (cGenMask WidthMask(MModReg, Mask_Size32); !WidthMask.End(); WidthMask++)
          for (cGenMask OffsetMask(Mask_Source, Mask_AllSize); !OffsetMask.End(); OffsetMask++)
            for (cGenMask BaseMask(Mask_PureMem, Mask_Size32); !BaseMask.End(); BaseMask++)
              cout << "\t" << InstrList << ":g\t" << OffsetMask << "," << WidthMask << "," << BaseMask << "," << DestMask << endl;

      for (cGenMask DestMask(MModReg, Mask_Size32); !DestMask.End(); DestMask++)
        for (cGenMask BaseMask(Mask_PureMem, Mask_Size32); !BaseMask.End(); BaseMask++)
          cout << "\t" << InstrList << ":e\t#22,#7," << BaseMask << "," << DestMask << endl;
    }
  }
  cout << endl;

  {  
    vector<const char*> Instructions({"bsch/0", "bsch/1" });

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cGenMask SrcMask(Mask_Source, Mask_Size32); !SrcMask.End(); SrcMask++)
        for (cGenMask OffsetMask(Mask_PureDest, Mask_AllSize); !OffsetMask.End(); OffsetMask++)
           cout << "\t" << InstrList << ":g\t" << SrcMask << "," << OffsetMask << endl;
  }
  cout << endl;
 
  {
    vector<const char*> Instructions({"acb","scb" });

    for (cGenMask StepMask(Mask_AllGen & ~MModPush, Mask_AllSize); !StepMask.End(); StepMask++)
      for (cGenMask LimitMask(Mask_AllGen & ~MModPush, Mask_AllSize); !LimitMask.End(); LimitMask++)
        for (cDispMask DispMask(Mask_AllSize); !DispMask.End(); DispMask++)
          for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
            cout << "\t" << InstrList << ":g\t" << StepMask << ",R12" << *(reinterpret_cast<const cSizeMask*>(&LimitMask)) << "," << LimitMask << "," << DispMask << endl;

    for (cGenMask LimitMask(Mask_AllGen & ~MModPush, Mask_AllSize); !LimitMask.End(); LimitMask++)
      for (cDispMask DispMask(Mask_AllSize); !DispMask.End(); DispMask++)
        for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
          cout << "\t" << InstrList << ":e\t#42,R13" << *(reinterpret_cast<const cSizeMask*>(&LimitMask)) << "," << LimitMask << "," << DispMask << endl;

    for (cDispMask DispMask(Mask_AllSize); !DispMask.End(); DispMask++)
      for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
        cout << "\t" << InstrList << ":q\t#1,R14.w,#23," << DispMask << endl;

    for (cDispMask DispMask(Mask_AllSize); !DispMask.End(); DispMask++)
      for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
        cout << "\t" << InstrList << ":r\t#1,R12.w,R14.w," << DispMask << endl;
  }
  cout << endl;

  {
    vector<const char*> Instructions({"bsr", "bra", "bxs", "bxc", "beq", "bne", "blt", "bge",
                                      "ble", "bgt", "bvs", "bvc", "bms", "bmc", "bfs", "bfc" });

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
    {
      for (cSizeMask SizeMask(Mask_AllSize); !SizeMask.End(); SizeMask++)
        cout << "\t" << InstrList << ":g\t$+6" << SizeMask << endl;
      cout << "\t" << InstrList << ":d\t$+6" << endl;
    }
  }
  cout << endl;  

  {
    vector<const char*> Instructions({"trap/xs", "trap/xc", "trap/eq", "trap/ne", "trap/lt", "trap/ge", "trap/le",
                                      "trap/gt", "trap/vs", "trap/vc", "trap/ms", "trap/mc", "trap/fs", "trap/fc" });

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      cout << "\t" << InstrList << endl;
  }
  cout << endl;

  cout << "\ttrapa\t#11" << endl << endl;

  {
    for (cGenMask SizeMask(MModReg|MModImm, Mask_AllSize); !SizeMask.End(); SizeMask++)
    {
      cout << "\tenter:g\t" << SizeMask << ",r3-r7" << endl;
      cout << "\texitd:g\t" << "r3-r7," << SizeMask << endl;
    }
    cout << "\tenter:g\t#5,r3-r7" << endl;
    cout << "\texitd:g\tr3-r7,#5" << endl;
  }
  cout << endl;

  cout << "\twait\t#2" << endl << endl;

  {
    vector<const char*> Conditions({"ltu","geu","eq","ne","lt","ge","n"});
    vector<const char*> Instructions({"smov","ssch"});
    vector<const char*> Options({"","/f","/b"});

    for (cSizeMask SizeMask(Mask_AllSize); !SizeMask.End(); SizeMask++)
    {
      for (cInstrList ConditionList(Conditions); !ConditionList.End(); ConditionList++)
        cout << "\tscmp" << "/" << ConditionList << SizeMask << endl;
      cout << "\tscmp" << SizeMask << endl;
    }
    cout << endl;

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cSizeMask SizeMask(Mask_AllSize); !SizeMask.End(); SizeMask++)
        for (cInstrList OptionList(Options); !OptionList.End(); OptionList++)
        {
          cout << "\t" << InstrList << OptionList << SizeMask << endl;
          for (cInstrList ConditionList(Conditions); !ConditionList.End(); ConditionList++)
            cout << "\t" << InstrList << "/" << ConditionList << OptionList << SizeMask << endl;
        }
  }
  cout << endl;

  for (cSizeMask SizeMask(Mask_AllSize); !SizeMask.End(); SizeMask++)
    cout << "\tsstr" << SizeMask << endl;
  cout << endl;

  for (cGenMask SrcMask(MModIReg+MModDisp16+MModDisp32+MModAbs16+MModAbs32+MModPCRel16+MModPCRel32+MModPop); !SrcMask.End(); SrcMask++)
    cout << "\tldm\t" << SrcMask << ",r0-r7/r12" << endl;
  for (cGenMask DestMask(MModIReg+MModDisp16+MModDisp32+MModAbs16+MModAbs32+MModPCRel16+MModPCRel32+MModPush); !DestMask.End(); DestMask++)
    cout << "\tstm\tr0-r7/r12," << DestMask << endl;
  cout << endl;

  {
    vector<const char*> Instructions({"stc","stp"});

    for (cInstrList InstrList(Instructions); !InstrList.End(); InstrList++)
      for (cSizeMask SizeMask(strcmp(InstrList.GetCurr(), "stc") ? Mask_AllSize : Mask_Size32); !SizeMask.End(); SizeMask++)
        for (cGenMask SrcMask(Mask_PureMem); !SrcMask.End(); SrcMask++)
          for (cGenMask DestMask(Mask_Dest); !DestMask.End(); DestMask++)
            cout << "\t" << InstrList << "\t" << SrcMask << SizeMask << "," << DestMask << SizeMask << endl;
  }
  cout << endl;

  for (cGenMask SrcMask(MModReg | MModImm); !SrcMask.End(); SrcMask++)
    cout << "\tjrng:g\t" << SrcMask << endl;
  cout << "\tjrng:e\t#17" << endl;
}
