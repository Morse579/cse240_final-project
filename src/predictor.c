//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName1 = "Yiran Chen";
const char *studentID1   = "A13722673";
const char *email1       = "yic328@ucsd.edu";

const char *studentName2 = "Yushan Liu";
const char *studentID2   = "A13817367";
const char *email2       = "yul579@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

// Unique data structure for gshare
uint32_t mask;
uint8_t* BHT_gshare;
uint32_t history;

// Unique data structure for tournament
uint32_t gMask;
uint32_t lMask;
uint32_t pcMask;
uint32_t globalHistory;
uint8_t* gBHT; // global history
uint8_t* lBHT; // local history
uint32_t* PHT; 
uint8_t* choicePredictor;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  // Init a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      init_predictor_gshare();
      break;
    case TOURNAMENT:
      init_predictor_tournament();
      break;
    case CUSTOM:
      init_predictor_custom();
      break;
    default:
      break;
  }
}

void
init_predictor_gshare()
{
  uint32_t length = (1 << ghistoryBits); // 2^k
  history = NOTTAKEN;
  mask = length - 1; // 111111...(k times)
  BHT_gshare = malloc(length * sizeof(uint8_t));
  for(uint32_t i = 0; i < length; i++){
    BHT_gshare[i] = WN; // every predictors initalized to be WN
  }
}

void
init_predictor_tournament()
{
  globalHistory = NOTTAKEN;

  gMask = (1 << ghistoryBits) - 1;
  gBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
  choicePredictor = malloc((1 << ghistoryBits) * sizeof(uint8_t));
  for(uint32_t i = 0; i < (1 << ghistoryBits); i++){
    gBHT[i] = WN; // every predictors initalized to be WN
    choicePredictor[i] = 2; // weakly select the global predictor
  }

  lMask = (1 << lhistoryBits) - 1;
  lBHT = malloc((1 << lhistoryBits) * sizeof(uint8_t));
  for(uint32_t i = 0; i < (1 << lhistoryBits); i++){
    lBHT[i] = WN; // every predictors initalized to be WN
  }

  pcMask = (1 << pcIndexBits) - 1;
  PHT = malloc((1 << pcIndexBits) * sizeof(uint32_t));
  for(uint32_t i = 0; i < (1 << pcIndexBits); i++){
    PHT[i] = 0;
  }
}

void
init_predictor_custom()
{
  ghistoryBits = 13;
  lhistoryBits = 11;
  pcIndexBits = 11;
    
  globalHistory = NOTTAKEN;
  gMask = (1 << ghistoryBits) - 1;
  gBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
  choicePredictor = malloc((1 << ghistoryBits) * sizeof(uint8_t));
  for(uint32_t i = 0; i < (1 << ghistoryBits); i++){
    gBHT[i] = WN; // every predictors initalized to be WN
    choicePredictor[i] = 2; // weakly select the global predictor
  }

  lMask = (1 << lhistoryBits) - 1;
  lBHT = malloc((1 << lhistoryBits) * sizeof(uint8_t));
  for(uint32_t i = 0; i < (1 << lhistoryBits); i++){
    lBHT[i] = WWN; // using 3-bit predictor here
  }

  pcMask = (1 << pcIndexBits) - 1;
  PHT = malloc((1 << pcIndexBits) * sizeof(uint32_t));
  for(uint32_t i = 0; i < (1 << pcIndexBits); i++){
    PHT[i] = 0;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return make_prediction_gshare(pc);
    case TOURNAMENT:
      return make_prediction_tournament(pc);
    case CUSTOM:
      return make_prediction_custom(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

uint8_t
make_prediction_gshare(uint32_t pc)
{
  uint8_t prediction = BHT_gshare[(pc & mask)^(history & mask)];
  if(prediction >= WT){
    return TAKEN;
  } else {
    return NOTTAKEN;
  }
}

uint8_t
make_prediction_tournament(uint32_t pc)
{
  uint8_t chooser = choicePredictor[globalHistory];
  int choice;

  // global prediction via global history
  uint8_t globalPrediction = gBHT[globalHistory];

  // local prediction via PHT and local history
  uint32_t index = PHT[pc & pcMask];
  uint8_t localPrediction = lBHT[index];

  if(chooser >= 2){ // using global predictor
    choice = globalPrediction >= WT ? TAKEN : NOTTAKEN;
  } else {
    choice = localPrediction >= WT ? TAKEN : NOTTAKEN;
  }
  return choice;
}

uint8_t
make_prediction_custom(uint32_t pc)
{
  uint32_t xorGlobalIndex = (pc & pcMask) ^ (globalHistory & gMask);
  uint8_t chooser = choicePredictor[xorGlobalIndex];
  int choice;

  // global prediction via global history
  uint8_t globalPrediction = gBHT[xorGlobalIndex];

  // local prediction via PHT and local history
  uint32_t index = PHT[pc & pcMask];
  uint8_t localPrediction = lBHT[index];

  if(chooser >= 2){ // using global predictor
    choice = globalPrediction >= WT ? TAKEN : NOTTAKEN;
  } else {
    choice = localPrediction >= WWT ? TAKEN : NOTTAKEN;
  }
  return choice;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  // train a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      train_predictor_gshare(pc, outcome);
      break;
    case TOURNAMENT:
      train_predictor_tournament(pc, outcome);
      break;
    case CUSTOM:
      train_predictor_custom(pc, outcome);
      break;
    default:
      break;
  }
}

void train_predictor_gshare(uint32_t pc, uint8_t outcome)
{
  uint32_t index = (pc & mask) ^ (history & mask);
  uint8_t prediction = BHT_gshare[index];

  // update the global history
  history = ((history << 1) + outcome) & mask; // avoid overflow ?

  // update BHT
  if(outcome == TAKEN && prediction != ST){
    BHT_gshare[index]++;
  } 
  else if (outcome == NOTTAKEN && prediction != SN){
    BHT_gshare[index]--;
  }
}

void train_predictor_tournament(uint32_t pc, uint8_t outcome)
{
  uint8_t chooser = choicePredictor[globalHistory];

  // global prediction via global history
  uint8_t globalPrediction = gBHT[globalHistory];
  uint8_t globalAction = globalPrediction >= WT ? TAKEN : NOTTAKEN;

  // local prediction via PHT and local history
  uint32_t index = PHT[pc & pcMask];
  uint8_t localPrediction = lBHT[index];
  uint8_t localAction = localPrediction >= WT ? TAKEN : NOTTAKEN;

  // update choice predictor when making different predictions
  if(globalAction != localAction){
    // +1/-1 to the correct predictor
    if(globalAction == outcome && chooser != 3){
      choicePredictor[globalHistory]++;
    }
    else if(localAction == outcome && chooser != 0){
      choicePredictor[globalHistory]--;
    }
  }

  // update BHTs
  if(outcome == TAKEN){
    if(globalPrediction != ST) gBHT[globalHistory]++;
    if(localPrediction != ST) lBHT[index]++;
  }
  else{
    if(globalPrediction != SN) gBHT[globalHistory]--;
    if(localPrediction != SN) lBHT[index]--;
  }

  // update global history and local history
  globalHistory = ((globalHistory << 1) + outcome) & gMask;
  PHT[pc & pcMask] = ((PHT[pc & pcMask] << 1) + outcome) & pcMask;
}

void train_predictor_custom(uint32_t pc, uint8_t outcome)
{
  // using pc ^ global history instead of just globalHistory
  uint32_t xorGlobalIndex = (pc & pcMask) ^ (globalHistory & gMask);
  
  uint8_t chooser = choicePredictor[xorGlobalIndex];

  // global prediction via global history
  uint8_t globalPrediction = gBHT[xorGlobalIndex];
  uint8_t globalAction = globalPrediction >= WT ? TAKEN : NOTTAKEN;

  // local prediction via PHT and local history
  uint32_t index = PHT[pc & pcMask];
  uint8_t localPrediction = lBHT[index];
  uint8_t localAction = localPrediction >= WWT ? TAKEN : NOTTAKEN;

  // update choice predictor when making different predictions
  if(globalAction != localAction){
    // +1/-1 to the correct predictor
    if(globalAction == outcome && chooser != 3){
      choicePredictor[xorGlobalIndex]++;
    }
    else if(localAction == outcome && chooser != 0){
      choicePredictor[xorGlobalIndex]--;
    }
  }

  // update BHTs
  if(outcome == TAKEN){
    if(globalPrediction != ST) gBHT[xorGlobalIndex]++;
    if(localPrediction != SST) lBHT[index]++;
  }
  else{
    if(globalPrediction != SN) gBHT[xorGlobalIndex]--;
    if(localPrediction != SSN) lBHT[index]--;
  }

  // update global history and local history
  globalHistory = ((globalHistory << 1) + outcome) & gMask;
  PHT[pc & pcMask] = ((PHT[pc & pcMask] << 1) + outcome) & pcMask;
}
