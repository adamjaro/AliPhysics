//Pil
//Creation of TFile with track cut configuration
//#include "AliESDtrackCuts.h"
//#include "AliAnalysisCuts.h"
//#include "AliFlowTrackSimple.h"      // added as hint for hidden library dependency to libPWGflowBase
//#include "AliFlowCandidateTrack.h"   // added as hint for hidden library dependency to libPWGflowTasks
//#include "AliCFContainer.h"          // added as hint for hidden library dependency to libCORRFW
//#include "AliAODRecoDecayHF2Prong.h" // added as hint for hidden library dependency to libPWGHFvertexingHF
#include "AliRDHFCutsD0toKpi.h"
#include <Riostream.h>
#include <TFile.h>
#include <AliRDHFCutsD0toKpi.h>
#include <TClonesArray.h>
#include <TParameter.h>
#include "AliHFAssociatedTrackCuts.h"
#include "AliAnalysisTaskDxHFEParticleSelection.h"
#include "AliDxHFEParticleSelection.h"
#include "AliAnalysisManager.h"
#include "AliAnalysisDataContainer.h"
#include "AliHFEcuts.h"
#include "AliLog.h"
#include "TObject.h"
#include "TClass.h"
#include "TDirectory.h"
#include "TROOT.h"
#include "AliHFEextraCuts.h"
#include "TCanvas.h"
#include "TObject.h"
#include "AliHFEpid.h"
using namespace std;


//const char* includePath="-I. -I$ROOTSYS/include -I$ALICE_ROOT/include";
/*const char* libraryDependencies=
  "libSTEERBase.so "
  "libESD.so "
  "libAOD.so "
  "libANALYSIS.so "
  "libANALYSISalice.so "
  ;
*/
const char* poolInfoName="PoolInfo";
AliAnalysisCuts* createDefaultPoolConfig();
AliAnalysisCuts* createPbPbPoolConfig();
AliAnalysisCuts* createpPbPoolConfig();

void makeTFileCuts(TString arguments="")
{
  //  gSystem->AddIncludePath(includePath);
  //  TString libraries=libraryDependencies;
  //  Bool_t bUseMC=kFALSE;
  TString ofilename;
  Int_t system=0;
  TString taskOptions;
  Bool_t bUseMC=kFALSE;
  Bool_t bUseKine=kFALSE;
  Bool_t bUseMCReco=kFALSE;
  Int_t NrTPCclusters=120; // quick fix for problems sending track cut objects in some instances to task
  Int_t NrITSclusters=4; // quick fix for problem sending hfe track cut object to addtask
  Int_t ITSreq=AliHFEextraCuts::kFirst;
  Int_t Particle=AliAnalysisTaskDxHFEParticleSelection::kD0;
  TString extraname="";


  //----------------------------------------------------//
  //                                                    //
  //           Scanning arguments from string           //
  //             and changing configuration             //
  //                   based on them.                   //
  //                                                    //
  //----------------------------------------------------//
  
  cout << endl << "===============================================" << endl;
  cout << "Setting up Particle Selection task: " << arguments << endl;
  
  // look for configuration arguments if nothing specified
  // in the function call
  if (arguments.IsNull() && gDirectory) {                           //--------------------//
    const char* confObjectName="run_single_task_configuration";     //                    // 
    TObject* confObject=gDirectory->FindObject(confObjectName);     //   Remove this?     //
    if (confObject) {                                               //                    //
      arguments=confObject->GetTitle();                             //--------------------//
    }
  }
  {// deprecated, but keep for formatting
    {// deprecated, but keep for formatting
      TObjArray* tokens=arguments.Tokenize(" ");
      if (tokens) {
	TIter next(tokens);
	TObject* token;
	while ((token=next())) {
	  TString argument=token->GetName();
	  // if (argument.BeginsWith("file=")) {
	  //   argument.ReplaceAll("file=", "");
	  //   ofilename=argument;
	  // } else if (argument.BeginsWith("name=")) {
	  //   argument.ReplaceAll("name=", "");
	  //   analysisName=" "+argument+"PartSel";
	  // }	  
	  // if (argument.BeginsWith("cutname=")) {
	  //   argument.ReplaceAll("cutname=", "");
	  //   poolConfigFile=argument;
	  // }
	  // if (argument.BeginsWith("cutFilename=")) {
	  //   argument.ReplaceAll("cutFilename=", "");
	  //   cutFilename=argument;
	  // }
	  if (argument.BeginsWith("mc")) {
	    bUseMC=kTRUE;
	    taskOptions+=" mc";
	  }
	  if (argument.BeginsWith("PbPb") || argument.BeginsWith("Pb-Pb")) {
	    system=1;
	    taskOptions+=" system=Pb-Pb";
	    cout << "Use PbPb" << endl;
	  }
	  if (argument.BeginsWith("system=p-Pb") ||
	      argument.BeginsWith("pPb") ||
	      argument.BeginsWith("p-Pb") ||
	      argument.BeginsWith("system=2")) {
	    system=2;
	    taskOptions+=" system=p-Pb";
	    cout<<"Use pPb"<<endl;
	  }
	  
	  if(argument.BeginsWith("tpcclusters=")){
	    argument.ReplaceAll("tpcclusters=", "");
	    NrTPCclusters=argument.Atoi();
	    //	    ::Info("AddTaskDxHFEParticleSelection",Form("Setting nr TPC clusters to %d",NrTPCclusters));
	  }
	  if (argument.BeginsWith("fillD0scheme=")){
	    argument.ReplaceAll("fillD0scheme=","");
	    taskOptions+=" fillD0scheme="+argument;
	  }
	  if(argument.BeginsWith("elmcreco")){
	    bUseMCReco=kTRUE;
	    taskOptions+=" "+argument;
	  }
	  if (argument.BeginsWith("usekine") ||argument.BeginsWith("kine")) {
	    bUseKine=kTRUE;
	    taskOptions+=" usekine";
	  }
	  if (argument.BeginsWith("particle=")) {
	    taskOptions+=" "+argument;
	    argument.ReplaceAll("particle=","");
	    if (argument.CompareTo("D0")==0){ 
	      Particle=AliAnalysisTaskDxHFEParticleSelection::kD0; 
	    }
	    else if (argument.CompareTo("electron")==0){ 
	      Particle=AliAnalysisTaskDxHFEParticleSelection::kElectron; 
	    }	    
	  }
	  if(argument.BeginsWith("useinvmasscut"))
	    taskOptions+=" "+argument;
	  if(argument.BeginsWith("twoselectedinvmasscut"))
	    taskOptions+=" "+argument;
	  if(argument.BeginsWith("invmasscut="))
	    taskOptions+=" "+argument;
	  if(argument.BeginsWith("impactparamcut"))
	    taskOptions+=" "+argument;
	  if(argument.BeginsWith("etacut"))
	    taskOptions+=" "+argument;
	  if(argument.BeginsWith("storelastcutstep"))
	    taskOptions+=" "+argument;
	  if(argument.BeginsWith("itsclusters=")){
	    argument.ReplaceAll("itsclusters=", "");
	    NrITSclusters=argument.Atoi();
	  }
	  if(argument.BeginsWith("itsreq=")){
	    argument.ReplaceAll("itsreq=", "");
	    if(argument.CompareTo("kFirst")==0) ITSreq=AliHFEextraCuts::kFirst;
	    else if(argument.CompareTo("kAny")==0) ITSreq=AliHFEextraCuts::kAny;
	    else if(argument.CompareTo("kNone")==0) ITSreq=AliHFEextraCuts::kNone;
	  }
	  if(argument.BeginsWith("extraname=")){
	    argument.ReplaceAll("extraname=", "");
	    extraname=argument;
	  }
	}
	
      }
      delete tokens;
    }
  }
  
  if(bUseMCReco && bUseKine) {
    ::Fatal("AddTaskDxHFECorrelation","CAN'T SET BOTH usekine AND elmcreco AT THE SAME TIME");
    return;
  }

  ///////////////////////////////////////////////////////////////////////  
  //---------------Config from arugments complete----------------------//
  ///////////////////////////////////////////////////////////////////////

  AliRDHFCutsD0toKpi* RDHFD0toKpi=new AliRDHFCutsD0toKpi();
  if (system==0) {
    RDHFD0toKpi->SetStandardCutsPP2010();
  } 
  else if (system==1) {
    RDHFD0toKpi->SetStandardCutsPbPb2011();
    //[FIXME] seems to work now
    // For centrality 0-10%, add centrality flattening
    //NB! NEED FOR THE MOMENT THE FILE!
    TFile *fFlat=TFile::Open("CentrDistrBins005.root","READ");
    TCanvas *c;
    c=(TCanvas*)fFlat->Get("cintegral");
    TH1F *hfl=(TH1F*)c->FindObject("hint");
    RDHFD0toKpi->SetHistoForCentralityFlattening(hfl,0.,10.,0.,0);
    //  RDHFD0toKpi->SetUseCentrality(AliRDHFCuts::kCentV0M);
    
     RDHFD0toKpi->SetMinCentrality(0.);// 40.*1.01
     RDHFD0toKpi->SetMaxCentrality(10.);// 80.*1.01
  }
  else if (system==2) {
    RDHFD0toKpi->SetStandardCutsPP2010();
    RDHFD0toKpi->SetTriggerMask(AliVEvent::kINT7); //pPb
    RDHFD0toKpi->SetTriggerClass(""); //pPb  
  }
  else {
    //warning, no system set
  }
  printf("Saving D0toKpiCuts-object:\n");
  RDHFD0toKpi->PrintAll();
  
   ///______________________________________________________________________
  /// Cuts for HFE
  AliHFEcuts *hfecuts = new AliHFEcuts("hfeCutsTPCTOF","HFE Standard Cuts");
  hfecuts->CreateStandardCuts();

  hfecuts->SetTPCmodes(AliHFEextraCuts::kFound,AliHFEextraCuts::kFoundOverFindable);
  hfecuts->SetMinNClustersTPC(NrTPCclusters);	//Default = 80
  hfecuts->SetMinNClustersTPCPID(80);	//Default = 80
  hfecuts->SetMinRatioTPCclusters(0.6); 	//Default = 0.6
	
  ///ITS
  hfecuts->SetCutITSpixel(ITSreq); 	//Cut on SPD
  //hfecuts->SetCutITSdrift(AliHFEextraCuts::kAny); 	//Cut on SDD
  //hfecuts->SetCheckITSLayerStatus(kFALSE);
  hfecuts->SetMinNClustersITS(NrITSclusters);		//Default = 4
    
  ///TOF
  hfecuts->SetTOFPIDStep(kTRUE);
		
  ///Additional Cuts
  hfecuts->SetPtRange(0.30, 10.5);
  hfecuts->SetMaxImpactParam(1.,2.);
  hfecuts->SetVertexRange(10.);

  // ________________________________________________________________________
  // PID for HFE
  // PID for Only TOF
  AliHFEpid *fPIDOnlyTOF = new AliHFEpid("hfePidTOF");
  if(!fPIDOnlyTOF->GetNumberOfPIDdetectors()) { 
    fPIDOnlyTOF->AddDetector("TOF",0);
  }
  fPIDOnlyTOF->ConfigureTOF(3); // number of sigma TOF
  fPIDOnlyTOF->InitializePID();
  
  // PID object for TPC and TOF combined
  // Check if PID is set from outside (passed as argument)
  //  ::Info("AddTaskDxHFEParticleSelection",Form("Setting up new combined PID object"));
  AliHFEpid* fPID = new AliHFEpid("hfePid");
  if(!fPID->GetNumberOfPIDdetectors()) { 
    fPID->AddDetector("TOF",0);
    fPID->AddDetector("TPC",1);
  }
  //Add settings for asymmetric cut on nSigma TPC
  const int paramSize=4;
  Double_t params[paramSize];
  memset(params, 0, sizeof(Double_t)*paramSize);
  params[0]=-1.;
  fPID->ConfigureTPCdefaultCut(NULL, params, 3.);
  fPID->InitializePID();

   // PID for Only TPC
  AliHFEpid *fPIDOnlyTPC = new AliHFEpid("hfePidTPC");
  if(!fPIDOnlyTPC->GetNumberOfPIDdetectors()) { 
    fPIDOnlyTPC->AddDetector("TPC",0);
  }
  fPIDOnlyTPC->ConfigureTPCdefaultCut(NULL, params, 3.);
  fPIDOnlyTPC->InitializePID();

  //--------------------------------------------//
  //         Handling of pool-config            //
  //--------------------------------------------//
///______________________________________________________________________
 /// Info for Pool
 // TODO: Don't think we need the MC part of AliHFCorrelator, needs to be checked
 AliAnalysisCuts* poolConfiguration=NULL;
 // if (poolConfigFile.IsNull()) {
   // load the default configuration from below if no file is specified
   if (system==0) poolConfiguration=createDefaultPoolConfig();
   else if (system==1) poolConfiguration=createPbPbPoolConfig();
   else if (system==2) poolConfiguration=createpPbPoolConfig();
 
 // } else {
 //   // load configuration from file, and abort if something goes wrong
 //   TFile* filePoolConfiguration=TFile::Open(poolConfigFile.Data());
 //   if(!filePoolConfiguration){
 //     ::Error("AddTaskDxHFECorrelation", Form("Pool configuration object file %s not found, exiting", poolConfigFile.Data()));
 //      return 0;
 //    }
 //    TObject* pObj=filePoolConfiguration->Get(poolInfoName);
 //    if (!pObj) {
 //      ::Error("AddTaskDxHFECorrelation", Form("No Pool configuration object with name '%s' found in file %s, exiting", poolInfoName, poolConfigFile.Data()));
 //      return 0;
 //    }
 //    poolConfiguration = dynamic_cast<AliHFAssociatedTrackCuts*>(pObj);
 //    if (!poolConfiguration) {
 //      ::Error("AddTaskDxHFECorrelation", Form("Pool configuration object '%s' has inconsistent class type %s, exiting", poolInfoName, pObj->ClassName()));
 //      return 0;
 //    }
 // }

   //  if(!poolConfiguration){
   //    ::Fatal("AddTaskDxHFECorrelation", Form("Pool configuration not found"));
   //    return 0;
   //  } 
  poolConfiguration->Print();



  //============================================//
  //       Handling of pool-config complete     //
  //============================================//


  //=========================================================
  //Create TList of cut (and pid) objects for D0 or electron
  TList *Cutlist = new TList;
  // Particle==AliAnalysisTaskDxHFEParticleSelection::kElectron;
  /*if(Particle==AliAnalysisTaskDxHFEParticleSelection::kD0){
    Cutlist->SetName("cut objects D0");
    Cutlist->Add(RDHFD0toKpi);
  }
  else if(Particle==AliAnalysisTaskDxHFEParticleSelection::kElectron){
  */Cutlist->Add(RDHFD0toKpi);
    Cutlist->SetName("cut objects HFE");
    Cutlist->Add(hfecuts);
    Cutlist->Add(fPID);
    Cutlist->Add(fPIDOnlyTOF);
    Cutlist->Add(fPIDOnlyTPC);
    Cutlist->Add(poolConfiguration);
    printf("\n\n\ntest\n\n\n");
    Cutlist->Print();
    printf("\n\n\ntest\n\n\n");
    //}

  //[FIXME] Add HF asso track cuts
 TFile* fout=new TFile("Cutlist.root","recreate");   //set this!! 
 
 fout->cd();
 Cutlist->Write();
 fout->Close();


  //Save to *.root file


}


//---------------------------------------------------------//
//                                                         //
//                        NOTE!                            //
//                                                         //
//         These two pool-config functions have            //
//        been taken from AddTaskDxHFECorrelation          //
//                                                         //
//                                                         //
//---------------------------------------------------------//

// Note: AliHFAssociatedTrackCuts keeps an instance of the external
// pointer, the arrays thus need to be global
// TODO: try a proper implementation of AliHFAssociatedTrackCuts later
const Int_t    nofMBins=5;
/*const */Double_t MBins[nofMBins+1]={0,20,40,60,80,500};
/*const */Double_t * MultiplicityBins = MBins;
const Int_t    nofZBins=5;
/*const */Double_t ZBins[nofZBins+1]={-10,-5,-2.5,2.5,5,10};
/*const */Double_t *ZVrtxBins = ZBins;

AliAnalysisCuts* createDefaultPoolConfig()
{
  AliHFAssociatedTrackCuts* HFCorrelationCuts=new AliHFAssociatedTrackCuts();
  HFCorrelationCuts->SetName("PoolInfo");
  HFCorrelationCuts->SetTitle("Info on Pool for EventMixing");

  // NEED to check this
  HFCorrelationCuts->SetMaxNEventsInPool(200);
  HFCorrelationCuts->SetMinNTracksInPool(100);
  HFCorrelationCuts->SetMinEventsToMix(8);
  HFCorrelationCuts->SetNofPoolBins(nofZBins,nofMBins); // Note: the arrays have dimension x+1
  HFCorrelationCuts->SetPoolBins(ZVrtxBins,MultiplicityBins);

  TString description = "Info on Pool for EventMixing";   
  HFCorrelationCuts->AddDescription(description);

  return HFCorrelationCuts;
}

AliAnalysisCuts* createPbPbPoolConfig()
{
  AliHFAssociatedTrackCuts* HFCorrelationCuts=new AliHFAssociatedTrackCuts();
  HFCorrelationCuts->SetName("PoolInfo");
  HFCorrelationCuts->SetTitle("Info on Pool for EventMixing");

  // NEED to check this
  HFCorrelationCuts->SetMaxNEventsInPool(250);
  HFCorrelationCuts->SetMinNTracksInPool(80);
  HFCorrelationCuts->SetMinEventsToMix(5);
  HFCorrelationCuts->SetNofPoolBins(nofZBins,nofMBins); // Note: the arrays have dimension x+1
  HFCorrelationCuts->SetPoolBins(ZVrtxBins,MultiplicityBins);

  TString description = "Info on Pool for EventMixing";   
  HFCorrelationCuts->AddDescription(description);

  return HFCorrelationCuts;
}


AliAnalysisCuts* createpPbPoolConfig()
{
  AliHFAssociatedTrackCuts* HFCorrelationCuts=new AliHFAssociatedTrackCuts();
  HFCorrelationCuts->SetName("PoolInfo");
  HFCorrelationCuts->SetTitle("Info on Pool for EventMixing");

  // NEED to check this
  HFCorrelationCuts->SetMaxNEventsInPool(200);
  HFCorrelationCuts->SetMinNTracksInPool(100);
  HFCorrelationCuts->SetMinEventsToMix(8);
  HFCorrelationCuts->SetNofPoolBins(nofZBins,nofMBins); // Note: the arrays have dimension x+1
  HFCorrelationCuts->SetPoolBins(ZVrtxBins,MultiplicityBins);

  TString description = "Info on Pool for EventMixing";   
  HFCorrelationCuts->AddDescription(description);

  return HFCorrelationCuts;
}
