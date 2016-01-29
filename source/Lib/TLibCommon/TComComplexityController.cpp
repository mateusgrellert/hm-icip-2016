#include "TComComplexityController.h"

double TComComplexityController::SP;
double TComComplexityController::PV;
double TComComplexityController::avgPV;
double TComComplexityController::avgGopPV;
double TComComplexityController::accGopPV;
double TComComplexityController::accumPV;
double TComComplexityController::error;
double TComComplexityController::prevError;
double TComComplexityController::accumError;
double TComComplexityController::controlOutput;
double TComComplexityController::kd;
double TComComplexityController::ki;
double TComComplexityController::kp;
double TComComplexityController::targetSavings;
unsigned int TComComplexityController::trainingPeriod;
int TComComplexityController::gopSize;
int TComComplexityController::currPoc;
double TComComplexityController::frameTime;
clock_t TComComplexityController::tBegin;
unsigned int TComComplexityController::frameCount;
std::ofstream TComComplexityController::pidFile;
bool TComComplexityController::controlActive;

void TComComplexityController::init(int gop){
    gopSize = gop;
    //gopSize = 1;

    frameCount = 0;
    frameTime = 0.0;
    
    error = 0.0;
    prevError = 0.0;
    accumError = 0.0;    
    controlOutput = 0.0;
    controlActive = false;
    accumPV = 0.0;
    avgPV = 0.0;
    avgGopPV = 0.0;
}


void TComComplexityController::updateSP(){
    if(trainingPeriod > 0)
        SP = avgPV*(1-targetSavings);

}

void TComComplexityController::updatePV(int poc){
    if(poc < NUM_RD_FRAMES - 1) return;
    
    PV = frameTime;
    accumPV += PV;
    if(frameCount % gopSize == 0)
        accGopPV = PV;
    else
        accGopPV += PV;
    
    frameCount++;
    

    
    avgPV = accumPV/frameCount;
}

double TComComplexityController::calcPID(){

    double PIDOutput;
    //double factor[] = {1.5, 0.85, 0.8, 0.85};
    if(frameCount % gopSize == 0){
        avgGopPV = accGopPV/gopSize;
        prevError = error;
        error = SP - avgGopPV;

        accumError += error;           

        PIDOutput = kp*error + ki*accumError+ kd*(error - prevError);
        controlOutput = (avgGopPV + PIDOutput);
    }
    return controlOutput;
    
}

void TComComplexityController::beginFrameTimer(){
    tBegin = clock();
}

void TComComplexityController::endFrameTimer(){
    clock_t tAfter = clock();
    frameTime = (double) (tAfter - tBegin)/ CLOCKS_PER_SEC;
}

void TComComplexityController::printControlStats(double estimatedTime){
    openPidFile();
    pidFile << currPoc;
    pidFile << "\t" << controlActive << "\t" <<  SP << "\t" << PV << "\t" << avgGopPV << "\t" << avgPV << "\t" << controlOutput;
    pidFile << "\t" << estimatedTime << "\t\t\t" << kp*error << "\t" << ki*accumError << "\t" <<  kd*(error - prevError) << endl;
    
#if DISPLAY_VERBOSE
    printf( "@control\tControl\tSP\tPV\tGOP PV\tAVG PV\tPID\tREQSAV\t\t\te\tSe\tde\n");
    printf("@control\t%d\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f" , controlActive ,SP , PV ,avgGopPV , avgPV , controlOutput);
    printf( "\t%.2f\t\t\t%.2f\t%.2f\t%.2f\n" ,estimatedTime,kp*error , ki*accumError,  kd*(error - prevError));
#endif

}


bool TComComplexityController::activateControl(int poc){
    currPoc = poc;
    controlActive = (poc >= (NUM_RD_FRAMES + trainingPeriod));
    if (poc == (NUM_RD_FRAMES + trainingPeriod))
        updateSP();
    return controlActive;
}

Void TComComplexityController::openPidFile(){ 
    if(!pidFile.is_open()){
        pidFile.open("controlOut.csv",ofstream::out);
        pidFile << "POC\tControl Active\tSP\tPV\tGOP PV\tAVG PV\tPID\tEST\t\t\te\tSe\tde\n";
    }
}