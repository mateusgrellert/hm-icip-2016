#include "TComComplexityBudgeter.h"
#include "TComComplexityController.h"
#include "TComCABACTables.h"
#include "TLibEncoder/TEncCfg.h"
#include "TLibEncoder/TEncSearch.h"
#include <cmath>

using namespace std;

int TComComplexityBudgeter::psetCounter[NUM_PSETS];
int TComComplexityBudgeter::psetTotal;
int TComComplexityBudgeter::budgetCount;
long TComComplexityBudgeter::initCtuTime;
long TComComplexityBudgeter::endCtuTime;
double TComComplexityBudgeter::nCTUs;
double TComComplexityBudgeter::maxCtuTime;
double TComComplexityBudgeter::minCtuTime;
vector<vector <CTU_HIST_TYPE> > TComComplexityBudgeter::ctuHistory;
//vector<vector <int> > TComComplexityBudgeter::ctuCount;

vector<vector <config> > TComComplexityBudgeter::psetMap;
UInt TComComplexityBudgeter::gopSize;
Int TComComplexityBudgeter::currPredSavings;
UInt TComComplexityBudgeter::picWidth;
UInt TComComplexityBudgeter::picHeight;
UInt TComComplexityBudgeter::maxCUDepth;
UInt TComComplexityBudgeter::maxTUDepth;
UInt TComComplexityBudgeter::maxNumRefPics;
Int TComComplexityBudgeter::searchRange;
Int TComComplexityBudgeter::bipredSR;
Bool TComComplexityBudgeter::hadME;
Int TComComplexityBudgeter::enFME;
Bool TComComplexityBudgeter::enRDOQ;
Bool TComComplexityBudgeter::testAMP;
UInt TComComplexityBudgeter::testSMP;
UInt TComComplexityBudgeter::currPoc;
UInt TComComplexityBudgeter::budgetAlgorithm;
unsigned int TComComplexityBudgeter::fixPSet; // for budget algorithm 6
Double TComComplexityBudgeter::frameBudget; 
Double TComComplexityBudgeter::reqSavings;
Double TComComplexityBudgeter::prev_avgTimeFactor;
Double TComComplexityBudgeter::avgTimeFactor;
std::ofstream TComComplexityBudgeter::budgetFile;


Void TComComplexityBudgeter::init(UInt w, UInt h, UInt gop){
    resetConfig();
    vector<CTU_HIST_TYPE> tempHistRow;
    vector<int> tempCountRow;
    vector<config> tempConfigRow;
    config conf;
    avgTimeFactor = 0;
    reqSavings = 0.0;
    gopSize = gop;
    
    picWidth = w;
    picHeight = h;
    nCTUs = w*h/(4096.0);
            
    currPredSavings = PS0;

    maxCtuTime = 0.0;
    minCtuTime = MAX_INT;

    for(int i = 0; i < NUM_PSETS; i++)
        psetCounter[i] = 0;
    psetCounter[PS0] = nCTUs;
    
    for(int i = 0; i < (w >> 6) + 1; i++){
        tempHistRow.clear();
        tempConfigRow.clear();
        for(int j = 0; j < (h >> 6) + 1; j++){
            conf.clear();
            tempHistRow.push_back(-1);
            tempCountRow.push_back(0);

            // for config map
            for(int k = 0; k < NUM_PARAMS; k++){
                conf.push_back(PSET_TABLE[PS0][k]);
            }
            conf.push_back(PS0);
            tempConfigRow.push_back(conf);
        }
        ctuHistory.push_back(tempHistRow);
       // ctuCount.push_back(tempCountRow);
        psetMap.push_back(tempConfigRow);
    }
    budgetCount = 0;
}



void TComComplexityBudgeter::setDepthHistory(TComDataCU *&pcCU){
    
    unsigned int x = pcCU->getCUPelX();
    unsigned int y = pcCU->getCUPelY();
    double avg_depth=0;
    for(int i = 0; i < pcCU->getTotalNumPart(); i++)
      avg_depth += (int) pcCU->getDepth(i);
    //if((short)(depth) > ctuHistory[x >> 6][y >> 6])
    cout << avg_depth/256.0 << endl;
    avg_depth = round( avg_depth/256.0);
    ctuHistory[x >> 6][y >> 6] = (CTU_HIST_TYPE) avg_depth;
   
//    ctuCount[x >> 6][y >> 6]++;
    //cout << ctuHistory[x >> 6][y >> 6] << endl;

}

void TComComplexityBudgeter::setTimeHistory(TComDataCU *&pcCU){
    
    UInt x = pcCU->getCUPelX();
    UInt y = pcCU->getCUPelY();

    double ctu_time = (double)(endCtuTime - initCtuTime)*1.0/CLOCKS_PER_SEC*1.0;
    ctuHistory[x >> 6][y >> 6] = ctu_time;
    
    if (ctu_time > maxCtuTime)
        maxCtuTime = ctu_time;
    else if(ctu_time < minCtuTime)
        minCtuTime = ctu_time;
    
}

void TComComplexityBudgeter::updateCodingStructures(TEncCfg* encCfg, TComSPS* sps, TEncSearch* searchCfg){
    encCfg->setUseAMP(testAMP);
    encCfg->setTestRect(testSMP);
    encCfg->setUseHADME(hadME);
    encCfg->setRefFrames(maxNumRefPics);
    encCfg->setUseRDOQ(enRDOQ);
    encCfg->setFME(enFME);
    
    sps->setQuadtreeTUMaxDepthInter(maxTUDepth);
    sps->setQuadtreeTUMaxDepthIntra(maxTUDepth);
    sps->setUseAMP(testAMP); 
    for (int i = 0; i < g_uiMaxCUDepth-g_uiAddCUDepth; i++ )
        sps->setAMPAcc(i, testAMP);
    
    searchCfg->setSearchRange(searchRange);
    searchCfg->setSearchRangeBipred(bipredSR);
    searchCfg->setAdaptSearchRange(searchRange);
    
  //  g_uiMaxCUDepth = maxCUDepth;
    
}

void TComComplexityBudgeter::updateFrameConfig(UInt PSet){

    bipredSR      = PSET_TABLE[PSet][0];
    searchRange   = PSET_TABLE[PSet][1];
    testSMP       = PSET_TABLE[PSet][2];
    maxTUDepth    = PSET_TABLE[PSet][3];
    testAMP       = PSET_TABLE[PSet][4];
    hadME         = PSET_TABLE[PSet][5];
    maxNumRefPics = PSET_TABLE[PSet][6];
    enRDOQ        = PSET_TABLE[PSet][7];
    enFME         = PSET_TABLE[PSet][8];
    maxCUDepth    = PSET_TABLE[PSet][9];
}

void TComComplexityBudgeter::updateCTUConfig(TComDataCU*& cu){
    Int x = cu->getCUPelX() >> 6;
    Int y = cu->getCUPelY() >> 6;
      // bipred sr, sr, testrect, tu depth, amp, had me, num refs, rdoq, cu depth, 

    bipredSR      = psetMap[x][y][0];
    searchRange   = psetMap[x][y][1];
    testSMP       = psetMap[x][y][2];
    maxTUDepth    = psetMap[x][y][3];
    testAMP       = psetMap[x][y][4];
    hadME         = psetMap[x][y][5];
    maxNumRefPics = psetMap[x][y][6];
    enRDOQ        = psetMap[x][y][7];
    enFME         = psetMap[x][y][8];
    maxCUDepth    = psetMap[x][y][9];
}

void TComComplexityBudgeter::resetConfig(){
  
      // bipred sr, sr, testrect, tu depth, amp, had me, num refs, rdoq, cu depth, 

    bipredSR      = 4;
    searchRange   = 64;
    testSMP       = TEST_RECT_GOP;
    maxTUDepth    = 3;
    testAMP       = 1;
    hadME         = 1;
    maxNumRefPics = 4;
    enRDOQ        = 1;
    maxCUDepth    = 4;
    enFME = 2;
}



bool TComComplexityBudgeter::promote(UInt ctux, UInt ctuy){
    int new_pset = psetMap[ctux][ctuy][NUM_PARAMS]-1; // upgrading pset
    
    if(new_pset >= PS0){
        setPSetToCTU(ctux,ctuy,new_pset);
        psetCounter[new_pset]++;
        psetCounter[new_pset+1]--;
        return true;
    }
    return false;
}

bool TComComplexityBudgeter::demote(UInt ctux, UInt ctuy){
    UInt new_pset = psetMap[ctux][ctuy][NUM_PARAMS]+1; // downgrading pset
    
        
    //int max_pset = (int) (TComComplexityController::targetSavings*NUM_PSETS)+1;
    //int min_pset = (int) (TComComplexityController::targetSavings*NUM_PSETS)-1;
    //    min_pset = (min_pset < 0) ? 0 : min_pset;
   //     max_pset = (max_pset >= NUM_PSETS) ? NUM_PSETS-1 : max_pset;
    
    if(new_pset < NUM_PSETS){
        setPSetToCTU(ctux,ctuy,new_pset);
        psetCounter[new_pset]++;
        psetCounter[new_pset-1]--;
        return true;
    }

    return false;
}


void TComComplexityBudgeter::priorityObliviousBudget(){

    reqSavings = (1-(frameBudget/(TComComplexityController::PV))) - 0;
    
    int nbCTUsToDemote = 0;
    nbCTUsToDemote = (int) round(nCTUs*(reqSavings/0.2));
    

    while(nbCTUsToDemote > 0 and psetCounter[NUM_PSETS-1] != nCTUs){
        for(int i = 0; i < ctuHistory.size(); i++){
            for(int j = 0; j < ctuHistory[0].size() and nbCTUsToDemote > 0; j++){
                if (ctuHistory[i][j] == -1) // sometimes the history table has more nodes than CTUs
                    continue;
                if(demote(i,j)){
                    nbCTUsToDemote--;            
                }
            }
        }
    }
        

    while(nbCTUsToDemote < 0 and psetCounter[PS0] != nCTUs){
        for(int i = 0; i < ctuHistory.size(); i++){
            for(int j = 0; j < ctuHistory[0].size() and nbCTUsToDemote < 0; j++){
                if (ctuHistory[i][j] == -1) // sometimes the history table has more nodes than CTUs
                    continue;
                if(promote(i,j)){
                    nbCTUsToDemote++;            
                }
            }
        }
    }
    

}

        


Void TComComplexityBudgeter::frameLevelAllocation() {
    reqSavings = (int) round(1-(frameBudget/TComComplexityController::PV))*10;
    int reqPreset = reqSavings;
    
    for(int i = 0; i < ctuHistory.size(); i++){
       for(int j = 0; j < ctuHistory[0].size(); j++){
           setPSetToCTU(i,j,reqPreset);
                       

       }
    }
    updateFrameConfig(reqPreset);
    
}


Void TComComplexityBudgeter::priorityAwareAllocation(){
    reqSavings = 1-(frameBudget/TComComplexityController::PV);
    int nbCTUsToDemote = 0;

    nbCTUsToDemote = (int) round(nCTUs*(reqSavings/0.2));

    int demotedCTUs;
    for(int demote_depth = 0; demote_depth <= 3; demote_depth++){
        while(nbCTUsToDemote > 0 and psetCounter[NUM_PSETS-1] != nCTUs){
            demotedCTUs = demoteCTUSWithDepthUpTo(demote_depth, nbCTUsToDemote);
            nbCTUsToDemote -= demotedCTUs;   
        }
    }
            
                        
    int promotedCTUs;

    for(int promote_depth = 3; promote_depth >= 0 ; promote_depth--){
          while(nbCTUsToDemote < 0 and psetCounter[PS0] != nCTUs){
            promotedCTUs = promoteCTUSWithDepthUpFrom(promote_depth, -nbCTUsToDemote);
            nbCTUsToDemote += promotedCTUs;   
        }
    }
    

}


int TComComplexityBudgeter::demoteCTUSWithDepthUpTo(int demote_depth, int nb){
    int nbCTUsDemoted = 0;

    for(int i = 0; i < ctuHistory.size(); i++){
        for(int j = 0; j < ctuHistory[0].size(); j++){
            if (ctuHistory[i][j] == -1) continue;

            int avg_depth = ctuHistory[i][j];   // check rounded avg depth
            if(avg_depth == demote_depth){
                    if(demote(i,j)){
                        nbCTUsDemoted++;
                    }
                    if (nbCTUsDemoted == nb)
                       return nbCTUsDemoted;
            }
        }
    }
    return nbCTUsDemoted;
}
int TComComplexityBudgeter::promoteCTUSWithDepthUpFrom(int promote_depth, int nb){
    int nbCTUsPromoted = 0;

    for(int i = 0; i < ctuHistory.size(); i++){
        for(int j = 0; j < ctuHistory[0].size(); j++){
            if (ctuHistory[i][j] == -1) continue;

            int avg_depth = ctuHistory[i][j];   // check rounded avg depth
            if(avg_depth == promote_depth){
                    if(promote(i,j) ){
                        nbCTUsPromoted++;
                    }
                    if (nbCTUsPromoted == nb)
                        return nbCTUsPromoted;
            }
        }
    }
    return nbCTUsPromoted;
}

Void TComComplexityBudgeter::setPSetToCTU(UInt i, UInt j, UInt pset){
      // bipred sr, sr, testrect, tu depth, amp, had me, num refs, rdoq, en fme,  cu depth, 
         
            psetMap[i][j][0] = PSET_TABLE[pset][0]; // bipred SR
            psetMap[i][j][1] = PSET_TABLE[pset][1]; // SR
            psetMap[i][j][2] = PSET_TABLE[pset][2]; // TestRect
            psetMap[i][j][3] = PSET_TABLE[pset][3]; // TU Depth
            psetMap[i][j][4] = PSET_TABLE[pset][4]; // AMP
            psetMap[i][j][5] = PSET_TABLE[pset][5]; // HAD ME
            psetMap[i][j][6] = PSET_TABLE[pset][6]; // Max Num Ref Pics
            psetMap[i][j][7] = PSET_TABLE[pset][7]; // rdoq
            psetMap[i][j][8] = PSET_TABLE[pset][8]; // EN FME
            psetMap[i][j][9] = PSET_TABLE[pset][9]; // cu depth
            psetMap[i][j][NUM_PARAMS] = pset;

}

Void TComComplexityBudgeter::distributeBudget(){

    
    switch(budgetAlgorithm){
        case FRAME_LEVEL_ALLOCATION:  frameLevelAllocation(); break;
        case PRIORITY_OBLIVIOUS_ALLOCATION:  priorityObliviousBudget(); break;
        case PRIORITY_AWARE_ALLOCATION:  priorityAwareAllocation(); break;
    }
    budgetCount++;
    
    printBudgetStats();

    resetCTUStats();
    
    maxCtuTime = 0.0;
    minCtuTime = MAX_INT;
}


Void TComComplexityBudgeter::printBudgetStats(){
    
    if(!budgetFile.is_open()){
        budgetFile.open("budgetDistribution.csv",ofstream::out);
        int inc = 110/NUM_PSETS;
        int num = 0;
        for (int i = 0; i < NUM_PSETS; i++){
            budgetFile << "PS" << num << "\t";
            num += inc;
        }
       budgetFile << endl;

    }


    
#if DISPLAY_VERBOSE
    cout << "@budget\t";
    int inc = 100/NUM_PSETS;
    int num = 0;
    for (int i = 0; i < NUM_PSETS; i++){
        cout << "PS" << num << "\t";
            num += inc;
    }
    cout << endl << "@budget\t";
#endif
    
    for (int i = 0; i < NUM_PSETS; i++){
        budgetFile << (Double) psetCounter[i]/nCTUs << "\t";
#if DISPLAY_VERBOSE
        cout << (Double) psetCounter[i]/nCTUs << "\t";
#endif
    }

    budgetFile << endl;
    
#if DISPLAY_VERBOSE
    cout << endl;
#endif
    
}

Void TComComplexityBudgeter::setFrameBudget(Double budget){
    frameBudget = budget;
}

void TComComplexityBudgeter::resetCTUStats(){
    for(int i = 0; i < ctuHistory.size(); i++){
        for(int j = 0; j < ctuHistory[0].size(); j++){
            ctuHistory[i][j] = -1;
            //ctuCount[i][j] = 0;
        }
    }
    
}



