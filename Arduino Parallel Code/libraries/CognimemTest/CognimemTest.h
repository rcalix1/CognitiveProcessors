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

#ifndef COGNIMEMTEST_H
#define COGNIMEMTEST_H

class CognimemTestClass {
public:
  CognimemTestClass() { };
  
  // Simple Read Write Test
  void SimpleReadWrite();
  
  // Simple Learn and Recognize Test 
  void SimpleLearnRecognize(); 

private:

  int testSimpleReadWrite();
  int testSimpleLearnRecognize(); 
	
};

extern CognimemTestClass cognimemtest;

#endif
