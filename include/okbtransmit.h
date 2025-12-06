#pragma once

#include <Arduino.h>
#include <List.hpp>
#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>     //get it here: https://www.github.com/lowpowerlab/rfm69

typedef struct {
  int           nodeId; //store this nodeId
  unsigned long sendTime; //wann gesendet
  uint8_t       setNumber; // aktuelle Datensatznummer
  uint8_t       maxSetNumber; // wieviele Datensätze
  char          nachricht[50];   //maximale Nachrichtenlänge
} Payload;

void sendToNode(RFM69_ATC &, List<Payload>&);
bool getRadioData(RFM69_ATC &, List<Payload>&); // return ist, ob der letzte oder einzige Part eines Satzes empfangen worden ist
void deleleAllSameItems(int, unsigned long, List<Payload>& );
void splitToList( List<Payload>&, int, char[] );
int combineFromList(List<Payload>&, char[] );  // Ausgangsliste, Ziel Payload return ist die NodeID
