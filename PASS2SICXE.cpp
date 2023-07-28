#include "PASS1SICXE.cpp"

using namespace std;

ifstream intermediateFile; 
ofstream passTwoErrorFile,objectFile,ListingFile;
//passTwoErrorFile -> contains all the errors generated during the second pass of the assembler
//objectFile -> contains the object code of the program, including header record, text record, modification record, and end record
//ListingFile -> contains the object code for every instruction

ofstream printtab ;
string writestring ;
  
bool isComment;
string label,opcode,operand,comment;
string operand1,operand2;

int lineNumber,blockNumber,address,startAddress;
string objectCode, writeData, currentRecord, modificationRecord="M^", endRecord, currentSectName="DEFAULT";

int program_counter, base_register_value, currentTextRecordLength;
bool nobase;

string readTillTab(string data,int& index){
  string tempBuffer = "";

  while(index<data.length() && data[index] != '\t'){
    tempBuffer += data[index];
    index++;
  }
  index++;
  if(tempBuffer==" "){
    tempBuffer="-1" ;
  }
  return tempBuffer;
}
bool readIntermediateFile(ifstream& readFile,bool& isComment, int& lineNumber, int& address, int& blockNumber, string& label, string& opcode, string& operand, string& comment){
  string fileLine="";
  string tempBuffer="";
  bool tempStatus;
  int index=0;
  if(!getline(readFile, fileLine)){
    return false;
  }
  lineNumber = stoi(readTillTab(fileLine,index));

  isComment = (fileLine[index]=='.')?true:false;
  if(isComment){
    readFirstNonWhiteSpace(fileLine,index,tempStatus,comment,true);
    return true;
  }
  address = hexadecimalStringToInt(readTillTab(fileLine,index));
  tempBuffer = readTillTab(fileLine,index);
  if(tempBuffer == " "){
    blockNumber = -1;
  }
  else{
    blockNumber = stoi(tempBuffer);
  }
  label = readTillTab(fileLine,index);
  if(label=="-1"){
    label=" " ;
  }
  opcode = readTillTab(fileLine,index);
  if(opcode=="BYTE"){
    readByteOperand(fileLine,index,tempStatus,operand);
  }
  else{
    operand = readTillTab(fileLine,index);
    if(operand=="-1"){
      operand=" " ;
    }
  }
  readFirstNonWhiteSpace(fileLine,index,tempStatus,comment,true);  
  return true;
}

string createObjectCodeFormat34(){
  string objcode;
  int halfBytes;
  halfBytes = (getFlagFormat(opcode)=='+')?5:3;

  if(getFlagFormat(operand)=='#'){
    if(operand.substr(operand.length()-2,2)==",X"){
      writeData = "Line: "+to_string(lineNumber)+" Index based addressing not supported with Indirect addressing";
      writeToFile(passTwoErrorFile,writeData);
      objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+1,2);
      objcode += (halfBytes==5)?"100000":"0000";
      return objcode;
    }

    string tempOperand = operand.substr(1,operand.length()-1);
    if(checkIfAllDigits(tempOperand)||((SYMTAB[tempOperand].ifexists==1)&&(SYMTAB[tempOperand].relative==0))){
      int immediateValue;

      if(checkIfAllDigits(tempOperand)){
        immediateValue = stoi(tempOperand);
      }
      else{
        immediateValue = hexadecimalStringToInt(SYMTAB[tempOperand].address);
      }
      
      if(immediateValue>=(1<<4*halfBytes)){
        writeData = "Line: "+to_string(lineNumber)+" Immediate value exceeds format limit";
        writeToFile(passTwoErrorFile,writeData);
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+1,2);
        objcode += (halfBytes==5)?"100000":"0000";
      }
      else{
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+1,2);
        objcode += (halfBytes==5)?'1':'0';
        objcode += ConvertIntToHexadecimalString(immediateValue,halfBytes);
      }
      return objcode;
    }
    else if(SYMTAB[tempOperand].ifexists==0) {
     
      if(halfBytes==3) { 
         writeData += " : Symbol doesn't exists. Found " + tempOperand; 
      writeToFile(passTwoErrorFile,writeData);
      objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+1,2);
      objcode += (halfBytes==5)?"100000":"0000";
      return objcode;
    }
            if(SYMTAB[tempOperand].ifexists==1) {
             objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+1,2);
             objcode += "100000";

             modificationRecord += "M^" + ConvertIntToHexadecimalString(address+1,6) + '^';
             modificationRecord += "05+";
             modificationRecord += '\n';       

            return objcode ;
         }
    }
    else{
      int operandAddress = hexadecimalStringToInt(SYMTAB[tempOperand].address) + hexadecimalStringToInt(BLOCKS[BLocksNumToName[SYMTAB[tempOperand].blockNumber]].startAddress);

      if(halfBytes==5){
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+1,2);
        objcode += '1';
        objcode += ConvertIntToHexadecimalString(operandAddress,halfBytes);

        
        modificationRecord += "M^" + ConvertIntToHexadecimalString(address+1,6) + '^';
        modificationRecord += (halfBytes==5)?"05":"03";
        modificationRecord += '\n';

        return objcode;
      }
      program_counter = address + hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress);
      program_counter += (halfBytes==5)?4:3;
      int relativeAddress = operandAddress - program_counter;

     
      if(relativeAddress>=(-2048) && relativeAddress<=2047){
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+1,2);
        objcode += '2';
        objcode += ConvertIntToHexadecimalString(relativeAddress,halfBytes);
        return objcode;
      }
      
      if(!nobase){
        relativeAddress = operandAddress - base_register_value;
        if(relativeAddress>=0 && relativeAddress<=4095){
          objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+1,2);
          objcode += '4';
          objcode += ConvertIntToHexadecimalString(relativeAddress,halfBytes);
          return objcode;
        }
      }
      
      if(operandAddress<=4095){
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+1,2);
        objcode += '0';
        objcode += ConvertIntToHexadecimalString(operandAddress,halfBytes);

        modificationRecord += "M^" + ConvertIntToHexadecimalString(address+1+hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress),6) + '^';
        modificationRecord += (halfBytes==5)?"05":"03";
        modificationRecord += '\n';

        return objcode;
      }
    }
  }
  else if(getFlagFormat(operand)=='@'){
    string tempOperand = operand.substr(1,operand.length()-1);
    if(tempOperand.substr(tempOperand.length()-2,2)==",X" || SYMTAB[tempOperand].ifexists==0){
      
      writeData = "Line "+to_string(lineNumber);
      writeData += (SYMTAB[tempOperand].ifexists==0)?": Symbol doesn't exists":" Index based addressing not supported with Indirect addressing";
      writeToFile(passTwoErrorFile,writeData);
      objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+2,2);
      objcode += (halfBytes==5)?"100000":"0000";
      return objcode;
   
}
    int operandAddress = hexadecimalStringToInt(SYMTAB[tempOperand].address) + hexadecimalStringToInt(BLOCKS[BLocksNumToName[SYMTAB[tempOperand].blockNumber]].startAddress);
    program_counter = address + hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress);
    program_counter += (halfBytes==5)?4:3;

    if(halfBytes==3){
      int relativeAddress = operandAddress - program_counter;
      if(relativeAddress>=(-2048) && relativeAddress<=2047){
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+2,2);
        objcode += '2';
        objcode += ConvertIntToHexadecimalString(relativeAddress,halfBytes);
        return objcode;
      }

      if(!nobase){
        relativeAddress = operandAddress - base_register_value;
        if(relativeAddress>=0 && relativeAddress<=4095){
          objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+2,2);
          objcode += '4';
          objcode += ConvertIntToHexadecimalString(relativeAddress,halfBytes);
          return objcode;
        }
      }

      if(operandAddress<=4095){
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+2,2);
        objcode += '0';
        objcode += ConvertIntToHexadecimalString(operandAddress,halfBytes);

        
        modificationRecord += "M^" + ConvertIntToHexadecimalString(address+1+hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress),6) + '^';
        modificationRecord += (halfBytes==5)?"05":"03";
        modificationRecord += '\n';

        return objcode;
      }
    }
    else{
      objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+2,2);
      objcode += '1';
      objcode += ConvertIntToHexadecimalString(operandAddress,halfBytes);

      
      modificationRecord += "M^" + ConvertIntToHexadecimalString(address+1+hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress),6) + '^';
      modificationRecord += (halfBytes==5)?"05":"03";
      modificationRecord += '\n';

      return objcode;
    }

    writeData = "Line: "+to_string(lineNumber);
    writeData += "Can't fit into program counter based or base register based addressing.";
    writeToFile(passTwoErrorFile,writeData);
    objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+2,2);
    objcode += (halfBytes==5)?"100000":"0000";
    return objcode;
  }
  else if(getFlagFormat(operand)=='='){
    string tempOperand = operand.substr(1,operand.length()-1);

    if(tempOperand=="*"){
      tempOperand = "X'" + ConvertIntToHexadecimalString(address,6) + "'";

      modificationRecord += "M^" + ConvertIntToHexadecimalString(hexadecimalStringToInt(LITTAB[tempOperand].address)+hexadecimalStringToInt(BLOCKS[BLocksNumToName[LITTAB[tempOperand].blockNumber]].startAddress),6) + '^';
      modificationRecord += ConvertIntToHexadecimalString(6,2);
      modificationRecord += '\n';
    }

    if(LITTAB[tempOperand].ifexists==0){
      writeData = "Line "+to_string(lineNumber)+" : Symbol doesn't exists. Found " + tempOperand;
      writeToFile(passTwoErrorFile,writeData);

      objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
      objcode += (halfBytes==5)?"000":"0";
      objcode += "000";
      return objcode;
    }

    int operandAddress = hexadecimalStringToInt(LITTAB[tempOperand].address) + hexadecimalStringToInt(BLOCKS[BLocksNumToName[LITTAB[tempOperand].blockNumber]].startAddress);
    program_counter = address + hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress);
    program_counter += (halfBytes==5)?4:3;

    if(halfBytes==3){
      int relativeAddress = operandAddress - program_counter;
      if(relativeAddress>=(-2048) && relativeAddress<=2047){
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
        objcode += '2';
        objcode += ConvertIntToHexadecimalString(relativeAddress,halfBytes);
        return objcode;
      }

      if(!nobase){
        relativeAddress = operandAddress - base_register_value;
        if(relativeAddress>=0 && relativeAddress<=4095){
          objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
          objcode += '4';
          objcode += ConvertIntToHexadecimalString(relativeAddress,halfBytes);
          return objcode;
        }
      }

      if(operandAddress<=4095){
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
        objcode += '0';
        objcode += ConvertIntToHexadecimalString(operandAddress,halfBytes);

       
        modificationRecord += "M^" + ConvertIntToHexadecimalString(address+1+hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress),6) + '^';
        modificationRecord += (halfBytes==5)?"05":"03";
        modificationRecord += '\n';

        return objcode;
      }
    }
    else{
      objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
      objcode += '1';
      objcode += ConvertIntToHexadecimalString(operandAddress,halfBytes);

     
      modificationRecord += "M^" + ConvertIntToHexadecimalString(address+1+hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress),6) + '^';
      modificationRecord += (halfBytes==5)?"05":"03";
      modificationRecord += '\n';

      return objcode;
    }

    writeData = "Line: "+to_string(lineNumber);
    writeData += "Can't fit into program counter based or base register based addressing.";
    writeToFile(passTwoErrorFile,writeData);
    objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
    objcode += (halfBytes==5)?"100":"0";
    objcode += "000";

    return objcode;
  }
  else{
    int xbpe=0;
    string tempOperand = operand;
    if(operand.substr(operand.length()-2,2)==",X"){
      tempOperand = operand.substr(0,operand.length()-2);
      xbpe = 8;
    }

   if(SYMTAB[tempOperand].ifexists==0){
      writeData = "Line "+to_string(lineNumber)+" : Symbol doesn't exists. Found " + tempOperand;
      writeToFile(passTwoErrorFile,writeData);

      objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
      objcode += (halfBytes==5)?(ConvertIntToHexadecimalString(xbpe+1,1)+"00"):ConvertIntToHexadecimalString(xbpe,1);
      objcode += "000";
      return objcode;
    }
else{
  
    int operandAddress = hexadecimalStringToInt(SYMTAB[tempOperand].address) + hexadecimalStringToInt(BLOCKS[BLocksNumToName[SYMTAB[tempOperand].blockNumber]].startAddress);
    program_counter = address + hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress);
    program_counter += (halfBytes==5)?4:3;

    if(halfBytes==3){
      int relativeAddress = operandAddress - program_counter;
      if(relativeAddress>=(-2048) && relativeAddress<=2047){
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
        objcode += ConvertIntToHexadecimalString(xbpe+2,1);
        objcode += ConvertIntToHexadecimalString(relativeAddress,halfBytes);
        return objcode;
      }

      if(!nobase){
        relativeAddress = operandAddress - base_register_value;
        if(relativeAddress>=0 && relativeAddress<=4095){
          objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
          objcode += ConvertIntToHexadecimalString(xbpe+4,1);
          objcode += ConvertIntToHexadecimalString(relativeAddress,halfBytes);
          return objcode;
        }
      }

      if(operandAddress<=4095){
        objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
        objcode += ConvertIntToHexadecimalString(xbpe,1);
        objcode += ConvertIntToHexadecimalString(operandAddress,halfBytes);

       
        modificationRecord += "M^" + ConvertIntToHexadecimalString(address+1+hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress),6) + '^';
        modificationRecord += (halfBytes==5)?"05":"03";
        modificationRecord += '\n';

        return objcode;
      }
    }
    else{
      objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
      objcode += ConvertIntToHexadecimalString(xbpe+1,1);
      objcode += ConvertIntToHexadecimalString(operandAddress,halfBytes);

     
      modificationRecord += "M^" + ConvertIntToHexadecimalString(address+1+hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress),6) + '^';
      modificationRecord += (halfBytes==5)?"05":"03";
      modificationRecord += '\n';

      return objcode;
    }

    writeData = "Line: "+to_string(lineNumber);
    writeData += "Can't fit into program counter based or base register based addressing.";
    writeToFile(passTwoErrorFile,writeData);
    objcode = ConvertIntToHexadecimalString(hexadecimalStringToInt(OPTAB[getActualOpcode(opcode)].opcode)+3,2);
    objcode += (halfBytes==5)?(ConvertIntToHexadecimalString(xbpe+1,1)+"00"):ConvertIntToHexadecimalString(xbpe,1);
    objcode += "000";

    return objcode;
  }}

  return objcode;
}
void writeTextRecord(bool lastRecord=false){
  if(lastRecord){
    if(currentRecord.length()>0){//Write last text record
      writeData = ConvertIntToHexadecimalString(currentRecord.length()/2,2) + '^' + currentRecord;
      writeToFile(objectFile,writeData);
      currentRecord = "";
    }
    return;
  }
  if(objectCode != ""){
    if(currentRecord.length()==0){
      writeData = "T^" + ConvertIntToHexadecimalString(address+hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress),6) + '^';
      writeToFile(objectFile,writeData,false);
    }
   
    if((currentRecord + objectCode).length()>60){
      
      writeData = ConvertIntToHexadecimalString(currentRecord.length()/2,2) + '^' + currentRecord;
      writeToFile(objectFile,writeData);

     
      currentRecord = "";
      writeData = "T^" + ConvertIntToHexadecimalString(address+hexadecimalStringToInt(BLOCKS[BLocksNumToName[blockNumber]].startAddress),6) + '^';
      writeToFile(objectFile,writeData,false);
    }

    currentRecord += objectCode;
  }
  else{
    if(opcode=="START"||opcode=="END"||opcode=="BASE"||opcode=="NOBASE"||opcode=="LTORG"||opcode=="ORG"||opcode=="EQU"){
     
    }
    else{
      
      if(currentRecord.length()>0){
        writeData = ConvertIntToHexadecimalString(currentRecord.length()/2,2) + '^' + currentRecord;
        writeToFile(objectFile,writeData);
      }
      currentRecord = "";
    }
  }
}

void writeEndRecord(bool write=true){
  if(write){
    if(endRecord.length()>0){
      writeToFile(objectFile,endRecord);
     
    }
    else{
      writeEndRecord(false);
    }
  }
    int firstExecutableAddress;
   
      firstExecutableAddress = hexadecimalStringToInt(SYMTAB[firstExecutable_Sec].address);
    
    endRecord = "E^" + ConvertIntToHexadecimalString(firstExecutableAddress,6)+"\n";
}

void pass2(){
  string tempBuffer;
  intermediateFile.open("intermediate.txt");
  if(!intermediateFile){
    cout<<"Unable to open file: intermediate.txt"<<endl;
    exit(1);
  }
  getline(intermediateFile, tempBuffer);

  objectFile.open("object.txt");
  if(!objectFile){
    cout<<"Unable to open file: object.txt"<<endl;
    exit(1);
  }

  ListingFile.open("listing.txt");
  if(!ListingFile){
    cout<<"Unable to open file: listing.txt"<<endl;
    exit(1);
  }
  writeToFile(ListingFile,"Line\tAddress\tLabel\tOPCODE\tOPERAND\tObjectCode\tComment");

  passTwoErrorFile.open("pass_two_errors.txt");
  if(!passTwoErrorFile){
    cout<<"Unable to open file: pass_two_errors.txt"<<endl;
    exit(1);
  }
  writeToFile(passTwoErrorFile,"###############PASS2###############");
  objectCode = "";
  currentTextRecordLength=0;
  currentRecord = "";
  modificationRecord = "";
  blockNumber = 0;
  nobase = true;

  readIntermediateFile(intermediateFile,isComment,lineNumber,address,blockNumber,label,opcode,operand,comment);
  while(isComment){
    writeData = to_string(lineNumber) + "\t" + comment;
    writeToFile(ListingFile,writeData);
    readIntermediateFile(intermediateFile,isComment,lineNumber,address,blockNumber,label,opcode,operand,comment);
  }

  if(opcode=="START"){
    startAddress = address;
    writeData = to_string(lineNumber) + "\t\t" + ConvertIntToHexadecimalString(address) + "\t\t" + to_string(blockNumber) + "\t\t" + label + "\t\t" + opcode + "\t\t" + operand + "\t\t" + objectCode +"\t\t" + comment;
    writeToFile(ListingFile,writeData);
  }
  else{
    label = "";
    startAddress = 0;
    address = 0;
  }
  
  writeData = "H^"+increaseStringLength(label,6,' ',true)+'^'+ConvertIntToHexadecimalString(address,6)+'^'+ConvertIntToHexadecimalString(program_length,6);
  writeToFile(objectFile,writeData);
 
  readIntermediateFile(intermediateFile,isComment,lineNumber,address,blockNumber,label,opcode,operand,comment);
  currentTextRecordLength  = 0;

  while(opcode!="END"){
    if(!isComment){
      if(OPTAB[getActualOpcode(opcode)].isValid == 1){
        if(OPTAB[getActualOpcode(opcode)].instructionFormat==1){
          objectCode = OPTAB[getActualOpcode(opcode)].opcode;
        }
        else if(OPTAB[getActualOpcode(opcode)].instructionFormat==2){
          operand1 = operand.substr(0,operand.find(','));
          operand2 = operand.substr(operand.find(',')+1,operand.length()-operand.find(',') -1 );

          if(operand2==operand){
            if(getActualOpcode(opcode)=="SVC"){
              objectCode = OPTAB[getActualOpcode(opcode)].opcode + ConvertIntToHexadecimalString(stoi(operand1),1) + '0';
            }
            else if(REGTAB[operand1].ifexists==1){
              objectCode = OPTAB[getActualOpcode(opcode)].opcode + REGTAB[operand1].symb + '0';
            }
            else{
              objectCode = getActualOpcode(opcode) + '0' + '0';
              writeData = "Line: "+to_string(lineNumber)+" Invalid Register name";
              writeToFile(passTwoErrorFile,writeData);
            }
          }
          else{
            if(REGTAB[operand1].ifexists==0){
              objectCode = OPTAB[getActualOpcode(opcode)].opcode + "00";
              writeData = "Line: "+to_string(lineNumber)+" Invalid Register name";
              writeToFile(passTwoErrorFile,writeData);
            }
            else if(getActualOpcode(opcode)=="SHIFTR" || getActualOpcode(opcode)=="SHIFTL"){
              objectCode = OPTAB[getActualOpcode(opcode)].opcode + REGTAB[operand1].symb + ConvertIntToHexadecimalString(stoi(operand2),1);
            }
            else if(REGTAB[operand2].ifexists == 0){
              objectCode = OPTAB[getActualOpcode(opcode)].opcode + "00";
              writeData = "Line: "+to_string(lineNumber)+" Invalid Register name";
              writeToFile(passTwoErrorFile,writeData);
            }
            else{
              objectCode = OPTAB[getActualOpcode(opcode)].opcode + REGTAB[operand1].symb + REGTAB[operand2].symb;
            }
          }
        }
        else if(OPTAB[getActualOpcode(opcode)].instructionFormat==3){
          if(getActualOpcode(opcode)=="RSUB"){
            objectCode = OPTAB[getActualOpcode(opcode)].opcode;
            objectCode += (getFlagFormat(opcode)=='+')?"000000":"0000";
          }
          else{
            objectCode = createObjectCodeFormat34();
          }
        }
      }
      else if(opcode=="BYTE"){
        if(operand[0]=='X'){
          objectCode = operand.substr(2,operand.length()-3);
        }
        else if(operand[0]=='C'){
          objectCode = stringToHexString(operand.substr(2,operand.length()-3));
        }
      }
      else if(label=="*"){
        if(opcode[1]=='C'){
          objectCode = stringToHexString(opcode.substr(3,opcode.length()-4));
        }
        else if(opcode[1]=='X'){
          objectCode = opcode.substr(3,opcode.length()-4);
        }
      }
      else if(opcode=="WORD"){
        objectCode = ConvertIntToHexadecimalString(stoi(operand),6);
      }
      else if(opcode=="BASE"){
        if(SYMTAB[operand].ifexists==1){
          base_register_value = hexadecimalStringToInt(SYMTAB[operand].address) + hexadecimalStringToInt(BLOCKS[BLocksNumToName[SYMTAB[operand].blockNumber]].startAddress);
          nobase = false;
        }
        else{
          writeData = "Line "+to_string(lineNumber)+" : Symbol doesn't exists. Found " + operand;
          writeToFile(passTwoErrorFile,writeData);
        }
        objectCode = "";
      }
      else if(opcode=="NOBASE"){
        if(nobase){
          writeData = "Line "+to_string(lineNumber)+": Assembler wasn't using base addressing";
          writeToFile(passTwoErrorFile,writeData);
        }
        else{
          nobase = true;
        }
        objectCode = "";
      }
      else{
        objectCode = "";
      }
      
      writeTextRecord();

      if(blockNumber==-1 && address!=-1){
        writeData = to_string(lineNumber) + "\t" + ConvertIntToHexadecimalString(address) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
      }
      else{ writeData = to_string(lineNumber) + "\t" + ConvertIntToHexadecimalString(address) + "\t" + to_string(blockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
      }
    }
    else{
      writeData = to_string(lineNumber) + "\t" + comment;
    }
    writeToFile(ListingFile,writeData);//Write listing line
    readIntermediateFile(intermediateFile,isComment,lineNumber,address,blockNumber,label,opcode,operand,comment);//Read next line
    objectCode = "";
  }
  writeTextRecord();

  
  writeEndRecord(false);
  
   if(!isComment){
  writeData = to_string(lineNumber) + "\t" + ConvertIntToHexadecimalString(address) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + "" +"\t" + comment;
}else{
  writeData = to_string(lineNumber) + "\t" + comment;
 }
  writeToFile(ListingFile,writeData);


  while(readIntermediateFile(intermediateFile,isComment,lineNumber,address,blockNumber,label,opcode,operand,comment)){
    if(label=="*"){
      if(opcode[1]=='C'){
        objectCode = stringToHexString(opcode.substr(3,opcode.length()-4));
      }
      else if(opcode[1]=='X'){
        objectCode = opcode.substr(3,opcode.length()-4);
      }
      writeTextRecord();
    }
    writeData = to_string(lineNumber) + "\t" + ConvertIntToHexadecimalString(address) + "\t" + to_string(blockNumber) + label + "\t" + opcode + "\t" + operand + "\t" + objectCode +"\t" + comment;
    writeToFile(ListingFile,writeData);
  }
  
writeTextRecord(true);
if(!isComment){
  
  writeToFile(objectFile,modificationRecord,false);
  writeEndRecord(true);
  currentSectName=label;
  modificationRecord="";
}
if(!isComment&&opcode!="END"){
writeData = "\n********************object program for "+ label+" **************";
  writeToFile(objectFile,writeData);

writeData = "\nH^"+increaseStringLength(label,6,' ',true)+'^'+ConvertIntToHexadecimalString(address,6);
  writeToFile(objectFile,writeData);
  }

  readIntermediateFile(intermediateFile,isComment,lineNumber,address,blockNumber,label,opcode,operand,comment);
    objectCode = "";

    passTwoErrorFile.close();
}

int main(){
  cout<<"** NOTE: The Input file and executable assembler file should be in same folder **"<<endl<<endl;
  cout<<"Enter name of the Input File=";
  cin>>inputFile;

  cout<<"\nLoading OPTAB"<<endl;
  load_tables_on_program_start();

  cout<<"\nPerforming Pass 1"<<endl;
  cout<<"Writing the Intermediate File to \'intermediate.txt\'"<<endl;
  cout<<"Writing the Error File to \'pass_one_errors.txt\'"<<endl;
  pass1();

cout<<"Creating Symbol Table in \'tables.txt\'"<<endl;
  printtab.open("tables.txt") ;
  writeToFile(printtab,"################################# SYMBOL TABLE #################################\n") ;
    for (auto const& it: SYMTAB) { 
        writestring+=it.first+":-\t"+ "name:"+it.second.name+"\t|"+ "address:"+it.second.address+"\t|"+ "relative:"+ConvertIntToHexadecimalString(it.second.relative)+" \n" ;
    } 
    writeToFile(printtab,writestring) ;

writestring="" ;
    cout<<"Creating Literal Table in \'tables.txt\'"<<endl;
  
  writeToFile(printtab,"################################# LITERAL TABLE #################################\n") ;
    for (auto const& it: LITTAB) { 
        writestring+=it.first+":-\t"+ "value:"+it.second.value+"\t|"+ "address:"+it.second.address+" \n" ;
    } 
    writeToFile(printtab,writestring) ;
    
writestring="" ;
    cout<<"Creating Block Table in \'tables.txt\'"<<endl;
  
  writeToFile(printtab,"################################# BLOCK TABLE #################################\n") ;
    for (auto const& it: BLOCKS) { 
        writestring+=it.first+":-\t"+ "value:"+it.second.name+"\t|"+ "address:"+it.second.startAddress+" \n" ;
    } 
    writeToFile(printtab,writestring) ;
  cout<<"\nPerforming Pass 2"<<endl;
  cout<<"Writing the Object File to \'object.txt\'"<<endl;
  cout<<"Writing the Listing File to \'listing.txt\'"<<endl;
  pass2();

}
