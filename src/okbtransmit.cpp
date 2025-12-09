#include "okbtransmit.h"


void sendToNode(RFM69_ATC &radio, List<Payload> &transferListe)
{
  Payload myPayload;
//  digitalWrite(NRFTX, HIGH);
  
  if(!transferListe.isEmpty())
  {
    myPayload = transferListe[0];

    Serial.print("Sending struct ");
    if (radio.sendWithRetry(myPayload.nodeId, (const void*)(&myPayload), sizeof(Payload)))
    {
      Serial.print(" ok!");
      printPayload(myPayload);
    }
    else 
    {
      Serial.print(" nothing...");

      if(myPayload.setNumber == 1 && myPayload.maxSetNumber == 1)
      {
        transferListe.removeFirst();  // Eintrag aus Liste löschen
        Serial.println("Lösche Transfer aus Liste");
      }
      else
      {
        deleleAllSameItems(myPayload.nodeId, myPayload.sendTime, transferListe);   // alle zugehörigen Nachrichten Löschen
      }
        
    }
  }

//  digitalWrite(NRFTX, LOW);

}

bool getRadioData(RFM69_ATC &radio, List<Payload> &transferListe)
{
    Payload myPayload;
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");
    // if (spy) Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");

    if (radio.DATALEN != sizeof(Payload))
      Serial.print("Invalid payload length received, not matching Payload struct!");
    else
    {
      myPayload = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      myPayload.nodeId = radio.SENDERID;  // Sender ID speichern
      transferListe.add(myPayload);       // das Ganze in eine Liste
      
    }

    if (radio.ACKRequested())
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      Serial.print(" - ACK sent.");
    }
    if(myPayload.maxSetNumber == myPayload.setNumber) // letztes oder einziges Teil des Sets empfangen
      return true;
    else
      return false;
}

void deleleAllSameItems(int nodeID, unsigned long Zeit, List<Payload> &transferListe )
{
  Payload myPayload;
  if(!transferListe.isEmpty())
  {
    Serial.println("Lösche aus Liste :");
    for(uint16_t i = 0; i < transferListe.getSize(); i++)
    {
      myPayload = transferListe[i];
      if(myPayload.nodeId == nodeID && Zeit == myPayload.sendTime)  /// hier weiter machen
      {
        transferListe.remove(i);
        Serial.print("Lösche Index : ");
        Serial.print(i);
        i-=1;
      }
    }
  }
}


void splitToList( List<Payload>& myTransferList, int myNodeID, char transferString[])
{
  Payload myPayload;
  Serial.print("Payload : ");
  Serial.println(transferString);
  
  uint16_t stringLaenge = strlen(transferString);
  uint16_t lauf = 0;                              // über den gesamten String
  uint8_t teiler = sizeof(myPayload.nachricht);   // wie lang darf die Nachrit sein?
  uint8_t Anzahl = stringLaenge / teiler ;        // Anzahl der Pakete - 1
  uint8_t rest = stringLaenge % teiler;          // Anzahl der Zeichen die nach der for Schleife übrig bleiben

  Serial.print("Stringlaenge :" );
  Serial.println(stringLaenge);
  Serial.print("Teiler :" );
  Serial.println(teiler);
  Serial.print("Anzahl :" );
  Serial.println(Anzahl);
  Serial.print("Rest :" );
  Serial.println(rest);
  

  myPayload.nodeId = myNodeID;
  myPayload.sendTime = millis();
  myPayload.maxSetNumber = Anzahl +1;

  for(uint8_t lauf = 0; lauf < Anzahl; lauf++)
  {
    strncpy(myPayload.nachricht,transferString + (lauf * 50), 50);
    myPayload.setNumber = lauf +1;
    myTransferList.add(myPayload);
    //Serial.println(myPayload.nachricht);
  }
  if(rest > 0)
  {
    Serial.println(lauf * 50);
    strncpy(myPayload.nachricht,transferString + (lauf * 50), rest);
    myPayload.nachricht[rest] = '\0';
    myPayload.setNumber = lauf +1;
    myTransferList.add(myPayload);
    Serial.println(myPayload.nachricht);
  }  
}


int combineFromList(List<Payload> &transferListe, char myString[])
{
  Payload myPayload;
  uint8_t maxAnzahl = 0;
  unsigned long zeitStempel;
  uint8_t durchlauf = 0;
  uint8_t lauf = 0;
  uint8_t retNode = 0;    // Rückgabewert, 0= nix gefunden, ansonsten Absendernode
  uint8_t zz = 0;

  if(!transferListe.isEmpty())
  {
    Serial.println("Addiere aus Liste :");
      myPayload = transferListe[transferListe.getSize()-1]; // letztes Element
      retNode = myPayload.nodeId;
      if(myPayload.maxSetNumber == 1 && myPayload.setNumber == 1)
      {
        strcpy(myString, myPayload.nachricht);
        transferListe.removeLast();
        return retNode;
      }
      else
      {
        while(durchlauf <= maxAnzahl && lauf < transferListe.getSize())
        {
          myPayload = transferListe[lauf];
          zeitStempel = myPayload.sendTime;
          maxAnzahl = myPayload.maxSetNumber;
          if(myPayload.nodeId == retNode && zeitStempel == myPayload.sendTime)  /// hier weiter machen
          {
            transferListe.remove(lauf);
            Serial.print("Lösche Index : ");
            Serial.print(lauf);
            if(myPayload.maxSetNumber == myPayload.setNumber)
              zz  = strlen(myPayload.nachricht + 1);  // wegen abschießendem '\0'
            else
              zz = 50;
            for(int z = 0; z < zz; z++)
                myString[z+(myPayload.setNumber * 50)] = myPayload.nachricht[z];
            durchlauf += 1;
          }
          lauf += 1;
        }
        if( durchlauf == maxAnzahl )
          return retNode;
      }
  }
  return 0;
}

void printPayload( Payload &myPayload )
{
  Serial.print("NodeID : ");
  Serial.println(myPayload.nodeId);
  Serial.print("Max Setnumber : ");
  Serial.println(myPayload.maxSetNumber);
  Serial.print("Setnummer");
  Serial.println(myPayload.setNumber);
  Serial.print("Zeitstempal");
  Serial.println(myPayload.sendTime);
  
  Serial.print("Nachricht");
  if(myPayload.setNumber == myPayload.maxSetNumber)
    Serial.println(myPayload.setNumber);
  else
  {
    for(int lauf = 0; lauf < 50; lauf++)
      Serial.print(myPayload.nachricht[lauf]);
    Serial.println();
  }
}
