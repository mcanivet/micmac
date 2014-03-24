// File Automatically generated by eLiSe
#ifndef _cB_GCF0_cpp_
#define _cB_GCF0_cpp_

#include "general/all.h"
#include "private/all.h"
#include "cB_GCF0.h"


cB_GCF0::cB_GCF0():
    cElCompiledFonc()
{
   AddIntRef (cIncIntervale("Interv1",0,2));
   AddIntRef (cIncIntervale("Interv2",2,5));
   Close(false);
}



void cB_GCF0::ComputeVal()
{

  mVal = mCompCoord[0]*mCompCoord[1]*mLocA+mLocB;

}


void cB_GCF0::ComputeValDeriv()
{
   double tmp0_ = mCompCoord[1];
   double tmp1_ = mCompCoord[0];

  mVal = tmp1_*tmp0_*mLocA+mLocB;

  mCompDer[0] = tmp0_*mLocA;
  mCompDer[1] = tmp1_*mLocA;
  mCompDer[2] = 0;
  mCompDer[3] = 0;
  mCompDer[4] = 0;
}


void cB_GCF0::ComputeValDerivHessian()
{
   double tmp0_ = mCompCoord[1];
   double tmp1_ = mCompCoord[0];

  mVal = tmp1_*tmp0_*mLocA+mLocB;

  mCompDer[0] = tmp0_*mLocA;
  mCompDer[1] = tmp1_*mLocA;
  mCompDer[2] = 0;
  mCompDer[3] = 0;
  mCompDer[4] = 0;
  mCompHessian[0][0]=   mCompHessian[0][0]= 0;
  mCompHessian[1][0]=   mCompHessian[0][1]= mLocA;
  mCompHessian[1][1]=   mCompHessian[1][1]= 0;
  mCompHessian[2][0]=   mCompHessian[0][2]= 0;
  mCompHessian[2][1]=   mCompHessian[1][2]= 0;
  mCompHessian[2][2]=   mCompHessian[2][2]= 0;
  mCompHessian[3][0]=   mCompHessian[0][3]= 0;
  mCompHessian[3][1]=   mCompHessian[1][3]= 0;
  mCompHessian[3][2]=   mCompHessian[2][3]= 0;
  mCompHessian[3][3]=   mCompHessian[3][3]= 0;
  mCompHessian[4][0]=   mCompHessian[0][4]= 0;
  mCompHessian[4][1]=   mCompHessian[1][4]= 0;
  mCompHessian[4][2]=   mCompHessian[2][4]= 0;
  mCompHessian[4][3]=   mCompHessian[3][4]= 0;
  mCompHessian[4][4]=   mCompHessian[4][4]= 0;
}



void cB_GCF0::SetA(double aVal){ mLocA = aVal;}
void cB_GCF0::SetB(double aVal){ mLocB = aVal;}



double * cB_GCF0::AdrVarLocFromString(const std::string & aName)
{
   if (aName == "A") return & mLocA;
   if (aName == "B") return & mLocB;
   return 0;
}


cElCompiledFonc::cAutoAddEntry cB_GCF0::mTheAuto("cB_GCF0",cB_GCF0::Alloc);


cElCompiledFonc *  cB_GCF0::Alloc()
{  return new cB_GCF0();
}


#endif // _cB_GCF0_cpp_
