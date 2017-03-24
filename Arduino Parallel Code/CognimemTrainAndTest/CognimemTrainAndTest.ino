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

// Enable KNN: 1-Yes, 0-No(RCE).
int EN_KNN = 1;

// Train and Test Files from microSD card
// file name should be UPPERCASE and lenght should be 8 character or fewer string, and 3 character extension

char TrainFile[15] = "TR4096.TXT";
char TestFile[15] = "TR4096.TXT";
    
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

#define MODE_KNN                       16
#define MODE_RBF                       0



// DARPA data 
File myFile;
int NumColsGlobal=41; //41 features + 1 class
int FeaturesGlobal=40;

// Initialize SD Card...
void initialize_SD_card()
{
  pinMode(10, OUTPUT);
  if (!SD.begin(4)) {
    Serial.println("SD initialization failed!");
    return;
  }
}


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
    
    Serial.println("\n\n============================================");
    Serial.println("  CM1K parallel Communication : Tic Toc Clock");
    Serial.println("=============================================\n");
    if ( EN_KNN == 1 )
      {
            printf("|           CM1K Running KNN Mode           |\n");

      }else {
            printf("|           CM1K Running RBF Mode           |\n");

      }
    Serial.println("=============================================\n\n");      
    printf("*************Verifying Cognimem Hardware**************\n\n");
    // Using CognimemTest Library
  
    // simple read and write to MINIF and MAXIF registers
    cognimemtest.SimpleReadWrite();
    
    // Simple Learn and Recognize 
    cognimemtest.SimpleLearnRecognize();
    
    printf("=========================================\n");
    printf("|         VERIFICATION COMPLETED        |\n");
    printf("=========================================\n\n");
    
    // Wait 10 seconds....
    delay(1000 * 10); 
    printf("**********Starting Learn And Recognize*************\n\n");
    
    delay(100);
    
    //Reset CM1K 
    cm1k.reset();
    delay(100);
    
    //Clear all neurons MINIF, MAXIF
    cognimem.clear();
    delay(100);
    
    // Train Cognimem Hardware
    CognimemTrain();
    
    // Wait 5 seconds....
    delay(1000 * 5); 
    
    // Train Cognimem Hardware
    CognimemTest();
    
    
    printf("=====================================================\n");
    printf("|           LEARN AND RECOGNIZE COMPLETED           |\n");
    printf("=====================================================\n\n");
    // Pause for 15 min
    delay(1000 * 60 * 15); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Train the cognimem chip model
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CognimemTrain()
{
  Serial.println("Starting Training ");
  
  // Create Array of 41 to read from microSD card
  unsigned char MatrixFromSD[NumColsGlobal];
  unsigned char current_vector[NumColsGlobal-1];
  int cell = 0;
  int result;
   
  initialize_SD_card();
  
  //Open Training file 
  myFile = SD.open(TrainFile);
  int my_count = 0;
  if (myFile) {
    Serial.println("the file was opened");
    
    //printf("Total Neuron Count: %u\n", result);
    cognimem.clear();
    int num_cols = FeaturesGlobal;
    int the_class;
    int aurora_count = 0;
    
    // If KNN is selected write NSR Register value 16 to enable KNN Learning... 
    if ( EN_KNN == 1 ){cm1k.write(CM_REG_NETWORK_STATUS, 16);}
    
    // Start Clock Start 
    startTime = millis();
    while (myFile.available()) {
        cell = myFile.parseInt();
        MatrixFromSD[aurora_count] = cell;
        aurora_count = aurora_count + 1;
        if (aurora_count > num_cols)
        {
          aurora_count = 0;
          for (int j=0; j<num_cols; j++)
          {
              current_vector[j] = MatrixFromSD[j];
              //Serial.println(MatrixFromSD[j]);   
          }
          the_class = MatrixFromSD[num_cols];
          cognimem.learn(current_vector, sizeof(current_vector), the_class);
          
          my_count = my_count + 1;
          Serial.println(my_count);
        }
    }
    elapsedTime = millis() - startTime;

    if ( EN_KNN == 1 ){cm1k.write(CM_REG_NETWORK_STATUS, 0);}
    
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
        Serial.println("error opening the file ricardo");
  }
  
    // Get the Number of commited Neuron Count.
    result = cognimem.getCommittedNeuronCount();
    printf("Committed Neuron Count: %u\n\n", result);
    
    // dont use below for large number of commited Neurons. Arduino Memory Limitations...
    //printf("Observe the contents of the existing knowledgebase (KB)...\n");
    //cognimem.displayNeurons(sizeof(current_vector));	
    
    Serial.print("\nFinished Training.. Elasped Time : ");
    Serial.print(elapsedTime);
    Serial.println("  milli Seconds\n");
    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Test the input sample given the cognimem model on the chip
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CognimemTest()
{
  Serial.println("Starting Testing ");
  
  String PrintString;

  unsigned char MatrixFromSD[NumColsGlobal];
  unsigned char current_vector[NumColsGlobal-1];
  int cell = 0;
  int result = 0;
    
  unsigned short dsts[] = { 0x0000, 0x0000 };  // distances
  unsigned short cats[] = { 0x0000, 0x0000 };  // categories
  unsigned short nids[] = { 0x0000, 0x0000 };  // neuron ids

  initialize_SD_card();
  
  //Open Training file 
  myFile = SD.open(TestFile);
  int num_cols = FeaturesGlobal;
  int the_class;
  int sample_n = 0;
    
  int aurora_count =0;
  if (myFile) {
    Serial.println("The file was opened");
    printf("Sample, Result, actual class, Dist, Cat, Nid\n");
    
    startTime = millis();
    while (myFile.available()) {

        cell = myFile.parseInt();
        MatrixFromSD[aurora_count] = cell;
        aurora_count = aurora_count + 1;

        if (aurora_count > num_cols)
        {
          aurora_count = 0;
          for (int j=0; j<num_cols; j++)
          {
              current_vector[j] = MatrixFromSD[j];
          }
      
          the_class = MatrixFromSD[num_cols];
          
          // If KNN is selected write NSR Register value 32 to enable KNN Recognition... 
          if ( EN_KNN == 1 ){ cm1k.write(CM_REG_NETWORK_STATUS, 32);}
          
          // Recognize current_vector
          result = cognimem.recognize(current_vector, sizeof(current_vector));
     
          sample_n = sample_n + 1;
          
          // for each recognised vector retrive the distance, category and neuron id ( k >=1 )
          for (int i = 0; i < k; i++)
          //while(dist!=65535)
          {
            dsts[i] = cm1k.read(CM_REG_DISTANCE,R_DST);
            cats[i] = cm1k.read(CM_REG_CATEGORY,R_CAT);
            nids[i] = cm1k.read(CM_REG_NEURON_ID,R_NID);

            PrintString = String(sample_n,DEC) + "," + String(result,DEC) + "," + String(the_class,DEC) + "," + String(dsts[i],DEC) + "," + String(cats[i],DEC) + "," + String(nids[i],DEC);
            //printf("    Sample, %02u ,Result, %02u ,actual class, %02u , Dist, %02u, Cat, %02u, Nid, %02u\n", sample_n, result ,the_class, dsts[i], cats[i], nids[i]);
            Serial.println(PrintString);
        
          }//end for loop
        }//end if  
    }//end while
    
    // close the file:
    elapsedTime = millis() - startTime;
    myFile.close();
    
  } else {
  	// if the file didn't open, print an error:
        Serial.println("error opening the file ricardo");
  }
    Serial.print("\nFinished Testing.. Elasped Time : ");
    Serial.print(elapsedTime);
    Serial.println("   milli Seconds\n");
    return 0;
}


