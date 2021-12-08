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

// Data structure for gshare
uint32_t history;
uint32_t mask;
uint8_t* BHT_gshare;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  // Init a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      init_predictor_gshare();
      break;
    case TOURNAMENT:
    case CUSTOM:
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

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return make_prediction_gshare(pc);
    case TOURNAMENT:
    case CUSTOM:
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

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  // train a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      break;
    case GSHARE:
      train_predictor_gshare(pc, outcome);
      break;
    case TOURNAMENT:
    case CUSTOM:
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
