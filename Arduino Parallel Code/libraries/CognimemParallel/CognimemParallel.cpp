/*
 ****************************************************************************************
 *     
 *     COMPANY: Purdue University Calumet Department of Computer Information Technology 
 *
 * DESCRIPTION: Cognimem Hardware Interfacing with Arduino CM1K Parallel Library  
 *
 * 	   VERSION: 1.0 (05/7/2015) 
 *
 ***************************************************************************************
 */
 
#include "CognimemParallel.h"
#include "CM1KParallel.h"

uint32_t dwMask;

// Reset the chip to its defaults, including clearing all committed neurons, and resetting the global MINIF and MAXIF
void CognimemClass::clear(void){
    cm1k.write(CM_REG_FORGET, 0x0000);	
}

// Broadcast the passed in vector pattern to the current neuron context
int CognimemClass::broadcast(unsigned char vector[], int length){
    
    // write all but the last byte to the COMP register    
    for (int i = 0; i < length - 1; i++) { 
        cm1k.write(CM_REG_COMPONENT, vector[i],W_COMP);
    }
	
    // now write the last byte to the LCOMP register (this triggers the search-and-sort function in the chip)
    cm1k.write(CM_REG_LAST_COMPONENT, vector[length - 1],W_LCOMP); 
    return 0;
}

// Learn a vector using the current context value
int CognimemClass::learn(unsigned char vector[], int length, int category){
    // first broadcast the vector to the network
    broadcast(vector, length);

    // now write to the CAT register, this will trigger a learn if the model is not represented in the network
    cm1k.write(CM_REG_CATEGORY, category);

    return 0;
}

// Learn a vector, in a specific context
int CognimemClass::learn(unsigned char vector[], int length, int category, int context){
    // first switch to the desired network context
    cm1k.write(CM_REG_GLOBAL_CONTEXT, context);
    
    // second broadcast the vector to the network
    broadcast(vector, length);

    // now write to the CAT register, this will trigger a learn if the model is not represented in the network
    cm1k.write(CM_REG_CATEGORY, category);
	
	return 0;
}

// Recognize a vector, assume the current context
int CognimemClass::recognize(unsigned char vector[], int length){
	// first broadcast the vector to the network
    broadcast(vector, length);

    // finally, check the network status register, to see if it was IDENTIFIED, UNCERTAIN, or UNKNOWN
    return cm1k.read(CM_REG_NETWORK_STATUS,R_NSR);
}

// Recognize a vector, in a specific context
int CognimemClass::recognize(unsigned char vector[], int length, int context){
    // first switch to the desired network context
    cm1k.write(CM_REG_GLOBAL_CONTEXT, context);
    
    // second broadcast the vector to the network
    broadcast(vector, length);

    // finally, check the network status register, to see if it was IDENTIFIED, UNCERTAIN, or UNKNOWN
    return cm1k.read(CM_REG_NETWORK_STATUS,R_NSR);
}

// Retrieve the number of neurons that are being used from the CM1K
int CognimemClass::getCommittedNeuronCount(){
    return cm1k.read(CM_REG_NEURON_COUNT);
}

// Read the content of the currently pointed at neuron and store in a 264 unsigned char array
// It is assumed that the network is already in SR mode and positioned on the proper neuron
void CognimemClass::readNeuron(unsigned char neuron[]){
	int temp = cm1k.read(CM_REG_NEURON_CONTEXT); 

	neuron[0] = 0;
	neuron[1] = (unsigned char)(temp & 0x00FF);

	for (int i = 0; i < 265; i++) 
	{
		neuron[i + 2] =cm1k.read(CM_REG_COMPONENT) & 0x00FF;
	}

	temp = cm1k.read(CM_REG_ACTIVE_INF);
	neuron[258]=(unsigned char)((temp & 0xFF00) >> 8);
	neuron[259]=(unsigned char)(temp & 0x00FF);
	
	temp = cm1k.read(CM_REG_MINIMUM_INF);
	neuron[260]=(unsigned char)((temp & 0xFF00) >> 8);
	neuron[261]=(unsigned char)(temp & 0x00FF);

	temp = cm1k.read(CM_REG_CATEGORY);
	neuron[262]=(unsigned char)((temp & 0xFF00) >> 8);
	neuron[263]=(unsigned char)(temp & 0x00FF);
}

// Change the Network Mode to Save-and-Restore and read out the contents of the network (committed neurons)
int CognimemClass::readNeurons(unsigned char neurons[], int ncount){
	int category = 0x00;
	int result = 0;
	int p = 0;
	int tempNsr = cm1k.read(CM_REG_NETWORK_STATUS,R_NSR);
	

	cm1k.write(CM_REG_NETWORK_STATUS, 0x0010);
	cm1k.write(CM_REG_RESET_CHAIN, 0x00);
	
	
	unsigned char neuron[CM_NEURON_BYTE_LENGTH];
	
	do
	{
		readNeuron(neuron);

		category = ((neuron[262] << 8) + neuron[263]);

		if  (category == 0 || category == 0xFFFF)
		{
			break;
		}			
		else
		{
			ncount++;

			for (int i = 0; i < CM_NEURON_BYTE_LENGTH; i++)
			{
				neurons[p++] = neuron[i];
			}
		}
	}
	while (category != 0 && category !=0xFFFF);
	
	cm1k.write(CM_REG_NETWORK_STATUS, tempNsr);

	return (result);
}

// Write the content of the RTL neuron and store in a 264 unsigned char array
// It is assumed that the network is already in SR mode and positioned on the proper neuron
void CognimemClass::writeNeuron(unsigned char neuron[]){
	cm1k.write(CM_REG_GLOBAL_CONTEXT, neuron[1]);

	for (int i = 0; i < CM_MAX_VECTOR_LENGTH; i++) { 
		cm1k.write(CM_REG_COMPONENT, neuron[2 + i]);
	}

	cm1k.write(CM_REG_ACTIVE_INF,  (neuron[258] << 8) + neuron[259]);
	cm1k.write(CM_REG_MINIMUM_INF, (neuron[260] << 8) + neuron[261]);
	cm1k.write(CM_REG_CATEGORY,    (neuron[262] << 8) + neuron[263]);
}

// Change the Network Mode to Save-and-Restore and restore the contents of the network (committed neurons)
// It is assumed that the neurons array is initialized to the correct length (ncount * 264)
int CognimemClass::writeNeurons(unsigned char neurons[], int ncount){
	unsigned char neuron[CM_NEURON_BYTE_LENGTH];

	int tempNsr = cm1k.read(CM_REG_NETWORK_STATUS);
	int tempGcr = cm1k.read(CM_REG_GLOBAL_CONTEXT);

 
	cm1k.write(CM_REG_FORGET, 0x00);
	cm1k.write(CM_REG_NETWORK_STATUS, 0x0010);
	cm1k.write(CM_REG_RESET_CHAIN, 0x00);

	for (int i = 0; i < ncount; i++)
	{
		memcpy(neuron, neurons + (i * CM_NEURON_BYTE_LENGTH), CM_NEURON_BYTE_LENGTH);
		
		writeNeuron(neuron);
	}

	//to update the nselect of the newly loaded neurons
	cm1k.write(CM_REG_NETWORK_STATUS, tempNsr);
	cm1k.write(CM_REG_GLOBAL_CONTEXT, tempGcr); 
	
	return (getCommittedNeuronCount());
}

// Retrieve the contents of the network and write them to the screen
// We are asking for the vectorLength so that we only display the interesting memory bytes 
void CognimemClass::displayNeurons(int vectorLength){
	int ncount = getCommittedNeuronCount();
	unsigned char *neurons = (unsigned char*)malloc(ncount * CM_NEURON_BYTE_LENGTH);
	int i = 0;
	int i2 = 0;


	readNeurons(neurons, ncount);
	
	printf("\n------------------- Begin Network Details -------------------");
	printf("\nCommitted Neuron Count: %u", ncount);

	for (i = 0; i < ncount; i++)
	{
		printf("\n\n NCR: %02X", neurons[(i * 264) + 1]);
		printf("  \n NID: %02X", i); 
		printf("\nCOMP:");
		
		for (i2 = 0; i2 < vectorLength; i2++) 
		{
			printf(" %02X", neurons[(i * 264) + 2 + i2]);
		}

		printf("\n AIF: %u", (neurons[(i * 264) + 258] << 8) + neurons[(i * 264) + 259]);
		printf("\n MIF: %u", (neurons[(i * 264) + 260] << 8) + neurons[(i * 264) + 261]); 
		printf("\n CAT: %u", (neurons[(i * 264) + 262] << 8) + neurons[(i * 264) + 263]);
	}
	
	printf("\n------------------- End Network Details -------------------\n\n");

	delete(neurons);
}

// Create our object
CognimemClass cognimem = CognimemClass();
