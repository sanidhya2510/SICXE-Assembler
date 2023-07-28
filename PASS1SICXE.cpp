#include "required.cpp"  //file containing all the required tables and functions

using namespace std;

string inputFile, *BLocksNumToName, firstExecutable_Sec;
bool error_flag=false;
int program_length;

void LTORGsupport(string& litPrefix, int& lineNumberDelta,int lineNumber,int& LOCCTR, int& lastDeltaLOCCTR, int currentBlockNumber){
  string litAddress,litValue;
  litPrefix = "";
  for(auto const& it: LITTAB){
    litAddress = it.second.address;
    litValue = it.second.value;
    if(litAddress!="?"){
    }
    else{
      lineNumber += 5;
      lineNumberDelta += 5;
      LITTAB[it.first].address = ConvertIntToHexadecimalString(LOCCTR);
      LITTAB[it.first].blockNumber = currentBlockNumber;
      litPrefix += "\n" + to_string(lineNumber) + "\t" + ConvertIntToHexadecimalString(LOCCTR) + "\t" + to_string(currentBlockNumber) + "\t" + "*" + "\t" + "="+litValue + "\t" + " " + "\t" + " ";

      if(litValue[0]=='X'){
        LOCCTR += (litValue.length() -3)/2;
        lastDeltaLOCCTR += (litValue.length() -3)/2;
      }
      else if(litValue[0]=='C'){
        LOCCTR += litValue.length() -3;
        lastDeltaLOCCTR += litValue.length() -3;
      }
    }
  }
}

void evaluateExpression(string expression, bool& relative,string& tempOperand,int lineNumber, ofstream& passOneErrorFile,bool& error_flag){
  string singleOperand="?",singleOperator="?",valueString="",valueTemp="",writeData="";
  int lastOperand=0,lastOperator=0,pairCount=0;
  char lastByte = ' ';
  bool Illegal = false;

  for(int i=0;i<expression.length();){
    singleOperand = "";

    lastByte = expression[i];
    while((lastByte!='+' && lastByte!='-' && lastByte!='/' && lastByte!='*') && i<expression.length()){
      singleOperand += lastByte;
      lastByte = expression[++i];
    }

    if(SYMTAB[singleOperand].ifexists==1){
      lastOperand = SYMTAB[singleOperand].relative;
      valueTemp = to_string(hexadecimalStringToInt(SYMTAB[singleOperand].address));
    }
    else if((singleOperand != "" || singleOperand !="?" ) && checkIfAllDigits(singleOperand)){
      lastOperand = 0;
      valueTemp = singleOperand;
    }
    else{
      writeData = "Line: "+to_string(lineNumber)+" : Can't find symbol. Found "+singleOperand;
      writeToFile(passOneErrorFile,writeData);
      Illegal = true;
      break;
    }

    if(lastOperand*lastOperator == 1){
      writeData = "Line: "+to_string(lineNumber)+" : Illegal expression";
      writeToFile(passOneErrorFile,writeData);
      error_flag = true;
      Illegal = true;
      break;
    }
    else if((singleOperator=="-" || singleOperator=="+" || singleOperator=="?")&&lastOperand==1){
      if(singleOperator=="-"){
        pairCount--;
      }
      else{
        pairCount++;
      }
    }
    valueString += valueTemp;

    singleOperator= "";
    while(i<expression.length()&&(lastByte=='+'||lastByte=='-'||lastByte=='/'||lastByte=='*')){
      singleOperator += lastByte;
      lastByte = expression[++i];
    }

    if(singleOperator.length()>1){
      writeData = "Line: "+to_string(lineNumber)+" : Illegal operator in expression. Found "+singleOperator;
      writeToFile(passOneErrorFile,writeData);
      error_flag = true;
      Illegal = true;
      break;
    }

    if(singleOperator=="*" || singleOperator == "/"){
      lastOperator = 1;
    }
    else{
      lastOperator = 0;
    }

    valueString += singleOperator;
  }

  if(!Illegal){
    if(pairCount==1){
      relative = 1;
      EvaluateString tempOBJ(valueString);
      tempOperand = ConvertIntToHexadecimalString(tempOBJ.getResult());
    }
    else if(pairCount==0){
      relative = 0;
      cout<<valueString<<endl;
      EvaluateString tempOBJ(valueString);
      tempOperand = ConvertIntToHexadecimalString(tempOBJ.getResult());
    }
    else{
      writeData = "Line: "+to_string(lineNumber)+" : Illegal expression";
      writeToFile(passOneErrorFile,writeData);
      error_flag = true;
      tempOperand = "00000";
      relative = 0;
    }
  }
  else{
    tempOperand = "00000";
    error_flag = true;
    relative = 0;
  }
}
void pass1(){
  ifstream sourceFile;
  ofstream intermediateFile, passOneErrorFile;
  //intermediateFile - after pass 1, intermediate file will contain the line number and location of each instruction
  //passOneErrorFile - after pass 1, the pass one error file will contains all the errors in the code after first pass.

  sourceFile.open(inputFile);
  if(!sourceFile){
    cout<<"Unable to open file: "<<inputFile<<endl;
    exit(1);
  }

  intermediateFile.open("intermediate.txt");
  if(!intermediateFile){
    cout<<"Unable to open file: intermediate.txt"<<endl;
    exit(1);
  }
  writeToFile(intermediateFile,"Line\tAddress\tLabel\tOPCODE\tOPERAND\tComment");
  passOneErrorFile.open("pass_one_errors.txt");
  if(!passOneErrorFile){
    cout<<"Unable to open file: pass_one_errors.txt"<<endl;
    exit(1);
  }
  writeToFile(passOneErrorFile,"###############PASS1###############");  

  string fileLine;
  string writeData,writeDataSuffix="",writeDataPrefix="";
  int index=0;

  string currentBlockName = "DEFAULT";
  int currentBlockNumber = 0;
  int totalBlocks = 1;

  bool statusCode;
  string label,opcode,operand,comment;
  string tempOperand;

  int startAddress,LOCCTR,saveLOCCTR,lineNumber,lastDeltaLOCCTR,lineNumberDelta=0;
  lineNumber = 0;
  lastDeltaLOCCTR = 0;

  getline(sourceFile,fileLine);
  lineNumber += 5;
  while(checkCommentLine(fileLine)){
    writeData = to_string(lineNumber) + "\t" + fileLine;
    writeToFile(intermediateFile,writeData);
    getline(sourceFile,fileLine);
    lineNumber += 5;
    index = 0;
  }

  readFirstNonWhiteSpace(fileLine,index,statusCode,label);
  readFirstNonWhiteSpace(fileLine,index,statusCode,opcode);

  if(opcode=="START"){
    readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
    readFirstNonWhiteSpace(fileLine,index,statusCode,comment,true);
    startAddress = hexadecimalStringToInt(operand);
    LOCCTR = startAddress;
    writeData = to_string(lineNumber) + "\t" + ConvertIntToHexadecimalString(LOCCTR-lastDeltaLOCCTR) + "\t" + to_string(currentBlockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment;
    writeToFile(intermediateFile,writeData); 

    getline(sourceFile,fileLine); 
    lineNumber += 5;
    index = 0;
    readFirstNonWhiteSpace(fileLine,index,statusCode,label); 
    readFirstNonWhiteSpace(fileLine,index,statusCode,opcode);
  }
  else{
    startAddress = 0;
    LOCCTR = 0;
  }
  while(opcode!="END"){
    if(!checkCommentLine(fileLine)){
      if(label!=""){
        if(SYMTAB[label].ifexists==0){
          SYMTAB[label].name = label;
          SYMTAB[label].address = ConvertIntToHexadecimalString(LOCCTR);
          SYMTAB[label].relative = 1;
          SYMTAB[label].ifexists = 1;
          SYMTAB[label].blockNumber = currentBlockNumber;
        }
        else{
          writeData = "Line: "+to_string(lineNumber)+" : Duplicate symbol for '"+label+"'. Previously defined at "+SYMTAB[label].address;
          writeToFile(passOneErrorFile,writeData);
          error_flag = true;
        }
      }
      if(OPTAB[getActualOpcode(opcode)].isValid == 1){
        if(OPTAB[getActualOpcode(opcode)].instructionFormat==3){
          LOCCTR += 3;
          lastDeltaLOCCTR += 3;
          if(getFlagFormat(opcode)=='+'){
            LOCCTR += 1;
            lastDeltaLOCCTR += 1;
          }
          if(getActualOpcode(opcode)=="RSUB"){
            operand = " ";
          }
          else{
            readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
            if(operand[operand.length()-1] == ','){
              readFirstNonWhiteSpace(fileLine,index,statusCode,tempOperand);
              operand += tempOperand;
            }
          }

          if(getFlagFormat(operand)=='='){
            tempOperand = operand.substr(1,operand.length()-1);
            if(tempOperand=="*"){
              tempOperand = "X'" + ConvertIntToHexadecimalString(LOCCTR-lastDeltaLOCCTR,6) + "'";
            }
            if(LITTAB[tempOperand].ifexists==0){
              LITTAB[tempOperand].value = tempOperand;
              LITTAB[tempOperand].ifexists = 1;
              LITTAB[tempOperand].address = "?";
              LITTAB[tempOperand].blockNumber = -1;
            }
          }
        }
        else if(OPTAB[getActualOpcode(opcode)].instructionFormat==1){
          operand = " ";
          LOCCTR += OPTAB[getActualOpcode(opcode)].instructionFormat;
          lastDeltaLOCCTR += OPTAB[getActualOpcode(opcode)].instructionFormat;
        }
        else{
          LOCCTR += OPTAB[getActualOpcode(opcode)].instructionFormat;
          lastDeltaLOCCTR += OPTAB[getActualOpcode(opcode)].instructionFormat;
          readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
          if(operand[operand.length()-1] == ','){
            readFirstNonWhiteSpace(fileLine,index,statusCode,tempOperand);
            operand += tempOperand;
          }
        }
      }
      else if(opcode == "WORD"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        LOCCTR += 3;
        lastDeltaLOCCTR += 3;
      }
      else if(opcode == "RESW"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        LOCCTR += 3*stoi(operand);
        lastDeltaLOCCTR += 3*stoi(operand);
      }
      else if(opcode == "RESB"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        LOCCTR += stoi(operand);
        lastDeltaLOCCTR += stoi(operand);
      }
      else if(opcode == "BYTE"){
        readByteOperand(fileLine,index,statusCode,operand);
        if(operand[0]=='X'){
          LOCCTR += (operand.length() -3)/2;
          lastDeltaLOCCTR += (operand.length() -3)/2;
        }
        else if(operand[0]=='C'){
          LOCCTR += operand.length() -3;
          lastDeltaLOCCTR += operand.length() -3;
        }
      }
      else if(opcode=="BASE"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
      }
      else if(opcode=="LTORG"){
        operand = " ";
        LTORGsupport(writeDataSuffix,lineNumberDelta,lineNumber,LOCCTR,lastDeltaLOCCTR,currentBlockNumber);
      }
      else if(opcode=="ORG"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);

        char lastByte = operand[operand.length()-1];
        while(lastByte=='+'||lastByte=='-'||lastByte=='/'||lastByte=='*'){
          readFirstNonWhiteSpace(fileLine,index,statusCode,tempOperand);
          operand += tempOperand;
          lastByte = operand[operand.length()-1];
        }

        int tempVariable;
        tempVariable = saveLOCCTR;
        saveLOCCTR = LOCCTR;
        LOCCTR = tempVariable;

        if(SYMTAB[operand].ifexists==1){
          LOCCTR = hexadecimalStringToInt(SYMTAB[operand].address);
        }
        else{
          bool relative;
          error_flag = false;
          evaluateExpression(operand,relative,tempOperand,lineNumber,passOneErrorFile,error_flag);
          if(!error_flag){
            LOCCTR = hexadecimalStringToInt(tempOperand);
          }
          error_flag = false;
        }
      }
      else if(opcode=="USE"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        BLOCKS[currentBlockName].LOCCTR = ConvertIntToHexadecimalString(LOCCTR);
        if(operand=="")
        {
          operand = "DEFAULT";
        }
        if(BLOCKS[operand].ifexists==0){
          BLOCKS[operand].ifexists = 1;
          BLOCKS[operand].name = operand;
          BLOCKS[operand].number = totalBlocks++;
          BLOCKS[operand].LOCCTR = "0";
        }


        currentBlockNumber = BLOCKS[operand].number;
        currentBlockName = BLOCKS[operand].name;
        LOCCTR = hexadecimalStringToInt(BLOCKS[operand].LOCCTR);

      }
      else if(opcode=="EQU"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        tempOperand = "";
        bool relative;

        if(operand=="*"){
          tempOperand = ConvertIntToHexadecimalString(LOCCTR-lastDeltaLOCCTR,6);
          relative = 1;
        }
        else if(checkIfAllDigits(operand)){
          tempOperand = ConvertIntToHexadecimalString(stoi(operand),6);
          relative = 0;
        }
        else{
          char lastByte = operand[operand.length()-1];
        
          while(lastByte=='+'||lastByte=='-'||lastByte=='/'||lastByte=='*'){
            readFirstNonWhiteSpace(fileLine,index,statusCode,tempOperand);
            operand += tempOperand;
            lastByte = operand[operand.length()-1];
          }
         
          evaluateExpression(operand,relative,tempOperand,lineNumber,passOneErrorFile,error_flag);
        }

        SYMTAB[label].name = label;
        SYMTAB[label].address = tempOperand;
        SYMTAB[label].relative = relative;
        SYMTAB[label].blockNumber = currentBlockNumber;
        lastDeltaLOCCTR = LOCCTR - hexadecimalStringToInt(tempOperand);
      }
      else{
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        writeData = "Line: "+to_string(lineNumber)+" : Invalid OPCODE. Found " + opcode;
        writeToFile(passOneErrorFile,writeData);
        error_flag = true;
      }
      readFirstNonWhiteSpace(fileLine,index,statusCode,comment,true);
      if(opcode=="EQU" && SYMTAB[label].relative == 0){
        writeData = writeDataPrefix + to_string(lineNumber) + "\t" + ConvertIntToHexadecimalString(LOCCTR-lastDeltaLOCCTR) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
      } 
      else{
        writeData = writeDataPrefix + to_string(lineNumber) + "\t" + ConvertIntToHexadecimalString(LOCCTR-lastDeltaLOCCTR) + "\t" + to_string(currentBlockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;

      }
      writeDataPrefix = "";
      writeDataSuffix = "";
    }
    else{
      writeData = to_string(lineNumber) + "\t" + fileLine;
    }
    writeToFile(intermediateFile,writeData);

    BLOCKS[currentBlockName].LOCCTR = ConvertIntToHexadecimalString(LOCCTR);
    getline(sourceFile,fileLine); 
    lineNumber += 5 + lineNumberDelta;
    lineNumberDelta = 0;
    index = 0;
    lastDeltaLOCCTR = 0;
    readFirstNonWhiteSpace(fileLine,index,statusCode,label); 
    readFirstNonWhiteSpace(fileLine,index,statusCode,opcode);
}

if(opcode=="END"){
	firstExecutable_Sec=SYMTAB[label].address;
	SYMTAB[firstExecutable_Sec].name=label;
	SYMTAB[firstExecutable_Sec].address=firstExecutable_Sec;
}

  readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
  readFirstNonWhiteSpace(fileLine,index,statusCode,comment,true);

  
  currentBlockName = "DEFAULT";
  currentBlockNumber = 0;
  LOCCTR = hexadecimalStringToInt(BLOCKS[currentBlockName].LOCCTR);

  LTORGsupport(writeDataSuffix,lineNumberDelta,lineNumber,LOCCTR,lastDeltaLOCCTR,currentBlockNumber);

  writeData = to_string(lineNumber) + "\t" + ConvertIntToHexadecimalString(LOCCTR-lastDeltaLOCCTR) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
  writeToFile(intermediateFile,writeData);

  int LocctrArr[totalBlocks];
  BLocksNumToName = new string[totalBlocks];
  for(auto const& it: BLOCKS){
    LocctrArr[it.second.number] = hexadecimalStringToInt(it.second.LOCCTR);
    BLocksNumToName[it.second.number] = it.first;
  }
  for(int i = 1 ;i<totalBlocks;i++){
    LocctrArr[i] += LocctrArr[i-1];
  }
  for(auto const& it: BLOCKS){
    if(it.second.startAddress=="?"){
      BLOCKS[it.first].startAddress= ConvertIntToHexadecimalString(LocctrArr[it.second.number - 1]);
    }
  }
  program_length = LocctrArr[totalBlocks - 1] - startAddress;
  sourceFile.close();
  intermediateFile.close();
  passOneErrorFile.close();
}
