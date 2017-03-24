/*
 ****************************************************************************************
 *     
 *     COMPANY: Purdue University Calumet Department of Computer Information Technology 
 *
 * DESCRIPTION: Cognimem CM1K KNN/RCE Machine Learning via Parallel Interfacing with Arduino-Due Micro controller 
 *
 * 	   VERSION: 1.0 (05/7/2015) 
 *
 * 		http://www.ricardocalix.com/researchfiles/mlhardware/mlhardware.htm
 *
 ***************************************************************************************
 */
#include <CM1KParallel.h>
#include <CognimemParallel.h>
#include <SPI.h>
#include <SD.h>
#include <math.h>
#include <CognimemTest.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Modify Following as needed
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Enable KNN: 1-Yes, 0-No.
int EN_KNN = 0;

// Train and Test Files from microSD card
// file name should be UPPERCASE and lenght should be 8 character or fewer string, and 3 character extension

char TrainFile[15] = "TRAIN1K.TXT";
char TestFile[15] = "TRAIN1K.TXT";


// Select Number of Train and Test samples to load to Arduino Due Memory
// Maximem Memory size is 96 KB ( maximum 1600 vectors of 41 features )
int NumRowsGlobalTrain=1000;
int NumRowsGlobalTest = 100;


// we are retrieving a "k" of one (asking for the first neurons that fired)
int k = 1; //2;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


long startTime ;
long elapsedTime ;
long startTime2 ;
long elapsedTime2 ;
int n_status;
volatile int vBusBusy = LOW;

// Board specific constants re
#define PIN_BUS_BUSY                    2
#define PIN_LED                        13

////////////////////////////////////////////////////////////////////
// darpa data

File myFile;

int NumColsGlobal=41; //40 features + 1 class
int FeaturesGlobal=40;


void setup() {

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BUS_BUSY, INPUT);
  
  // CM1K Parallel PIN Configuration 
  cm1k.begin();
  
  Serial.begin(9600);
  
}

void loop() {
    
    unsigned long result = 0;
    delay(100);
    cm1k.reset();
    delay(100);
    cognimem.clear();
    
    Serial.println("\n\n=========================================");
    Serial.println("  CM1K parallel Communication : Tic Toc Clock");
    Serial.println("=============================================\n\n");

    if ( EN_KNN == 1 )
      {
            printf("CM1K Running KNN Mode.....\n");

      }else {
            printf("CM1K Running RBF Mode.....\n");

      }
      
    cognimemtest.SimpleReadWrite();
    cognimemtest.SimpleLearnRecognize();
    
    printf("---------- Starting 'Ricardo Learn And Recognize'  ----------\n");
    
    delay(100);
    cm1k.reset();
    delay(100);
    cognimem.clear();
    delay(100);
    
    RicardoTrain();
    RicardoTest();
    delay(1000 * 60 * 30); // wait for 30 min
}


////////////////////////////////////////////////////////////////////////////////////
//Train the cognimem chip model
int RicardoTrain()
{
  Serial.println("Loading Training Data....");
  unsigned char MatrixFromSD[NumRowsGlobalTrain][NumColsGlobal];
   ////////////////////////////////////////////////////////////////////////////////////////////////////
  initialize_SD_card();
  myFile = SD.open(TrainFile);
  if (myFile) {
    Serial.println("the file was opened");
    
    int cell = 0x00; 
    int super_index = 0;
    int col_hex = 0;
    int row_hex = 0;
    int control = 0;
    int green_i = 0;

    while (myFile.available()) {
   
        cell = myFile.parseInt();
        //Serial.println(cell);
        row_hex = round(super_index/NumColsGlobal);   
        col_hex = (super_index - (row_hex*NumColsGlobal));  
       
        if (row_hex > NumRowsGlobalTrain)
        {
          break;
        }
        
        MatrixFromSD[row_hex][col_hex] = cell;
        super_index = super_index + 1;
        
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening the file ricardo");
  }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   //to view data
   /*
  for (int i =0; i<NumRowsGlobalTrain; i++)
    {
      Serial.print(i);
      Serial.print(" = Vector  : [ ");
      for (int j=0; j<NumColsGlobal; j++)
      {
        
        Serial.print(MatrixFromSD[i][j]);
        Serial.print(",");
      }
      Serial.println("]");
    }
    */
  ////////////////////////////////////////////////////////////////////////////////////////////////////
    
    Serial.println("Starting Training ");

    int result = 0;
    printf("Total Neuron Count: %u\n", result);
    cognimem.clear();
        
    int num_samples = NumRowsGlobalTrain;
    int num_cols = FeaturesGlobal;
    int the_class;

    unsigned char current_vector[num_cols];
    for (int i =0; i<num_cols; i++){ current_vector[i] = 0;}
    if ( EN_KNN == 1 ){cm1k.write(CM_REG_NETWORK_STATUS, 16);}


    startTime = millis();
    
    for (int i =0; i<num_samples; i++)
    {
      for (int j=0; j<num_cols; j++)
      {
        current_vector[j] = MatrixFromSD[i][j];
      }
      the_class = MatrixFromSD[i][num_cols];
      cognimem.learn(current_vector, sizeof(current_vector), the_class);
      
    }
    
    if ( EN_KNN == 1 ){cm1k.write(CM_REG_NETWORK_STATUS, 0);}
    elapsedTime = millis() - startTime;

    result = cognimem.getCommittedNeuronCount();
    printf("Committed Neuron Count: %u\n\n", result);

    //printf("Observe the contents of the existing knowledgebase (KB)...\n");
    //cognimem.displayNeurons(sizeof(current_vector));	
    
    Serial.print("Finished Training.. Elasped Time : ");
    Serial.print(elapsedTime);
    Serial.println("  milli Seconds");
    return 0;
}


void initialize_SD_card()
{
 
  
  pinMode(10, OUTPUT);
   
  if (!SD.begin(4)) {
    Serial.println("SD initialization failed!");
    return;
  }
   
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Test the input sample given the cognimem model on the chip
int RicardoTest()
{
   Serial.println("Loading Testing Data....");
  unsigned char MatrixFromSD2[NumRowsGlobalTest][NumColsGlobal];
   ////////////////////////////////////////////////////////////////////////////////////////////////////
  initialize_SD_card();
  myFile = SD.open(TestFile);
  if (myFile) {
    Serial.println("the file was opened");
    
    int cell = 0x00; 
    int super_index = 0;
    int col_hex = 0;
    int row_hex = 0;
    int control = 0;
    int green_i = 0;
    while (myFile.available()) {
   
        cell = myFile.parseInt();
        //Serial.println(cell);
        row_hex = round(super_index/NumColsGlobal);   
        col_hex = (super_index - (row_hex*NumColsGlobal));  
       
        if (row_hex > NumRowsGlobalTest)
        {
          break;
        }
        
        MatrixFromSD2[row_hex][col_hex] = cell;
        super_index = super_index + 1;
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening the file ricardo");
  }
     ///////////////////////////////////////////////////////////////////////////////////////////////////
   //to view data
   /*
  for (int i =0; i<NumRowsGlobalTest; i++)
    {
      Serial.print(i);
      Serial.print(" = Vector  : [ ");
      for (int j=0; j<NumColsGlobal; j++)
      {
        
        Serial.print(MatrixFromSD2[i][j]);
        Serial.print(",");
      }
      Serial.println("]");
    }
    */
  ////////////////////////////////////////////////////////////////////////////////////////////////////

  Serial.println("Starting Testing "); 
  String PrintString;
  int result = 0;
    
    unsigned short dsts[] = { 0x0000, 0x0000 };  // distances
    unsigned short cats[] = { 0x0000, 0x0000 };  // categories
    unsigned short nids[] = { 0x0000, 0x0000 };  // neuron ids
   ////////////////////////////////////////////////////////////////////////////////////////////////////

    int num_samples = NumRowsGlobalTest;
    int num_cols = FeaturesGlobal;
    int the_class;

    unsigned char current_vector[num_cols];
    for (int i =0; i<num_cols; i++){ current_vector[i] = 0;}
    
    startTime = millis();
    for (int n =0; n<num_samples; n++)
    {
      for (int j=0; j<num_cols; j++)
      {
        current_vector[j] = MatrixFromSD2[n][j];
      }
      
      the_class = MatrixFromSD2[n][num_cols];
      
      if ( EN_KNN == 1 ){ cm1k.write(CM_REG_NETWORK_STATUS, 32);}

      result = cognimem.recognize(current_vector, sizeof(current_vector));
      
      for (int i = 0; i < k; i++)
      {
            dsts[i] = cm1k.read(CM_REG_DISTANCE,R_DST);
            cats[i] = cm1k.read(CM_REG_CATEGORY,R_CAT);
            nids[i] = cm1k.read(CM_REG_NEURON_ID,R_NID);
         //printf("Result %02u: ; Dist: %02u; Cat: %02u; Nid: %02u\n", (n + 1), dsts[i], cats[i], nids[i]);
      }
    }

    elapsedTime = millis() - startTime;
    Serial.print("Finished Testing.. Elasped Time : ");
    Serial.print(elapsedTime);
    Serial.println("   milli Seconds");
    
    return 0;
  
}


