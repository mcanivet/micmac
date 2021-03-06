/*Header-MicMac-eLiSe-25/06/2007

    MicMac : Multi Image Correspondances par Methodes Automatiques de Correlation
    eLiSe  : ELements of an Image Software Environnement

    www.micmac.ign.fr

   
    Copyright : Institut Geographique National
    Author : Marc Pierrot Deseilligny
    Contributors : Gregoire Maillet, Didier Boldo.

[1] M. Pierrot-Deseilligny, N. Paparoditis.
    "A multiresolution and optimization-based image matching approach:
    An application to surface reconstruction from SPOT5-HRS stereo imagery."
    In IAPRS vol XXXVI-1/W41 in ISPRS Workshop On Topographic Mapping From Space
    (With Special Emphasis on Small Satellites), Ankara, Turquie, 02-2006.

[2] M. Pierrot-Deseilligny, "MicMac, un lociel de mise en correspondance
    d'images, adapte au contexte geograhique" to appears in 
    Bulletin d'information de l'Institut Geographique National, 2007.

Francais :

   MicMac est un logiciel de mise en correspondance d'image adapte 
   au contexte de recherche en information geographique. Il s'appuie sur
   la bibliotheque de manipulation d'image eLiSe. Il est distibue sous la
   licences Cecill-B.  Voir en bas de fichier et  http://www.cecill.info.


English :

    MicMac is an open source software specialized in image matching
    for research in geographic information. MicMac is built on the
    eLiSe image library. MicMac is governed by the  "Cecill-B licence".
    See below and http://www.cecill.info.

Header-MicMac-eLiSe-25/06/2007*/

#include "cNewO_SolGlobInit_RandomDFS.h"


using namespace SolGlobInit_DFS ;


#define MIN_WEIGHT 0.5
#define MAX_WEIGHT 10.0

extern double DistBase(Pt3dr  aB1,Pt3dr  aB2);
extern double DistanceRot(const ElRotation3D & aR1,const ElRotation3D & aR2,double aBSurH);

void PrintRotation(const ElMatrix<double> Mat,const std::string Msg)
{
	std::cout << " ===R" << Msg << " === \n";
    for (int l=0; l<3; l++){
        for (int c=0; c<3; c++)
            std::cout << Mat(l,c) << " ";

		std::cout << "\n";
	}
}

//=====================================================================================
#ifdef GRAPHVIZ_ENABLED


GVC_t *cSolGlobInit_NRandom::GRAPHVIZ_GVCInit(const std::string& aGName)
{
    std::string Prefix = "-o";

    int SzPref = int(Prefix.size());
    int aSzIn = int(aGName.size());
    int aSzOut = aSzIn + SzPref;
    char * aGNChar = new char[aSzOut];

    //copy the prefix of the name
    for (int i=0; i<SzPref; i++)
        aGNChar[i] = Prefix[i];


    //copy the name from the function argument
    for (int i=0; i<aSzIn; i++)
        aGNChar[SzPref+i] = aGName[i];

    int argc = 4;
    char* dummy_args[] = { (char *)"fn_name",
                           //(char *)"-Kneato",
                           (char *)"-Kdot",
                           aGNChar,
                           (char *)"-Teps" };
    char ** argv = dummy_args;

    GVC_t *gvc = gvContext();

    gvParseArgs(gvc, argc, argv);

	return gvc;
	
}

std::pair<graph_t *,GVC_t *> cSolGlobInit_NRandom::GRAPHVIZ_GraphInit(const std::string& aGName)
{

	GVC_t *gvc = GRAPHVIZ_GVCInit(aGName);

	char * aNG = new char[2];
	aNG[0] = 'g';
    
	graph_t *g = agopen(aNG, Agdirected, 0);

	return std::pair<graph_t*,GVC_t*>(g,gvc);
}

graph_t *cSolGlobInit_NRandom::GRAPHIZ_GraphRead(std::string& aFName)
{
	graph_t *g;

	FILE *fp;

    fp = fopen(aFName.c_str(), "r");
    if (fp) 
    {
        g = agread(fp,NULL);
    }
	else
	{
		ELISE_ASSERT(false,"Could not open the graph file");
		return NULL;
	}

    fclose(fp);

	return g;
}

/* */
void cSolGlobInit_NRandom::GRAPHIZ_GraphSerialize(std::string& aFName,graph_t *g)
{
	FILE *fp;

    fp = fopen(aFName.c_str(), "w");
	agwrite(g,fp);
	fclose(fp);
}


void cSolGlobInit_NRandom::GRAPHIZ_GraphKill(graph_t *g,GVC_t *gvc,std::string aWriteName)
{
	gvLayoutJobs(gvc, g);

    gvRenderJobs(gvc, g);

	gvFreeLayout(gvc, g);

	//serialization of the graph	
	if (aWriteName != "")
	{
		GRAPHIZ_GraphSerialize(aWriteName,g);
	}

    agclose(g);

    gvFreeContext(gvc);

}

void cSolGlobInit_NRandom::GRAPHIZ_NodeInit(graph_t *g,
				      const std::string& aName1,
					  const std::string& aName2,
					  const std::string& aName3)
{
    node_t *ns, *ms;
    char *aNSNName = (char *)aName1.c_str();
    ns = agnode(g,aNSNName,1);


    char *aMSNName = (char *)aName2.c_str();
    ms = agnode(g,aMSNName,1);

    //add the new node
    node_t *os;
    char * aOSNName = (char *)aName3.c_str();

    os = agnode(g,aOSNName,1);

    //add new edges
    //edge_t *e1,*e2,*e3;
    agedge(g, ns, ms, 0, 1);
    agedge(g, ns, os, 0, 1);
    agedge(g, ms, os, 0, 1);

    char * aAttrShape= (char *)"shape";
    char * aShape= (char *)"circle";
    Agsym_t *symShape;
    symShape = agattr(g,AGNODE,aAttrShape,aShape);

    if (symShape)
        printf("The default shape is %s.\n",symShape->defval);

    /* changing colors removed from here; there are specific funcs to do that
	 * char * aC = (char *)"color";
    char * aCR = (char *)"blue";
    char  aCEmpty = ' ';

    agsafeset(ns,aC,aCR,&aCEmpty);
    agsafeset(ms,aC,aCR,&aCEmpty);
    agsafeset(os,aC,aCR,&aCEmpty);

    agsafeset(e1,aC,aCR,&aCEmpty);
    agsafeset(e2,aC,aCR,&aCEmpty);
    agsafeset(e3,aC,aCR,&aCEmpty);*/

}

void cSolGlobInit_NRandom::GRAPHVIZ_NodeAdd(graph_t* g,
				      const std::string& aName1,
					  const std::string& aName2,
					  const std::string& aName3)
{
    node_t *n, *m;
    char *aNNName = (char *)aName1.c_str();
    n = agnode(g,aNNName,0);

    char *aMNName = (char *)aName2.c_str();
    m = agnode(g,aMNName,0);

    //add the new node
    node_t *o;
    char * aONName = (char *)aName3.c_str();

    o = agnode(g,aONName,1);

    //add new edges
    agedge(g, n, o, 0, 1);
    agedge(g, m, o, 0, 1);

}

void cSolGlobInit_NRandom::GRAPHVIZ_EdgeChgColor(graph_t* g,
				                                   const std::string& aName1,
				                                   const std::string& aName2,
												   std::string aColor)
{
	char * aN1Name = (char *)aName1.c_str();
	char * aN2Name = (char *)aName2.c_str();

	node_t *n = agnode(g,aN1Name,0);
	node_t *m = agnode(g,aN2Name,0);

	edge_t *e1;
    e1 = agedge(g, n, m, 0, 1);

	// Change color
	char * aC = (char *)"color";
    char * aCR = (char *)aColor.c_str();
    char  aCEmpty = ' ';


	agsafeset(e1,aC,aCR,&aCEmpty);

}

/* Find node and change its color */
void cSolGlobInit_NRandom::GRAPHVIZ_NodeChgColor(graph_t* g,const std::string& aName)
{
	
    char * aNName = (char *)aName.c_str();


	char * aC = (char *)"color";
    char * aCR = (char *)"blue";
    char  aCEmpty = ' ';

	node_t *n_ = agnode(g,aNName,0);//0 to indicate that it exists
    agsafeset(n_,aC,aCR,&aCEmpty);

}

void cSolGlobInit_NRandom::WriteGraphToFile()
{
	// Clear the sommets
    mVS.clear();

    int aNumCC = 0;


    cNOSolIn_Triplet * aTri0 = mV3[0];
    /*std::cout << "First triplet " << aTri0->KSom(0)->attr().Im()->Name() << " "
                      				<< aTri0->KSom(1)->attr().Im()->Name() << " "
                      				<< aTri0->KSom(2)->attr().Im()->Name() << "\n";*/

	//initialise the graph
    std::pair<graph_t *,GVC_t *> aGGVC = GRAPHVIZ_GraphInit(mGraphName);
    graph_t *g = aGGVC.first;
    GVC_t *gvc = aGGVC.second;

    //initialise the starting node
    GRAPHIZ_NodeInit(g,
                     aTri0->KSom(0)->attr().Im()->Name(),
                     aTri0->KSom(1)->attr().Im()->Name(),
                     aTri0->KSom(2)->attr().Im()->Name());

	// Create a new component
    cNO_CC_TripSom * aNewCC3S = new cNO_CC_TripSom;
    aNewCC3S->mNumCC = aNumCC;
    mVCC.push_back(aNewCC3S);
    std::vector<cNOSolIn_Triplet*> * aCC3 = &(aNewCC3S->mTri);


    // Add first triplet
    aCC3->push_back(aTri0);  
    aTri0->Flag().set_kth_true(mFlag3CC);//explored
    aTri0->NumCC() = aNumCC;  
    int aKCur = 0;


    // Visit all sommets (not all triplets)
    while (aKCur!=int(aCC3->size()))
	{
		cNOSolIn_Triplet * aTri1 = (*aCC3)[aKCur];

		// For each edge of the current triplet
        for (int aKA=0 ; aKA<3 ; aKA++)
        {


           // Get triplet adjacent to this edge and parse them
           std::vector<cLinkTripl> &  aLnk = aTri1->KArc(aKA)->attr().ASym()->Lnk3();

           for (int aKL=0 ; aKL<int(aLnk.size()) ; aKL++)
           {

              // If not marked, mark it and push it in aCC3, return it was added
              if (SetFlagAdd(*aCC3,aLnk[aKL].m3,mFlag3CC))
              {
                aLnk[aKL].m3->NumCC() = aNumCC;
                mVS[aLnk[aKL].S3()->attr().Im()->Name()] = aLnk[aKL].S3();

                std::cout << aCC3->size() << "=[" <<  aLnk[aKL].S1()->attr().Im()->Name() ;
                std::cout << "," <<  aLnk[aKL].S2()->attr().Im()->Name() ;
                std::cout << "," <<  aLnk[aKL].S3()->attr().Im()->Name() << "=====\n";


                GRAPHVIZ_NodeAdd(g,
                                 aLnk[aKL].S1()->attr().Im()->Name(),
                                 aLnk[aKL].S2()->attr().Im()->Name(),
                                 aLnk[aKL].S3()->attr().Im()->Name());
              }
           }
        }
        aKCur++;

    }
	
	GRAPHIZ_GraphKill(g,gvc,mGraphName);

	// Unflag all triplets do pursue with DFS
	for (int  aK3=0 ; aK3<int (mV3.size()) ; aK3++)
    {
         mV3[aK3]->Flag().set_kth_false(mFlag3CC);
		 mV3[aK3]->NumCC() = -1;
    }

}

#endif



cNOSolIn_Triplet::cNOSolIn_Triplet(cSolGlobInit_NRandom *anAppli,tSomNSI * aS1,tSomNSI * aS2,tSomNSI *aS3,const cXml_Ori3ImInit &aTrip) :
    mAppli (anAppli),
    mNb3   (aTrip.NbTriplet()),
    mNumCC (-1),
	mNumId (-1),
	mNumTT (-1),
	mR2on1 (Xml2El(aTrip.Ori2On1())),
    mR3on1 (Xml2El(aTrip.Ori3On1())),
    mBOnH  (aTrip.BSurH())
{
   mSoms[0] = aS1;
   mSoms[1] = aS2;
   mSoms[2] = aS3;
}


double cNOSolIn_Triplet::CoherTest() const
{
    std::vector<ElRotation3D> aVRLoc;
    std::vector<ElRotation3D> aVRAbs;
    for (int aK=0 ; aK<3 ; aK++)
    {
         aVRLoc.push_back(RotOfK(aK));
         aVRAbs.push_back(mSoms[aK]->attr().TestRot());
    }

    //
    cSolBasculeRig aSolRig = cSolBasculeRig::SolM2ToM1(aVRAbs,aVRLoc);
    double aRes=0;

    for (int aK=0 ; aK<3 ; aK++)
    {
          const ElRotation3D & aRAbs =  aVRAbs[aK];
          const ElRotation3D & aRLoc =  aVRLoc[aK];
          ElRotation3D   aRA2 = aSolRig.TransformOriC2M(aRLoc);

          double aD = DistanceRot(aRAbs,aRA2,mBOnH);
          aRes += aD;
    }
    aRes = aRes / 3.0;


    return aRes;
}

tSomNSI * cLinkTripl::S1() const {return  m3->KSom(mK1);}
tSomNSI * cLinkTripl::S2() const {return  m3->KSom(mK2);}
tSomNSI * cLinkTripl::S3() const {return  m3->KSom(mK3);}


cNOSolIn_AttrSom::cNOSolIn_AttrSom(const std::string & aName,cSolGlobInit_NRandom & anAppli) :
	mName(aName),
	mAppli(&anAppli),
	mIm            (new cNewO_OneIm(mAppli->NM(),mName)),
	mCurRot        (ElRotation3D::Id),
    mTestRot       (ElRotation3D::Id)

{}

void cNOSolIn_AttrSom::AddTriplet(cNOSolIn_Triplet * aTrip,int aK1,int aK2,int aK3)
{
    mLnk3.push_back(cLinkTripl(aTrip,aK1,aK2,aK3));
}


cNOSolIn_AttrArc::cNOSolIn_AttrArc(cNOSolIn_AttrASym * anASym,bool OrASym) :
   mASym   (anASym),
   mOrASym (OrASym)
{
}

static cNO_CmpTriByCost TheCmp3;

cNOSolIn_AttrASym::cNOSolIn_AttrASym() :
	mHeapTri(TheCmp3),
    mNumArc (-1)
{
}

cLinkTripl * cNOSolIn_AttrASym::GetBestTri()
{
	cLinkTripl * aLnk;
    if (mHeapTri.pop(aLnk))
       return aLnk;

    return 0;


}

void  cNOSolIn_AttrASym::AddTriplet(cNOSolIn_Triplet * aTrip,int aK1,int aK2,int aK3)
{
    mLnk3.push_back(cLinkTripl(aTrip,aK1,aK2,aK3));
    mLnk3Ptr.push_back(new cLinkTripl(aTrip,aK1,aK2,aK3));
}


void cNOSolIn_Triplet::SetArc(int aK,tArcNSI * anArc)
{
   mArcs[aK] = anArc;
}


void cSolGlobInit_NRandom::CreateArc(tSomNSI *aS1,tSomNSI *aS2,cNOSolIn_Triplet *aTripl,int aK1,int aK2,int aK3)
{
     tArcNSI * anArc = mGr.arc_s1s2(*aS1,*aS2);
     if (anArc==0)
     {
         cNOSolIn_AttrASym * anAttrSym = new cNOSolIn_AttrASym;
         cNOSolIn_AttrArc anAttr12(anAttrSym,aS1<aS2);
         cNOSolIn_AttrArc anAttr21(anAttrSym,aS1>aS2);
         anArc = &(mGr.add_arc(*aS1,*aS2,anAttr12,anAttr21));
         mNbArc ++;
     }
     anArc->attr().ASym()->AddTriplet(aTripl,aK1,aK2,aK3);
     aTripl->SetArc(aK3,anArc);
}

cLinkTripl * cSolGlobInit_NRandom::GetBestTriDyn()
{
    cLinkTripl * aLnk;
    if (mHeapTriDyn.pop(aLnk))
       return aLnk;

    return 0;


}




cNOSolIn_Triplet* cSolGlobInit_NRandom::GetBestTri()
{
    cNOSolIn_Triplet * aLnk;
    if (mHeapTriAll.pop(aLnk))
       return aLnk;

    return 0;


}

static cNO_CmpTriSolByCost TheCmp3Sol;

cSolGlobInit_NRandom::cSolGlobInit_NRandom(int argc,char ** argv) :
	mNbSom(0),
	mNbTrip(0),
	mFlag3CC(mAllocFlag3.flag_alloc()),
	mFlagS(mGr.alloc_flag_som()),
	mDebug(false),
	mNbSamples(1000),
	mIterCur(0),
	mGraphCoher(false),
	mGraphName(""),
	mHeapTriAll(TheCmp3Sol),
	mHeapTriDyn(TheCmp3)

{
	bool aModeBin = true;

    ElInitArgMain
    (
         argc,argv,
         LArgMain() << EAMC(mFullPat,"Pattern"),
         LArgMain() << EAM(mDebug,"Debug",true,"Print some stuff, Def=false")
                    << EAM(mNbSamples,"Nb",true,"Number of samples, Def=1000")
                    << EAM(mGraphCoher,"GraphCoh",true,"Graph-based incoherence, Def=false")
					<< EAM(aModeBin,"Bin",true,"Binaries file, def = true",eSAM_IsBool)
                    << ArgCMA()
#ifdef GRAPHVIZ_ENABLED
					<< EAM(mGraphName,"Graph",true,"GraphViz filename")
#endif
    );

	
	mEASF.Init(mFullPat);
   	mNM = new cNewO_NameManager(mExtName,mPrefHom,mQuick,mEASF.mDir,mNameOriCalib,"dat");
   	const cInterfChantierNameManipulateur::tSet * aVIm = mEASF.SetIm();


	for (int aKIm=0 ; aKIm <int(aVIm->size()) ; aKIm++)
   	{
       const std::string & aName = (*aVIm)[aKIm];
       tSomNSI & aSom = mGr.new_som(cNOSolIn_AttrSom(aName,*this));
       mMapS[aName] = &aSom;
	   
	   std::cout << mNbSom << "=" << aName << "\n";
       mNbSom++;

	}

	cXml_TopoTriplet aXml3 =  StdGetFromSI(mNM->NameTopoTriplet(true),Xml_TopoTriplet);

    for
    (
         std::list<cXml_OneTriplet>::const_iterator it3=aXml3.Triplets().begin() ;
         it3 !=aXml3.Triplets().end() ;
         it3++
    )
    {
            tSomNSI * aS1 = mMapS[it3->Name1()];
            tSomNSI * aS2 = mMapS[it3->Name2()];
            tSomNSI * aS3 = mMapS[it3->Name3()];

            ELISE_ASSERT(it3->Name1()<it3->Name2(),"Incogeherence cAppli_NewSolGolInit\n");
            ELISE_ASSERT(it3->Name2()<it3->Name3(),"Incogeherence cAppli_NewSolGolInit\n");




            if (aS1 && aS2 && aS3)
            {
				if (mDebug){
					std::cout << "Tri=[" << it3->Name1() << "," << it3->Name2() << "," << it3->Name3() << "\n";
					}

                 mNbTrip++;

                 std::string  aN3 = mNM->NameOriOptimTriplet
                 //std::string  aN3 = mNM->NameOriInitTriplet
                                    (
                                        // mQuick,
                                        aModeBin,  // ModeBin
                                        aS1->attr().Im(),
                                        aS2->attr().Im(),
                                        aS3->attr().Im()
                                    );


                 cXml_Ori3ImInit aXml3Ori = StdGetFromSI(aN3,Xml_Ori3ImInit);


                 cNOSolIn_Triplet * aTriplet = new cNOSolIn_Triplet(this,aS1,aS2,aS3,aXml3Ori);
                 mV3.push_back(aTriplet);

				 if (mDebug)
					std::cout << "% " << aN3 << " " << mV3.size() << " " <<  mNbTrip << "\n";

                 ///  ADD-SOM-TRIPLET
                 aS1->attr().AddTriplet(aTriplet,1,2,0);
                 aS2->attr().AddTriplet(aTriplet,0,2,1);
                 aS3->attr().AddTriplet(aTriplet,0,1,2);

                 ///  ADD-EDGE-TRIPLET
                 CreateArc(aS1,aS2,aTriplet,0,1,2);
                 CreateArc(aS2,aS3,aTriplet,1,2,0);
                 CreateArc(aS3,aS1,aTriplet,2,0,1);

                 //aTriplet->CheckArcsSom();

            }
    }
    std::cout << "LOADED GRAPH " << mChrono.uval() << ", NbTrip=" << mNbTrip << "\n";

#ifdef GRAPHVIZ_ENABLED

	if (mGraphName!="")	
	{
		WriteGraphToFile();
    	std::cout << "SAVED GRAPHHIZ " << mChrono.uval() << "\n";
	}

#endif
}

/* Connected components */

void cSolGlobInit_NRandom::NumeroteCC()
{
    int aNumCC = 0;
    int aNumId = 0;

    for (int  aK3=0 ; aK3<int (mV3.size()) ; aK3++)
    {
        cNOSolIn_Triplet * aTri0 = mV3[aK3];

        // If the triplet has not been marked, it's a new component
        if ( !aTri0->Flag().kth(mFlag3CC))
        {
            // Create a new component
            cNO_CC_TripSom * aNewCC3S = new cNO_CC_TripSom;
            aNewCC3S->mNumCC = aNumCC;  // give it a number
            mVCC.push_back(aNewCC3S);   // add it to the vector of component
            std::vector<cNOSolIn_Triplet*> * aCC3 = &(aNewCC3S->mTri); // Quick acces to vec of tri in the CC
            std::vector<tSomNSI *> * aCCS = &(aNewCC3S->mSoms); // Quick accessto som

            // Calcul des triplets
            aCC3->push_back(aTri0);  // Add triplet T0
            aTri0->Flag().set_kth_true(mFlag3CC);// Mark it as explored
            aTri0->NumCC() = aNumCC;  // Put  right num to T0
            int aKCur = 0;
            // Traditional loop of CC : while  no new inexplored neighboor
            while (aKCur!=int(aCC3->size()))
            {
               cNOSolIn_Triplet * aTri1 = (*aCC3)[aKCur];
               // For each edge of the current triplet
               for (int aKA=0 ; aKA<3 ; aKA++)
               {
                  // Get triplet adjacent to this edge and parse them
                  std::vector<cLinkTripl> &  aLnk = aTri1->KArc(aKA)->attr().ASym()->Lnk3();
                  for (int aKL=0 ; aKL<int(aLnk.size()) ; aKL++)
                  {
                     // If not marked, mark it and push it in aCC3, return it was added
                     if (SetFlagAdd(*aCC3,aLnk[aKL].m3,mFlag3CC))
                     {
                          aLnk[aKL].m3->NumCC() = aNumCC;
                          aLnk[aKL].m3->NumId() = aNumId;
            			  aNumId++;
                     }
                  }
               }
               aKCur++;
            }

            // Compute the summit of the CC, it's easy, just be carrful to get them only once
            int aFlagSom = mGr.alloc_flag_som();
            for (int aKT=0 ; aKT<int(aCC3->size()) ; aKT++)
            {
                cNOSolIn_Triplet * aTri = (*aCC3)[aKT];
                for (int aKS=0 ;  aKS<3 ; aKS++)
                {
                    SetFlagAdd(*aCCS,aTri->KSom(aKS),aFlagSom);
                }
            }

            FreeAllFlag(*aCCS,aFlagSom);
            mGr.free_flag_som(aFlagSom);

			std::cout << "Nb of sommets " << aCCS->size() << " in CC " << aNumCC << "\n"; 

            aNumCC++;

        }
    }

    FreeAllFlag(mV3,mFlag3CC);
	std::cout << "Nb of CCs " << aNumCC << "\n"; 
}
	
/* Randomly choose an adjacent triplet from a set */
cLinkTripl * cSolGlobInit_NRandom::GetRandTri()
{


    cLinkTripl * aTri;

	if (int(mSCur3Adj.size()))
	{
		int aRndTriIdx = NRrandom3(int(mSCur3Adj.size())-1);


		// Get the triplet
		std::set<cLinkTripl *>::iterator it = mSCur3Adj.begin();
		std::advance(it, aRndTriIdx);
		aTri = *it;


		// Remove triplet from the set => mark as explored
		mSCur3Adj.erase(it);	


		// If the sommet was in the meantime added to global solution, 
		// search for another one
		int aCpt=0;
		while ( aTri->S3()->flag_kth(mFlagS) && int(mSCur3Adj.size()))
		{
			aRndTriIdx = NRrandom3(int(mSCur3Adj.size())-1);

			it = mSCur3Adj.begin();
			std::advance(it, aRndTriIdx);
			aTri = *it;

			std::cout << ++aCpt << "====";
			mSCur3Adj.erase(it);	


		}

    	return aTri;
	}
    return 0;
	
}

/* 
 *   Add neighbouring/adjacent triplets to set 
 *
 * */
void cSolGlobInit_NRandom::AddArcOrCur(cNOSolIn_AttrASym *anArc)
{
	// Adjacent triplets
	std::vector<cLinkTripl> &  aLnk = anArc->Lnk3();

	for (int aK=0; aK< int(aLnk.size()); aK++)
	{


		// Test if the sommet S3 exists
		if (! aLnk.at(aK).S3()->flag_kth(mFlagS))
		{
		

			// Add to dynamic structure
			mSCur3Adj.insert(&(aLnk.at(aK)));

/*							std::cout << "*                 " << aLnk.at(aK).S1()->attr().Im()->Name() << " "
									<< aLnk.at(aK).S2()->attr().Im()->Name() << " "
									<< aLnk.at(aK).S3()->attr().Im()->Name() << ",Num="
									<< aLnk.at(aK).S1()->attr().NumCC() << ","
									<< aLnk.at(aK).S2()->attr().NumCC() << ","
									<< aLnk.at(aK).S3()->attr().NumCC() << ",NTT="
									<< aLnk.at(aK).m3->NumTT() << "\n";*/


		}
		// If S3 exists, try to add triplets adjacent to edges: S1-S3 and S2-S3
		else
		{

			for (int aKArc=0; aKArc<3; aKArc++)
			{
				if (anArc!=aLnk.at(aK).m3->KArc(aKArc)->attr().ASym())
				{
					// Secondary triplets adjacent to S1-S3 or S2-S3
					std::vector<cLinkTripl> &  aLnkSec = aLnk.at(aK).m3->KArc(aKArc)->attr().ASym()->Lnk3();

					for (int aT=0; aT< int(aLnkSec.size()); aT++)
					{
						// Test if the "collateral" sommet S3 exists
        				if (! aLnkSec.at(aT).S3()->flag_kth(mFlagS))
        				{

							
							// Add to dynamic structure
							mSCur3Adj.insert(&(aLnkSec.at(aT)));
						
						/*	std::cout << "                  " << aLnkSec.at(aT).S1()->attr().Im()->Name() << " "
                                      << aLnkSec.at(aT).S2()->attr().Im()->Name() << " "
                                      << aLnkSec.at(aT).S3()->attr().Im()->Name() << ",Num=" 
                                      << aLnkSec.at(aT).S1()->attr().NumCC() << "," 
                                      << aLnkSec.at(aT).S2()->attr().NumCC() << "," 
                                      << aLnkSec.at(aT).S3()->attr().NumCC() << ",NTT="
									  << aLnkSec.at(aT).m3->NumTT() << "\n";*/

						}
					}		
				}
			}
		}


	}
}


void cSolGlobInit_NRandom::EstimRt(cLinkTripl * aLnk)
{

    // Get sommets
    tSomNSI * aS1 = aLnk->S1();
    tSomNSI * aS2 = aLnk->S2();
    tSomNSI * aS3 = aLnk->S3();

    // Get current R,t of the mother pair
    ElRotation3D aC1ToM = aS1->attr().CurRot();
    ElRotation3D aC2ToM = aS2->attr().CurRot();

    // Get rij,tij of the triplet sommets
    const ElRotation3D aC1ToL = aLnk->m3->RotOfSom(aS1);
    const ElRotation3D aC2ToL = aLnk->m3->RotOfSom(aS2);
    const ElRotation3D aC3ToL = aLnk->m3->RotOfSom(aS3);

    // Propagate R,t according to:
    // aC1ToM.tr() = T0 + aRL2M * aC1ToL.tr() * Lambda
    //
    // 1-R
    ElMatrix<double> aRL2Mprim = aC1ToM.Mat() * aC1ToL.Mat().transpose();
    ElMatrix<double> aRL2Mbis  = aC2ToM.Mat() * aC2ToL.Mat().transpose();
    ElMatrix<double> aRL2M     = NearestRotation((aRL2Mprim+aRL2Mbis)*0.5);



    // 2-Lambda
    double d12L  = euclid(aC2ToL.tr() - aC1ToL.tr());
    double d12M  = euclid(aC2ToM.tr() - aC1ToM.tr());
    double Lambda = d12M / ElMax(d12L,1e-20);


    // 3-t
    Pt3dr aC1ToLRot = aRL2M * aC1ToL.tr();
    Pt3dr aC2ToLRot = aRL2M * aC2ToL.tr();

    Pt3dr T0prim = aC1ToM.tr() - aC1ToLRot * Lambda;
    Pt3dr T0bis  = aC2ToM.tr() - aC2ToLRot * Lambda;
    Pt3dr T0 = (T0prim+T0bis) /2.0;

    Pt3dr aT3 = T0 + aRL2M * aC3ToL.tr() * Lambda;

    // 4- Set R3,t3
    aS3->attr().CurRot() = ElRotation3D(aT3,aRL2M*aC3ToL.Mat(),true);
    aS3->attr().TestRot() = ElRotation3D(aT3,aRL2M*aC3ToL.Mat(),true);//used in coherence
	

}


void cSolGlobInit_NRandom::FreeSCur3Adj(tSomNSI *aS)
{
	std::vector<cLinkTripl> & aLnk = aS->attr().Lnk3();

	for (int aK=0; aK<int(aLnk.size()); aK++)
	{

		std::set<cLinkTripl*>::iterator it = mSCur3Adj.find(&(aLnk[aK]));
		if (it != mSCur3Adj.end())
		{

			mSCur3Adj.erase(it);

		}
	}

}


void  cSolGlobInit_NRandom::RandomSolOneCC(cNOSolIn_Triplet * aSeed,int NbSomCC)
{
	int aNumCCSom=1;
	
	// Mark the concateantion order of the seed triplet
	aSeed->NumTT() = aNumCCSom;

	for (int aK=0; aK<3; aK++)
	{
		// Set the current R,t of the seed
    	aSeed->KSom(aK)->attr().CurRot() = aSeed->RotOfSom(aSeed->KSom(aK));
	
		// Mark as explored 
		aSeed->KSom(aK)->flag_set_kth_true(mFlagS);


		// Mark the concatenation order of the node;
		// the first three nodes arise from the same triplet therefore the same order 
		aSeed->KSom(aK)->attr().NumCC() = aNumCCSom;


	}
	for (int aK=0; aK<3; aK++)
	{
		// Add the seed to the set of adj triplets
		AddArcOrCur(aSeed->KArc(aK)->attr().ASym());

	}

	int Cpt=0;
	cLinkTripl * aTri=0;
	while ((aTri = GetRandTri()) && ((Cpt+3)<NbSomCC))
	{


		// Flag as visted
		aTri->m3->Flag().set_kth_true(mFlag3CC);

		// Flag triplet order 
		aTri->m3->NumTT() = ElMax(aTri->S1()->attr().NumCC(),aTri->S2()->attr().NumCC()) +1;

		// Flag the concatenation order of the node
		// = order of the "builder" triplet 
		aTri->S3()->attr().NumCC() = aTri->m3->NumTT();

		// Propagate R,t and flag sommet as visited
		EstimRt(aTri);	

		// Mark node as vistied
		aTri->S3()->flag_set_kth_true(mFlagS);

		// Free mSCur3Adj from all triplets connected to S3
		FreeSCur3Adj(aTri->S3());

		// Add two new edges and their respective adjacent triplets
		AddArcOrCur(aTri->m3->KArc(aTri->mK1)->attr().ASym());
		AddArcOrCur(aTri->m3->KArc(aTri->mK2)->attr().ASym());


		Cpt++;

	}
	
	std::cout << "\nIn this CC, nb of connected nodes " << Cpt+3 << "\n";



	
}


void  cSolGlobInit_NRandom::RandomSolOneCC(cNO_CC_TripSom * aCC)
{

	 NRrandom3InitOfTime();
	 std::cout << "DFS per CC, Nb Som " << aCC->mSoms.size() << ", nb triplets " << aCC->mTri.size() << "\n";

	
	 // Select random seed triplet
     int aSeed = NRrandom3(int(aCC->mTri.size()-1));

     cNOSolIn_Triplet * aTri0 = aCC->mTri[aSeed];
     std::cout << "Seed triplet " << aTri0->KSom(0)->attr().Im()->Name() << " "
                                  << aTri0->KSom(1)->attr().Im()->Name() << " "
                                  << aTri0->KSom(2)->attr().Im()->Name() << "\n";


	 // Flag as visited
	 aTri0->Flag().set_kth_true(mFlag3CC);

     ELISE_ASSERT(aTri0!=0,"Cannot compute seed in RandomSolOneCC");

	 // Build the initial solution in this CC
     RandomSolOneCC(aTri0,aCC->mSoms.size());

	 // Calculate coherence scores within this CC
	 if (! mGraphCoher)
     	CoherTriplets(aCC->mTri);
	 else
	 	CoherTripletsGraphBasedV2(aCC->mTri,aCC->mSoms.size());


	 // Print
	 //for (int aK=0; aK<int(aCC->mSoms.size()); aK++)
	 //	std::cout << aCC->mSoms[aK]->flag_kth(mFlagS) << ", " << aCC->mSoms[aK]->attr().Im()->Name() << "\n";

	 // Free flags
	 FreeAllFlag(aCC->mSoms,mFlagS);
	 FreeAllFlag(aCC->mTri,mFlag3CC);

	 // Free the set of current unvisted adjacent triplets
	 mSCur3Adj.clear();

 	 // Reset the concatenation order
	 FreeSomNumCCFlag(); 
}

void cSolGlobInit_NRandom::AddTriOnHeap(cLinkTripl *aLnk)
{
	std::vector<cLinkTripl *> aVMaj;

	std::vector<int> aSIdx {aLnk->mK1,aLnk->mK2,aLnk->mK3};
    for (int aK=0; aK<3; aK++)
    {
		std::vector<cLinkTripl *> &  aVT = aLnk->m3->KArc(aSIdx[aK])->attr().ASym()->Lnk3Ptr();

		for (int aT=0; aT< int(aVT.size()); aT++)
        {

            // If triplet was not visited
            if ( !aVT[aT]->m3->Flag().kth(mFlag3CC))
            {

				// Add to heap
                mHeapTriDyn.push(aVT[aT]);

                // Push to aVMaj
                aVMaj.push_back(aVT[aT]);

                // Flag as marked
                aVT[aT]->m3->Flag().set_kth_true(mFlag3CC);
		
				/*std::cout << "___________ " ;
                std::cout << "added triplet Lnk "
                          << aVT[aT]->S1()->attr().Im()->Name() << " "
                          << aVT[aT]->S2()->attr().Im()->Name() << " "
                          << aVT[aT]->S3()->attr().Im()->Name() << "\n";*/



			}
		}
	}

	for (int aK=0; aK<int(aVMaj.size()); aK++)
    {
        mHeapTriDyn.MAJ(aVMaj[aK]);

    }

}


void cSolGlobInit_NRandom::BestSolOneCC(cNO_CC_TripSom *aCC)
{

	int NbSomCC = int(aCC->mSoms.size());

	// Pick the best triplet
    cNOSolIn_Triplet * aTri0 = GetBestTri();
    std::cout << "Best triplet " << aTri0->KSom(0)->attr().Im()->Name() << " "
                                 << aTri0->KSom(1)->attr().Im()->Name() << " "
                                 << aTri0->KSom(2)->attr().Im()->Name() << " "
                                 << aTri0->CostArc() << "\n";

    // Flag triplet as marked
    aTri0->Flag().set_kth_true(mFlag3CC);

	
	for (int aK=0; aK<3; aK++)
    {
        // Flag sommets as explored
        aTri0->KSom(aK)->flag_set_kth_true(mFlagS);

        // Set the current R,t of the seed
        aTri0->KSom(aK)->attr().CurRot() = aTri0->RotOfSom(aTri0->KSom(aK));

		//PrintRotation(aTri0->KSom(aK)->attr().CurRot().Mat(),ToString(aK));


    }

	// Fill the dynamic heap with triplets connected to this triplet
	cLinkTripl * aLnk0 = new cLinkTripl(aTri0,0,1,2);
    AddTriOnHeap(aLnk0);


	// Iterate to build the solution while updating the heap
    int Cpt=0;
    cLinkTripl * aTriNext=0;
    while ( (aTriNext=GetBestTriDyn()) && ((Cpt+3)<NbSomCC))
    {
		// Check that the node has not been added in the meantime
		if (! aTriNext->S3()->flag_kth(mFlagS))
		{
    		std::cout << "=== Add new node " << Cpt << " "
                  << aTriNext->S1()->attr().Im()->Name() << " "
                  << aTriNext->S2()->attr().Im()->Name() << " "
                  << aTriNext->S3()->attr().Im()->Name() << " "
                  << aTriNext->m3->CostArcMed() << "\n";
            

			/*PrintRotation(aTriNext->S1()->attr().CurRot().Mat(),"0");
			PrintRotation(aTriNext->S2()->attr().CurRot().Mat(),"1");
			PrintRotation(aTriNext->S3()->attr().CurRot().Mat(),"2");*/
            
            
			// Propagate R,t
            EstimRt(aTriNext);
            
			// Mark node as vistied
			aTriNext->S3()->flag_set_kth_true(mFlagS);
            
			// Add to heap
            AddTriOnHeap(aTriNext);
            
            Cpt++;

		}
		else // however, try to adjacent triplets of that triplet
		{

			// Add to heap
            AddTriOnHeap(aTriNext);

		}

    }

    std::cout << "Nb final sommets=" << Cpt+3 << ", out of " << NbSomCC << "\n";


}



void  cSolGlobInit_NRandom::RandomSolAllCC()
{
	std::cout << "Nb of connected components=" << mVCC.size() << "\n";
	for (int aKC=0 ;  aKC<int(mVCC.size()) ; aKC++)
    	RandomSolOneCC(mVCC[aKC]);

}

void cSolGlobInit_NRandom::BestSolAllCC()
{
	// Add all triplets to global heap
	// (used only to get best seed)
	HeapPerSol();


	// Get best solution for each CC
	for (int aKC=0 ;  aKC<int(mVCC.size()) ; aKC++)
	{
        BestSolOneCC(mVCC[aKC]);
    
		// Save
		std::string aOutOri = "DSF_BestInit_CC" + ToString(aKC);
    	Save(aOutOri,true);
	}



	// Free triplets
	FreeAllFlag(mV3,mFlag3CC);
}

void  cSolGlobInit_NRandom::DoOneRandomDFS(bool UseCoherence)
{

	NRrandom3InitOfTime();

	// Clear the sommets
	mVS.clear();
	mVCC.clear();
	

	int aNumCC = 0;

	//select random seed triplet 
	int aSeed = NRrandom3(mNbTrip-1); 

	cNOSolIn_Triplet * aTri0 = mV3[aSeed];
	std::cout << "Seed triplet " << aTri0->KSom(0)->attr().Im()->Name() << " " 
			          			 << aTri0->KSom(1)->attr().Im()->Name() << " " 
					  			 << aTri0->KSom(2)->attr().Im()->Name() << "\n";

	if (UseCoherence)
	{
		aTri0 = GetBestTri();
		std::cout << "Best triplet " << aTri0->KSom(0)->attr().Im()->Name() << " " 
			          	  		     << aTri0->KSom(1)->attr().Im()->Name() << " " 
					  			     << aTri0->KSom(2)->attr().Im()->Name() << " " 
									 << aTri0->CostArc() << "\n";
	}

	mVS[aTri0->KSom(0)->attr().Im()->Name()] = aTri0->KSom(0);
	mVS[aTri0->KSom(1)->attr().Im()->Name()] = aTri0->KSom(1);
	mVS[aTri0->KSom(2)->attr().Im()->Name()] = aTri0->KSom(2);

	// Set the current R,t of the seed
	aTri0->KSom(0)->attr().CurRot() = aTri0->RotOfSom(aTri0->KSom(0));
	aTri0->KSom(1)->attr().CurRot() = aTri0->RotOfSom(aTri0->KSom(1));
	aTri0->KSom(2)->attr().CurRot() = aTri0->RotOfSom(aTri0->KSom(2));

	getchar();

#ifdef GRAPHVIZ_ENABLED


	// Read the graph from a file (contains all triplets possible)
	const std::string aGCurName = "Gr_"+ (UseCoherence ? "BestInit" : ToString(mIterCur));

	graph_t *g = GRAPHIZ_GraphRead(mGraphName);
	GVC_t *gvc = GRAPHVIZ_GVCInit(aGCurName);


	// Mark the seed triplet in red
	GRAPHVIZ_NodeChgColor(g,aTri0->KSom(0)->attr().Im()->Name());
	GRAPHVIZ_NodeChgColor(g,aTri0->KSom(1)->attr().Im()->Name());
	GRAPHVIZ_NodeChgColor(g,aTri0->KSom(2)->attr().Im()->Name());

	GRAPHVIZ_EdgeChgColor(g,
                          aTri0->KSom(0)->attr().Im()->Name(),
                          aTri0->KSom(1)->attr().Im()->Name(),
						  "blue");
	GRAPHVIZ_EdgeChgColor(g,
                          aTri0->KSom(0)->attr().Im()->Name(),
                          aTri0->KSom(2)->attr().Im()->Name(),
						  "blue");
	GRAPHVIZ_EdgeChgColor(g,
                          aTri0->KSom(1)->attr().Im()->Name(),
                          aTri0->KSom(2)->attr().Im()->Name(),
						  "blue");
#endif

	
	// Create a new component
    cNO_CC_TripSom * aNewCC3S = new cNO_CC_TripSom;
    aNewCC3S->mNumCC = aNumCC; 
    mVCC.push_back(aNewCC3S);  
    std::vector<cNOSolIn_Triplet*> * aCC3 = &(aNewCC3S->mTri); // Quick acces to vec of tri in the CC
    // std::vector<tSomNSI *> * aCCS = &(aNewCC3S->mSoms); // Quick access to som


	

    // Calcul des triplets 
    aCC3->push_back(aTri0);  // Add triplet T0

    //aTri0->Flag().set_kth_true(mFlag3CC);// Mark it as explored
    //aTri0->NumCC() = aNumCC;  // Put  right num to T0

    

	// Visit all sommets (not all triplets)  
	while (int(mVS.size())!=mNbSom)
	{
		if (mDebug)
			std::cout << int(mVS.size()) << "/" << mNbSom << " " ;

		// Randomly select a triplet (out of visited ones)
		int t_r = NRrandom3(int(aCC3->size()));
		if (mDebug)
			std::cout << ", trip_r=" << t_r << "/" << int(aCC3->size()) << " ";


		cNOSolIn_Triplet * aTri1 = (*aCC3)[t_r];


		// Randomly select an edge e_r
		int e_r = NRrandom3(3);
		
		if (mDebug)
			std::cout << ", edge_r=" << e_r << "/3 " ;

		//Triplets adjacent to edge e_r
		std::vector<cLinkTripl> &  aLnk = aTri1->KArc(e_r)->attr().ASym()->Lnk3();


		// Pick an edge adjacent to e_r (best or random)
	    int e_a = NRrandom3(int(aLnk.size()));
	
		cLinkTripl TriAdjCur = aLnk[e_a];
		if (UseCoherence)
		{
			cLinkTripl * TriAdjCurPtr;
			// If no best exists, it will take a random edge
			if ((TriAdjCurPtr = aTri1->KArc(e_r)->attr().ASym()->GetBestTri()))
				TriAdjCur = *TriAdjCurPtr;
		}	
		/*cLinkTripl TriAdjCur = (UseCoherence ? *(aTri1->KArc(e_r)->attr().ASym()->GetBestTri()) 
						                     : aLnk[e_a]);*/

		if (mDebug)
				std::cout << ", edge_a=" << e_a << "/" << int(aLnk.size()) << "\n";




		// If not marked, mark it and push it in aCC3, return it was added
		if (! DicBoolFind(mVS,TriAdjCur.S3()->attr().Im()->Name()))
		{
			//TriAdjCur.m3->NumCC() = aNumCC;
			mVS[TriAdjCur.S3()->attr().Im()->Name()] = TriAdjCur.S3();

			//Update aCC3
			aCC3->push_back(TriAdjCur.m3);

			// Get sommets
			tSomNSI * aS1 = TriAdjCur.S1();
			tSomNSI * aS2 = TriAdjCur.S2();
			tSomNSI * aS3 = TriAdjCur.S3();

			// Get current R,t of the mother pair
			ElRotation3D aC1ToM = aS1->attr().CurRot();
			ElRotation3D aC2ToM = aS2->attr().CurRot();
			
			// Get rij,tij of the triplet sommets
			const ElRotation3D aC1ToL = TriAdjCur.m3->RotOfSom(aS1);
			const ElRotation3D aC2ToL = TriAdjCur.m3->RotOfSom(aS2);
			const ElRotation3D aC3ToL = TriAdjCur.m3->RotOfSom(aS3);

			// Propagate R,t according to:
			// aC1ToM.tr() = T0 + aRL2M * aC1ToL.tr() * Lambda
			//
			// 1-R
			ElMatrix<double> aRL2Mprim = aC1ToM.Mat() * aC1ToL.Mat().transpose(); 
			ElMatrix<double> aRL2Mbis  = aC2ToM.Mat() * aC2ToL.Mat().transpose();
		    ElMatrix<double> aRL2M     = NearestRotation((aRL2Mprim+aRL2Mbis)*0.5);



			// 2-Lambda
			double d12L  = euclid(aC2ToL.tr() - aC1ToL.tr());
			double d12M  = euclid(aC2ToM.tr() - aC1ToM.tr());
			double Lambda = d12M / ElMax(d12L,1e-20);


			// 3-t
			Pt3dr aC1ToLRot = aRL2M * aC1ToL.tr();
			Pt3dr aC2ToLRot = aRL2M * aC2ToL.tr();

			Pt3dr T0prim = aC1ToM.tr() - aC1ToLRot * Lambda; 
			Pt3dr T0bis  = aC2ToM.tr() - aC2ToLRot * Lambda; 
			Pt3dr T0 = (T0prim+T0bis) /2.0;

			Pt3dr aT3 = T0 + aRL2M * aC3ToL.tr() * Lambda;

			// 4- Set R3,t3
			aS3->attr().CurRot() = ElRotation3D(aT3,aRL2M*aC3ToL.Mat(),true);
			aS3->attr().TestRot() = ElRotation3D(aT3,aRL2M*aC3ToL.Mat(),true);//used in coherence



			std::cout << "=== " << TriAdjCur.S1()->attr().Im()->Name() << " "
					            << TriAdjCur.S2()->attr().Im()->Name() << " "
								<< TriAdjCur.S3()->attr().Im()->Name() << ", Cost="
								<< TriAdjCur.m3->CostArc() << "\n";

#ifdef GRAPHVIZ_ENABLED
			/* Only edge between 2-3 is drawn */
			GRAPHVIZ_EdgeChgColor(g,
                                 TriAdjCur.S2()->attr().Im()->Name(),
                                 TriAdjCur.S3()->attr().Im()->Name());
#endif

		}

	
	}


	// Calculate coherence scores
	CoherTriplets();

	// Save
	std::string aOutOri = "DSF_" + (UseCoherence ? "BestInit" : ToString(mIterCur));
	Save(aOutOri);

	std::cout << "NbTriii " << aCC3->size() << " NbSooom " << mVS.size() << "\n";

#ifdef GRAPHVIZ_ENABLED 
	GRAPHIZ_GraphKill(g,gvc);
#endif
}

/*
 * Incoherence score weighted by the distance in the graph
 *   Note1: a triplet may or may not have contributed to the solution
 *   Note2: a "solution" triplet contributes by adding a new sommet to the solution
 *   Note3: the distance of a "solution" triplet is equivalent of the order of the new sommet 
 *   Note4: the distance of all other triplets is equivalent of a mean distance of all three sommets
 *    
 * */
void cSolGlobInit_NRandom::CoherTripletsGraphBasedV2(std::vector<cNOSolIn_Triplet *>& aV3,int NbSom) 
{
	double a1 = (MAX_WEIGHT - MIN_WEIGHT)/ElMax(NbSom-1,1);
    double a2 = (MIN_WEIGHT*NbSom - MAX_WEIGHT)/ElMax(NbSom-1,1);

	for (int aT=0; aT<int(aV3.size()); aT++)
	{
		/*std::cout << "[" 
                  << aV3[aT]->KSom(0)->attr().NumCC() << "," 
                  << aV3[aT]->KSom(1)->attr().NumCC() << ","
                  << aV3[aT]->KSom(2)->attr().NumCC() << "] NumTT="
                  << aV3[aT]->NumTT() << " "  
                  << aV3[aT]->KSom(0)->attr().Im()->Name() << " " 
                  << aV3[aT]->KSom(1)->attr().Im()->Name() << " "
                  << aV3[aT]->KSom(2)->attr().Im()->Name() << " ";*/

		double aDist = 0;
		// Not "solution" triplet
		if (aV3[aT]->NumTT() == -1)
		{
			for (int aS=0; aS<3; aS++)
				aDist += aV3[aT]->KSom(aS)->attr().NumCC(); 
			aDist /= 3;

		}
		// "Solution" triplet
		else
			aDist = aV3[aT]->NumTT();

		double aCostCur = ElMin(abs(aV3[aT]->CoherTest()),1e9);

        //std::cout << ",Dist=" << aDist << ",CostN=" <<  aCostCur << ",CPds=" << aCostCur/aDist << "\n";  
		
		//aV3[aT]->CostArcPerSample().push_back(aCostCur/sqrt(aDist));
		aV3[aT]->CostArcPerSample().push_back(aCostCur/(a1*aDist+a2));
		


	}


}

/* Old graph-based weigthing */
void cSolGlobInit_NRandom::CoherTripletsGraphBased(std::vector<cNOSolIn_Triplet *>& aV3)
{
	int Nb = int(aV3.size());
		
	double a1 = (MAX_WEIGHT - MIN_WEIGHT)/ElMax(Nb-1,1);
	double a2 = (MIN_WEIGHT*Nb - MAX_WEIGHT)/ElMax(Nb-1,1);

	for (int aT=0; aT<int(aV3.size()); aT++)
    {
		// Get the base edge being the two smaller NumCC
		// EB1 = min(a,b)    EB2 = min(c,max(a,b))
		int aEB1 = ElMin(aV3[aT]->KSom(0)->attr().NumCC(),
						 aV3[aT]->KSom(1)->attr().NumCC());
		int aEB2 = ElMin(aV3[aT]->KSom(2)->attr().NumCC(),
				   ElMax(aV3[aT]->KSom(0)->attr().NumCC(),
                         aV3[aT]->KSom(1)->attr().NumCC()));

		int aDist = ElMax(aEB1,aEB2);	


		double aCostCur = ElMin(abs(aV3[aT]->CoherTest()),1e9);

		aV3[aT]->CostArcPerSample().push_back(aCostCur/(a1*aDist+a2));

	}

}

void cSolGlobInit_NRandom::CoherTriplets(std::vector<cNOSolIn_Triplet *>& aV3)
{

    for (int aT=0; aT<int(aV3.size()); aT++)
    {
        double aCostCur = ElMin(abs(aV3[aT]->CoherTest()),1e9);
        aV3[aT]->CostArcPerSample().push_back(aCostCur);

        //std::cout << "cost=" << aCostCur << "\n"; 
    }

}



void cSolGlobInit_NRandom::CoherTriplets()
{
	//std::cout << "size CostArcPerSample=" <<  int(mV3[0]->CostArcPerSample().size()) << "\n";

	for (int aT=0; aT<int(mV3.size()); aT++)
	{
		double aCostCur = ElMin(abs(mV3[aT]->CoherTest()),1e9);
		mV3[aT]->CostArcPerSample().push_back(aCostCur);
	
		//std::cout << "cost=" << aCostCur << "\n";	
	}

}

void cSolGlobInit_NRandom::CoherTripletsAllSamples()
{
	for (int aT=0; aT<int(mV3.size()); aT++)
    {
		
		mV3[aT]->CostArc() = KthValProp(mV3[aT]->CostArcPerSample(),0.8);
		mV3[aT]->CostArcMed() = MedianeSup(mV3[aT]->CostArcPerSample());
		
		/*std::cout << "Ec80=" << mV3[aT]->CostArc() << 
				     ", MED=" << mV3[aT]->CostArcMed() << " " << 
					 mV3[aT]->KSom(0)->attr().Im()->Name() << " " << 
					 mV3[aT]->KSom(1)->attr().Im()->Name() << " " <<
					 mV3[aT]->KSom(2)->attr().Im()->Name() << "\n";*/
	}	
}

/* This heap will serve to GetBestTri when building the ultimate init solution */
void cSolGlobInit_NRandom::HeapPerEdge()
{

	// For all triplets
	for (auto aTri : mV3)
	{
		
		// For each edge of the current triplet
		for (int aK=0; aK<3; aK++)
		{
			std::vector<cLinkTripl*> &  aLnk = aTri->KArc(aK)->attr().ASym()->Lnk3Ptr();

			// For all adjacent triplets to the current edge
			for (auto aTriAdj : aLnk)
			{
				// Push to heap
				aTri->KArc(aK)->attr().ASym()->mHeapTri.push(aTriAdj);
			}

			// Order index
			for (auto aTriAdj : aLnk)
    		{
        		aTri->KArc(aK)->attr().ASym()->mHeapTri.MAJ(aTriAdj);
    		}
		}
		
	}

	
}


void cSolGlobInit_NRandom::HeapPerSol()
{
	// Update heap
    for (auto aTri : mV3)
    {
		mHeapTriAll.push(aTri);	
	}

	// Order index
	for (auto aTri : mV3)
    {
        mHeapTriAll.MAJ(aTri);
    }
}

void cSolGlobInit_NRandom::FreeSomNumCCFlag()
{
	for (tItSNSI anItS=mGr.begin(mSubAll) ; anItS.go_on(); anItS++)
    {
		(*anItS).attr().NumCC() = 0;
	}
}

void cSolGlobInit_NRandom::Save(std::string& OriOut,bool SaveListOfName)
{

	std::list<std::string> aListOfName;

    for (tItSNSI anItS=mGr.begin(mSubAll) ; anItS.go_on(); anItS++)
    {
		if ((*anItS).flag_kth(mFlagS))
		{
            cNewO_OneIm * anI = (*anItS).attr().Im();
            std::string aNameIm = anI->Name();
            CamStenope * aCS = anI->CS();
            ElRotation3D aROld2Cam = (*anItS).attr().CurRot().inv();
         
            aCS->SetOrientation(aROld2Cam);
         
            cOrientationConique anOC =  aCS->StdExportCalibGlob();
            anOC.Interne().SetNoInit();
         
	 	    std::string aFileIterne = mNM->ICNM()->StdNameCalib(mNameOriCalib,aNameIm);
            
	 	    std::string aNameOri = mNM->ICNM()->Assoc1To1("NKS-Assoc-Im2Orient@-"+OriOut,aNameIm,true);
            anOC.FileInterne().SetVal (NameWithoutDir(aFileIterne));
         
	 	    // Copy interior orientation
	 	    std::string aCom = "cp " + aFileIterne + " " + DirOfFile(aNameOri); 
	 	    System(aCom);
         
			aListOfName.push_back(aNameIm);

            MakeFileXML(anOC,aNameOri);
		}
    }

	if (SaveListOfName)
	{
		cListOfName aLOF;
		aLOF.Name() = aListOfName;
		MakeFileXML(aLOF,"ListPattern_" + OriOut + ".xml");
	}
}


void cSolGlobInit_NRandom::ShowTripletCost()
{
	for (auto aTri : mV3)
	{
		std::cout << "[" << aTri->KSom(0)->attr().Im()->Name() << ","  
				         << aTri->KSom(1)->attr().Im()->Name() << ","  
						 << aTri->KSom(2)->attr().Im()->Name() << "], "
						 << " Cost=" << aTri->CostArc() << "   ,MED=" << aTri->CostArcMed() << "\n";
	}
}

void  cSolGlobInit_NRandom::DoRandomDFS()
{
	for (mIterCur=0; mIterCur<mNbSamples; mIterCur++)
    {
        std::cout << "Iter=" << mIterCur << "\n";
		DoOneRandomDFS(false);
    }

	// Calculate median and 80% quantile coherence scores of mNbSamples
	CoherTripletsAllSamples();

	// Create a heap for each edge of the hypergraph and 
	// update with adjacent tripletis' coherence scores
	HeapPerEdge();

	// Create a unique heap for all triplets
	HeapPerSol();

	// Build a unique solution
	DoOneRandomDFS(true);

	// Print the cost for all triplets
	ShowTripletCost();
}

void cSolGlobInit_NRandom::DoNRandomSol()
{
	// Create connected components
	NumeroteCC();

	// Build random inital solutions
	for (mIterCur=0; mIterCur<mNbSamples; mIterCur++)
    {
		std::cout << "Iter=" << mIterCur << "\n";
		RandomSolAllCC();
	}


	// Calculate median and 80% quantile coherence scores of mNbSamples
    CoherTripletsAllSamples();

	// Print the cost for all triplets
    ShowTripletCost();

	// Build "most coherent" solution
	BestSolAllCC();
}

int CPP_SolGlobInit_RandomDFS_main(int argc,char ** argv)
{

	cSolGlobInit_NRandom aSGI(argc,argv);

	// New pipeline
	aSGI.DoNRandomSol();

	// Old pipeline
	//aSGI.DoRandomDFS();


    return EXIT_SUCCESS;
}


cAppliGenOptTriplets::cAppliGenOptTriplets(int argc,char ** argv) :
	mSigma(0),
	mRatioOutlier(0)
{

    NRrandom3InitOfTime();

	std::string aDir;
	bool aModeBin = true;

    ElInitArgMain
    (
         argc,argv,
         LArgMain() << EAMC(mFullPat,"Pattern of images")
		            << EAMC(InOri,"InOri that wil serve to build perfect triplets"),
         LArgMain() << EAM(mSigma,"Sigma",true,"Sigma of the added noise, Def=0 no noise added")
		            << EAM(mRatioOutlier,"Ratio","Good to bad triplet ratio (outliers), Def=0")
                    << EAM(aModeBin,"Bin",true,"Binaries file, def = true",eSAM_IsBool)
                    << ArgCMA()
    );

	mEASF.Init(mFullPat);
    mNM = new cNewO_NameManager(mExtName,mPrefHom,mQuick,mEASF.mDir,mNameOriCalib,"dat");
    //const cInterfChantierNameManipulateur::tSet * aVIm = mEASF.SetIm();

	StdCorrecNameOrient(mNameOriCalib,aDir);
	StdCorrecNameOrient(InOri,aDir);

    cXml_TopoTriplet aXml3 =  StdGetFromSI(mNM->NameTopoTriplet(true),Xml_TopoTriplet);
	mNbTri = int(aXml3.Triplets().size());

    
	// Identify a set of random triplets that will become outliers
    std::set<int> aOutlierList;
    if (mRatioOutlier>0)
    {
        int aNbOutliers = std::floor(mRatioOutlier * mNbTri);




        for (int aK=0; aK<aNbOutliers; aK++)
        {
            aOutlierList.insert(NRrandom3(mNbTri-1));
        }


    }


	// Save the triplets and perturb with outliers if asked 
	int aK=0;
    for
    (
         std::list<cXml_OneTriplet>::const_iterator it3=aXml3.Triplets().begin() ;
         it3 !=aXml3.Triplets().end() ;
         it3++
    )
    {

		bool Ok;
        std::pair<ElRotation3D,ElRotation3D> aPair = mNM->OriRelTripletFromExisting(
                                                        InOri,
                                                        it3->Name1(),
                                                        it3->Name2(),
                                                        it3->Name3(),
                                                        Ok);

        std::string aNameSauveXml = mNM->NameOriOptimTriplet(false,it3->Name1(),it3->Name2(),it3->Name3(),false);
        std::string aNameSauveBin = mNM->NameOriOptimTriplet(true ,it3->Name1(),it3->Name2(),it3->Name3(),false);

        cXml_Ori3ImInit aXml;

		if (DicBoolFind(aOutlierList,aK))
		{
			aXml.Ori2On1() = El2Xml(ElRotation3D(aPair.first.tr(),RandPeturbR(),true));
            aXml.Ori3On1() = El2Xml(ElRotation3D(aPair.second.tr(),RandPeturbR(),true));
			std::cout << "Perturbed R=["<< it3->Name1() <<","<< it3->Name2() << "," << it3->Name3() 
					                    << "], " << aPair.first.tr() << " " << aPair.second.tr() << "\n";	
		}
		else
		{
			aXml.Ori2On1() = El2Xml(ElRotation3D(aPair.first.tr(),aPair.first.Mat(),true));
            aXml.Ori3On1() = El2Xml(ElRotation3D(aPair.second.tr(),aPair.second.Mat(),true));
		
		}


        MakeFileXML(aXml,aNameSauveXml);
        MakeFileXML(aXml,aNameSauveBin);


		aK++;
	}
}


ElMatrix<double> cAppliGenOptTriplets::RandPeturbR()
{
	
	double aW[] = {NRrandom3(),NRrandom3(),NRrandom3()};

	Pt3dr aI(exp(0),exp(aW[2]),exp(-aW[1]));
	Pt3dr aJ(exp(-aW[2]),exp(0),exp(aW[0]));
	Pt3dr aK(exp(aW[1]),exp(-aW[0]),exp(0));	
	
   	ElMatrix<double> aRes = MatFromCol(aI,aJ,aK);


	return aRes;	
}


int CPP_GenOptTriplets(int argc,char ** argv)
{

	cAppliGenOptTriplets aAppGenTri(argc,argv);

	return EXIT_SUCCESS;
}



/*Footer-MicMac-eLiSe-25/06/2007

Ce logiciel est un programme informatique servant à la mise en
correspondances d'images pour la reconstruction du relief.

Ce logiciel est régi par la licence CeCILL-B soumise au droit français et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, modifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL-B telle que diffusée par le CEA, le CNRS et l'INRIA
sur le site "http://www.cecill.info".

En contrepartie de l'accessibilité au code source et des droits de copie,
de modification et de redistribution accordés par cette licence, il n'est
offert aux utilisateurs qu'une garantie limitée.  Pour les mêmes raisons,
seule une responsabilité restreinte pèse sur l'auteur du programme,  le
titulaire des droits patrimoniaux et les concédants successifs.

A cet égard  l'attention de l'utilisateur est attirée sur les risques
associés au chargement,  à l'utilisation,  à la modification et/ou au
développement et à la reproduction du logiciel par l'utilisateur étant
donné sa spécificité de logiciel libre, qui peut le rendre complexe à
manipuler et qui le réserve donc à des développeurs et des professionnels
avertis possédant  des  connaissances  informatiques approfondies.  Les
utilisateurs sont donc invités à charger  et  tester  l'adéquation  du
logiciel à leurs besoins dans des conditions permettant d'assurer la
sécurité de leurs systèmes et ou de leurs données et, plus généralement,
à l'utiliser et l'exploiter dans les mêmes conditions de sécurité.

Le fait que vous puissiez accéder à cet en-tête signifie que vous avez
pris connaissance de la licence CeCILL-B, et que vous en avez accepté les
termes.
Footer-MicMac-eLiSe-25/06/2007*/

