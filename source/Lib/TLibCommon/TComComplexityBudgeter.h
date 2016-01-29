/* 
 * File:   TComComplexityBudgeter.h
 * Author: mateusgrellert
 *
 * Created on November 27, 2012, 5:14 PM
 */

#ifndef TCOMCOMPLEXITYBUDGETER_H
#define	TCOMCOMPLEXITYBUDGETER_H

#include "TComComplexityManagement.h"

enum AllocationAlgorithm{
    FRAME_LEVEL_ALLOCATION,
    PRIORITY_OBLIVIOUS_ALLOCATION,
    PRIORITY_AWARE_ALLOCATION,
};


using namespace std;

typedef vector<unsigned int> config;
typedef int CTU_HIST_TYPE;

class TComComplexityBudgeter {
    
public:
    static vector<vector <config> > psetMap;
    static vector<vector <CTU_HIST_TYPE> > ctuHistory; //stores either depth or ctu time or anything that can be used to classify a ctu
//    static vector<vector <int> > ctuCount; //stores either depth or ctu time or anything that can be used to classify a ctu

    static long initCtuTime, endCtuTime;
    static double maxCtuTime, minCtuTime, nCTUs;
    static double frameBudget,reqSavings, prev_avgTimeFactor, avgTimeFactor;
    static unsigned int gopSize;
    static int psetCounter[NUM_PSETS], budgetCount, psetTotal;
    
    static unsigned int picWidth;
    static unsigned int picHeight;
    static unsigned int intraPeriod;

    static unsigned int currPoc;
    static unsigned int budgetAlgorithm;
    static unsigned int fixPSet;
    static int currPredSavings;
    
    static int searchRange, bipredSR;
    static bool hadME;
    static Int enFME;
    static bool testAMP;
    static unsigned int testSMP;
    static bool enRDOQ;
    static unsigned int maxNumRefPics;
    static unsigned int maxCUDepth;
    static unsigned int maxTUDepth;
    
    static ofstream budgetFile;
    
    
    TComComplexityBudgeter();
    
    static Void init(UInt, UInt, UInt);

    static Void printBudgetStats();
    static Void resetBudgetStats();
    
    static bool promote(UInt, UInt);
    static bool demote(UInt, UInt);
    
    static Void uniformBudget();

    static Void priorityAwareAllocation();
    static int demoteCTUSWithDepthUpTo(int demote_depth, int nb = 9999999);
    static int promoteCTUSWithDepthUpFrom(int depth, int nb = 9999999);
    static void priorityObliviousBudget();

    static Void frameLevelAllocation();
    static Void distributeBudget();
    static void updateEstimationAndStats(Int old, UInt neww);

    static Void setDepthHistory(TComDataCU *&);
    static Void setTimeHistory(TComDataCU *&);
    static void resetCTUStats();

    static Void setFrameBudget(Double);
    static Double estimateTime(UInt);
    static Void updateCTUConfig(TComDataCU*& cu);
    static Void updateFrameConfig(UInt);
    static void updateCodingStructures(TEncCfg* encCfg, TComSPS* sps, TEncSearch* searchCfg);
    static Void resetConfig();
    static Void setPSetToCTU(UInt i, UInt j, UInt prof);

};


#endif	/* TCOMCOMPLEXITYBUDGETER_H */

