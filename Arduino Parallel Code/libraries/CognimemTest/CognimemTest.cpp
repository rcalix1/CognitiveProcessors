/*
 ****************************************************************************************
 *     
 *     COMPANY: Purdue University Calumet Department of Computer Information Technology 
 *
 * DESCRIPTION: Test CM1K Parallel Interfacing Configuration with Arduino-Due Micro controller 
 *
 * 	   VERSION: 1.0 (05/7/2015) 
 *
 ***************************************************************************************
 */
 
#include "CM1KParallel.h"
#include "CognimemParallel.h"
#include "CognimemTest.h"

int result = 0;


// Perform sample learn and recognize functions on network to view/confirm behavior
int CognimemTestClass::testSimpleLearnRecognize(){
    result = 0;
    // we are retrieving a "k" of two (asking for the first two neurons that fired)
    int k = 1;
    unsigned short dsts[] = { 0x0000, 0x0000 };  // distances
    unsigned short cats[] = { 0x0000, 0x0000 };  // categories
    unsigned short nids[] = { 0x0000, 0x0000 };  // neuron ids

    unsigned char vector1[] = { 0x0B, 0x0B, 0x0B, 0x0B, };
    unsigned char vector2[] = { 0x0F, 0x0F, 0x0F, 0x0F, };
    unsigned char vector3[] = { 0x14, 0x14, 0x14, 0x14, };  
    unsigned char vector4[] = { 0x0C, 0x0C, 0x0C, 0x0C };
    unsigned char vector5[] = { 0x0E, 0x0E, 0x0E, 0x0E };
    unsigned char vector6[] = { 0x0D, 0x0D, 0x0D, 0x0D };
    unsigned char vector7[] = { 0x1E, 0x1E, 0x1E, 0x1E };
    unsigned char vector8[] = { 0x0D, 0x0D, 0x0D, 0x0D };
    unsigned char vector9[] = { 0x0C, 0x0C, 0x0C, 0x0C };
    unsigned char vectorRC[] = { 0x16, 0x16, 0x16, 0x16, };


    printf("Total Neuron Count: %u\n", result);
    cognimem.clear();

    printf("Learning vector { 0x0B, 0x0B, 0x0B, 0x0B } as Category 55...\n"); // 11,11,11,11
    cognimem.learn(vector1, sizeof(vector1), 55);
    
	printf("Learning vector { 0x0F, 0x0F, 0x0F, 0x0F } as Category 33...\n"); // 15,15,15,15
    cognimem.learn(vector2, sizeof(vector2), 33);
    
	printf("Learning vector { 0x14, 0x14, 0x14, 0x14 } as Category 100...\n");// 20,20,20,20
    cognimem.learn(vector3, sizeof(vector3), 100);
    
    result = cognimem.getCommittedNeuronCount();
    printf("Committed Neuron Count: %u\n\n", result);
    if (result != 3) return -1;
    printf("Observe the contents of the existing knowledgebase (KB)...\n");
    cognimem.displayNeurons(sizeof(vector8));		

    //recognition after training
    printf("\nRecognize  vector { 0x0B, 0x0B, 0x0B, 0x0B }...\n"); // 11,11,11,11
    result = cognimem.recognize(vector1, sizeof(vector1));
    
    printf("Network Status: %02X\n", result);
    for (int i = 0; i < k; i++)
    {
        dsts[i] = cm1k.read(CM_REG_DISTANCE,R_DST);
        cats[i] = cm1k.read(CM_REG_CATEGORY,R_CAT);
        nids[i] = cm1k.read(CM_REG_NEURON_ID,R_NID);
        printf("Result %02u: ; Dist: %02u; Cat: %02u; Nid: %02u\n", (i + 1), dsts[i], cats[i], nids[i]);
    }
    //if ((result & 0x0004) != 4 || dsts[0] != 4 || dsts[1] != 12 || cats[0] != 55 || cats[1] != 33 || nids[0] != 1 || nids[1] != 2) return -1;
    
    
    printf("\nRecognize evector { 0x0F, 0x0F, 0x0F, 0x0F }...\n"); // 15,15,15,15
    result = cognimem.recognize(vector2, sizeof(vector2));
    printf("Network Status: %02X\n", result);


    for (int i = 0; i < k; i++)
    {
        dsts[i] = cm1k.read(CM_REG_DISTANCE,R_DST);
        cats[i] = cm1k.read(CM_REG_CATEGORY,R_CAT);
        nids[i] = cm1k.read(CM_REG_NEURON_ID,R_NID);
        printf("Result %02u: ; Dist: %02u; Cat: %02u; Nid: %02u\n", (i + 1), dsts[i], cats[i], nids[i]);
    }
    //if ((result & 0x0004) != 4 || dsts[0] != 4 || dsts[1] != 12 || cats[0] != 33 || cats[1] != 55 || nids[0] != 2 || nids[1] != 1) return -1;
    
    
    printf("\nRecognize non equi-distant vector { 0x16, 0x16, 0x16, 0x16 }...\n"); // 22,22,22,22
    result = cognimem.recognize(vectorRC, sizeof(vectorRC));
    printf("Network Status: %02X\n", result);
    for (int i = 0; i < k; i++)
    {
        dsts[i] = cm1k.read(CM_REG_DISTANCE,R_DST);
        cats[i] = cm1k.read(CM_REG_CATEGORY,R_CAT);
        nids[i] = cm1k.read(CM_REG_NEURON_ID,R_NID);
        printf("Result %02u: ; Dist: %02u; Cat: %02u; Nid: %02u\n", (i + 1), dsts[i], cats[i], nids[i]);
    }
    //if ((result & 0x0004) != 4 || dsts[0] != 4 || dsts[1] != 12 || cats[0] != 33 || cats[1] != 55 || nids[0] != 2 || nids[1] != 1) return -1;
	

	return 0;
}

int CognimemTestClass::testSimpleReadWrite(){
	
    result = 0;
    
    // get the default MINIF
    result = cm1k.read(CM_REG_MINIMUM_INF);  
    printf("minif: %02X\n", result, 2);
    //if (result != 2) return -1;
    
    // update the MINIF (increment the default +1)
    cm1k.write(CM_REG_MINIMUM_INF, (result + 1));  
    
    // re-read the MINIF to confirm it was updated correctly
    result = cm1k.read(CM_REG_MINIMUM_INF);  
    printf("minif: %02X\n", result, 2);
    //if (result != 3) return -1;
    
    // get the default MAXIF
    result = cm1k.read(CM_REG_MAXIMUM_INF);  
    printf("maxif: %02X\n", result, 2);
    //if (result != 0x4000) return -1;
    
    // update the MAXIF (increment the default +1)
    cm1k.write(CM_REG_MAXIMUM_INF, (result + 1));  
    
    // re-read the MAXIF to confirm it was updated correctly
    result = cm1k.read(CM_REG_MAXIMUM_INF);  
    printf("maxif: %02X\n", result, 2);
    //if (result != 0x4001) return -1;
	    
    return 0;
}

void CognimemTestClass::SimpleReadWrite(){
    printf("---------- Starting 'Single Read and Write' Test ---------------\n");
    result = testSimpleReadWrite();
    printf("----------------------------------------------------------------");
    printf(" Test Results: %s\n\n", (result == 0 ? "PASSED" : " FAILED")); 
}

void CognimemTestClass::SimpleLearnRecognize(){
    printf("---------- Starting 'Simple Learn And Recognize' Test ----------\n");
    result = testSimpleLearnRecognize();
    printf("----------------------------------------------------------------");
    printf(" Test Results: %s\n\n", (result == 0 ? "PASSED" : " FAILED")); 
}

// Create our object
CognimemTestClass cognimemtest = CognimemTestClass();