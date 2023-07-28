#include <iostream>
#include <bits/stdc++.h>
// this file contains all the required tables, i.e, SYMBOL TABLE, OPCODE TABLE, BLOCK TABLE, LITERAL TABLE AND REGISTER TABLE

using namespace std;

struct OPCODE{
  string opcode; // the opcode of the instruction
  int instructionFormat;    // the format of the instruction(1,2 3, or 4)
  bool isValid;  // tells if the opcode is valid or not, if invalid, it shows an error
  OPCODE(){
    //initialization
    opcode = ""; instructionFormat = 0; isValid = 0;
  }
};

struct LITERAL{
  string value;        // the value of the literal
  string address;      // the address of the  literanl
  bool ifexists;         // tells if it is present in the literal table or not
  int blockNumber = 0; // blockNumber of the literal, used in Program Blocks
  LITERAL(){
      //initialization
    value = ""; address = "?"; blockNumber = 0; ifexists = 0;
  }
};

struct LABEL{
  string address;  // the address where the symbol is defined
  string name;     // the name of the label
  int relative;    // relative displacement
  int blockNumber; // blockNumber where the label is defined
  bool ifexists;     // tells if the label is present in the symbol table or not
  LABEL(){
    //initialization
    name = "undefined"; address = "0"; blockNumber = 0; ifexists = 0; relative = 0;
  }
};

struct BLOCKS_DEF{
  string startAddress; // starting address of the block
  string name;         // block name
  string LOCCTR; //used to assign address to the symbols
  int number;  // block number
  bool ifexists; // tells if the block is present in the block table or not
  BLOCKS_DEF(){
     //initialization
    name = "undefined"; startAddress = "?"; ifexists = 0; number = -1; LOCCTR = "0";
  }
};

struct REGISTER{
  char symb;    // every register has a register symbol assigned to it
  bool ifexists; // tells if the register is a valid register, i.e, if it is present in the register table or not
  REGISTER()
  {
      //initialization
    symb = 'F'; ifexists = 0;
  }
};

map<string, LABEL> SYMTAB; // a table which keeps track of all the symbols used in the program
map<string, OPCODE> OPTAB; // a table which keeps track of all the valid operations in SIC-XE, it is loaded when the program starts
map<string, REGISTER> REGTAB; // a table which keeps track of all the registers in the SIC-XE, it is loaded when the program starts
map<string, LITERAL> LITTAB; // a table which keeps track of all the literals in the programs
map<string, BLOCKS_DEF> BLOCKS;// a table used to maintain program blocks, each instruction in the SIC-XE program is given a block number


void load_REGTAB_on_start(){
  // A, X, L, B, S, T F, PC AND SW are the only registers used, any other register name will show an error
  REGTAB["A"].symb = '0'; REGTAB["A"].ifexists = 1;

  REGTAB["X"].symb = '1'; REGTAB["X"].ifexists = 1;

  REGTAB["L"].symb = '2'; REGTAB["L"].ifexists = 1;

  REGTAB["B"].symb = '3'; REGTAB["B"].ifexists = 1;

  REGTAB["S"].symb = '4'; REGTAB["S"].ifexists = 1;

  REGTAB["T"].symb = '5'; REGTAB["T"].ifexists = 1;

  REGTAB["F"].symb = '6'; REGTAB["F"].ifexists = 1;

  REGTAB["PC"].symb = '8'; REGTAB["PC"].ifexists = 1;

  REGTAB["SW"].symb = '9'; REGTAB["SW"].ifexists = 1;
}
void load_OPTAB_on_start(){
  //there are a total of 58 valid operations in the OPTAB

  OPTAB["ADD"].opcode = "18"; OPTAB["ADD"].instructionFormat = 3; OPTAB["ADD"].isValid = 1;

  OPTAB["ADDF"].opcode = "58"; OPTAB["ADDF"].instructionFormat = 3; OPTAB["ADDF"].isValid = 1;

  OPTAB["ADDR"].opcode = "90"; OPTAB["ADDR"].instructionFormat = 2; OPTAB["ADDR"].isValid = 1;

  OPTAB["AND"].opcode = "40"; OPTAB["AND"].instructionFormat = 3; OPTAB["AND"].isValid = 1;

  OPTAB["CLEAR"].opcode = "B4"; OPTAB["CLEAR"].instructionFormat = 2; OPTAB["CLEAR"].isValid = 1;

  OPTAB["COMP"].opcode = "28"; OPTAB["COMP"].instructionFormat = 3; OPTAB["COMP"].isValid = 1;

  OPTAB["COMPF"].opcode = "88"; OPTAB["COMPF"].instructionFormat = 3; OPTAB["COMPF"].isValid = 1;

  OPTAB["COMPR"].opcode = "A0"; OPTAB["COMPR"].instructionFormat = 2; OPTAB["COMPR"].isValid = 1;

  OPTAB["DIV"].opcode = "24"; OPTAB["DIV"].instructionFormat = 3; OPTAB["DIV"].isValid = 1;

  OPTAB["DIVF"].opcode = "64"; OPTAB["DIVF"].instructionFormat = 3; OPTAB["DIVF"].isValid = 1;

  OPTAB["DIVR"].opcode = "9C"; OPTAB["DIVR"].instructionFormat = 2; OPTAB["DIVR"].isValid = 1;

  OPTAB["FIX"].opcode = "C4"; OPTAB["FIX"].instructionFormat = 1; OPTAB["FIX"].isValid = 1;

  OPTAB["FLOAT"].opcode = "C0"; OPTAB["FLOAT"].instructionFormat = 1; OPTAB["FLOAT"].isValid = 1;

  OPTAB["HIO"].opcode = "F4"; OPTAB["HIO"].instructionFormat = 1; OPTAB["HIO"].isValid = 1;

  OPTAB["J"].opcode = "3C"; OPTAB["J"].instructionFormat = 3; OPTAB["J"].isValid = 1;

  OPTAB["JEQ"].opcode = "30"; OPTAB["JEQ"].instructionFormat = 3; OPTAB["JEQ"].isValid = 1;

  OPTAB["JGT"].opcode = "34"; OPTAB["JGT"].instructionFormat = 3; OPTAB["JGT"].isValid = 1;

  OPTAB["JLT"].opcode = "38"; OPTAB["JLT"].instructionFormat = 3; OPTAB["JLT"].isValid = 1;

  OPTAB["JSUB"].opcode = "48"; OPTAB["JSUB"].instructionFormat = 3; OPTAB["JSUB"].isValid = 1;

  OPTAB["LDA"].opcode = "00"; OPTAB["LDA"].instructionFormat = 3; OPTAB["LDA"].isValid = 1;

  OPTAB["LDB"].opcode = "68"; OPTAB["LDB"].instructionFormat = 3; OPTAB["LDB"].isValid = 1;

  OPTAB["LDCH"].opcode = "50"; OPTAB["LDCH"].instructionFormat = 3; OPTAB["LDCH"].isValid = 1;

  OPTAB["LDF"].opcode = "70"; OPTAB["LDF"].instructionFormat = 3; OPTAB["LDF"].isValid = 1;

  OPTAB["LDL"].opcode = "08"; OPTAB["LDL"].instructionFormat = 3; OPTAB["LDL"].isValid = 1;

  OPTAB["LDS"].opcode = "6C"; OPTAB["LDS"].instructionFormat = 3; OPTAB["LDS"].isValid = 1;

  OPTAB["LDT"].opcode = "74"; OPTAB["LDT"].instructionFormat = 3; OPTAB["LDT"].isValid = 1;

  OPTAB["LDX"].opcode = "04"; OPTAB["LDX"].instructionFormat = 3; OPTAB["LDX"].isValid = 1;

  OPTAB["LPS"].opcode = "D0"; OPTAB["LPS"].instructionFormat = 3; OPTAB["LPS"].isValid = 1;

  OPTAB["MUL"].opcode = "20"; OPTAB["MUL"].instructionFormat = 3; OPTAB["MUL"].isValid = 1;

  OPTAB["MULF"].opcode = "60"; OPTAB["MULF"].instructionFormat = 3; OPTAB["MULF"].isValid = 1;

  OPTAB["MULR"].opcode = "98"; OPTAB["MULR"].instructionFormat = 2; OPTAB["MULR"].isValid = 1;

  OPTAB["NORM"].opcode = "C8"; OPTAB["NORM"].instructionFormat = 1; OPTAB["NORM"].isValid = 1;

  OPTAB["OR"].opcode = "44"; OPTAB["OR"].instructionFormat = 3; OPTAB["OR"].isValid = 1; 

  OPTAB["RD"].opcode = "D8"; OPTAB["RD"].instructionFormat = 3; OPTAB["RD"].isValid = 1;

  OPTAB["RMO"].opcode = "AC"; OPTAB["RMO"].instructionFormat = 2; OPTAB["RMO"].isValid = 1;

  OPTAB["RSUB"].opcode = "4F"; OPTAB["RSUB"].instructionFormat = 3; OPTAB["RSUB"].isValid = 1;

  OPTAB["SHIFTL"].opcode = "A4"; OPTAB["SHIFTL"].instructionFormat = 2; OPTAB["SHIFTL"].isValid = 1;

  OPTAB["SHIFTR"].opcode = "A8"; OPTAB["SHIFTR"].instructionFormat = 2; OPTAB["SHIFTR"].isValid = 1;

  OPTAB["SIO"].opcode = "F0"; OPTAB["SIO"].instructionFormat = 1; OPTAB["SIO"].isValid = 1;

  OPTAB["SSK"].opcode = "EC"; OPTAB["SSK"].instructionFormat = 3; OPTAB["SSK"].isValid = 1;

  OPTAB["STA"].opcode = "0C"; OPTAB["STA"].instructionFormat = 3; OPTAB["STA"].isValid = 1;

  OPTAB["STB"].opcode = "78"; OPTAB["STB"].instructionFormat = 3; OPTAB["STB"].isValid = 1;

  OPTAB["STCH"].opcode = "54"; OPTAB["STCH"].instructionFormat = 3; OPTAB["STCH"].isValid = 1;

  OPTAB["STF"].opcode = "80"; OPTAB["STF"].instructionFormat = 3; OPTAB["STF"].isValid = 1;

  OPTAB["STI"].opcode = "D4"; OPTAB["STI"].instructionFormat = 3; OPTAB["STI"].isValid = 1;

  OPTAB["STL"].opcode = "14"; OPTAB["STL"].instructionFormat = 3; OPTAB["STL"].isValid = 1;

  OPTAB["STS"].opcode = "7C"; OPTAB["STS"].instructionFormat = 3; OPTAB["STS"].isValid = 1;

  OPTAB["STSW"].opcode = "E8"; OPTAB["STSW"].instructionFormat = 3; OPTAB["STSW"].isValid = 1; 

  OPTAB["STT"].opcode = "84"; OPTAB["STT"].instructionFormat = 3;OPTAB["STT"].isValid = 1;

  OPTAB["STX"].opcode = "10"; OPTAB["STX"].instructionFormat = 3; OPTAB["STX"].isValid = 1;

  OPTAB["SUB"].opcode = "1C"; OPTAB["SUB"].instructionFormat = 3; OPTAB["SUB"].isValid = 1;

  OPTAB["SUBF"].opcode = "5C"; OPTAB["SUBF"].instructionFormat = 3; OPTAB["SUBF"].isValid = 1;

  OPTAB["SUBR"].opcode = "94"; OPTAB["SUBR"].instructionFormat = 2; OPTAB["SUBR"].isValid = 1;

  OPTAB["SVC"].opcode = "B0"; OPTAB["SVC"].instructionFormat = 2; OPTAB["SVC"].isValid = 1;

  OPTAB["TD"].opcode = "E0"; OPTAB["TD"].instructionFormat = 3; OPTAB["TD"].isValid = 1;

  OPTAB["TIO"].opcode = "F8"; OPTAB["TIO"].instructionFormat = 1; OPTAB["TIO"].isValid = 1;

  OPTAB["TIX"].opcode = "2C"; OPTAB["TIX"].instructionFormat = 3; OPTAB["TIX"].isValid = 1;

  OPTAB["TIXR"].opcode = "B8"; OPTAB["TIXR"].instructionFormat = 2; OPTAB["TIXR"].isValid = 1;

  OPTAB["WD"].opcode = "DC"; OPTAB["WD"].instructionFormat = 3; OPTAB["WD"].isValid = 1;
  
}

void load_BLOCKS_on_start(){
  BLOCKS["DEFAULT"].ifexists = 1; BLOCKS["DEFAULT"].name = "DEFAULT"; BLOCKS["DEFAULT"].startAddress = "00000"; BLOCKS["DEFAULT"].number = 0; BLOCKS["DEFAULT"].LOCCTR = "0";
}
void load_tables_on_program_start(){
  load_BLOCKS_on_start();
  load_OPTAB_on_start();
  load_REGTAB_on_start();
}

//all the function that are called in pass 1 and pass 2 are declared here

string ConvertIntToHexadecimalString(int x, int fill = 5){
  // to convert an integer to a hexadecimal string with a minimum of 5 chars.
  stringstream s;
  s << setfill('0') << setw(fill) << hex << x;
  string temp = s.str();
  temp = temp.substr(temp.length() - fill, fill);
  transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
  return temp;
}

string increaseStringLength(string data, int length, char fillChar, bool back = false){
  // here length is the required length and data is a string
  //if data length is short, we append from back if back is true else we append from front till the length of the data is equal to the required length
  if (back){
    if (length <= data.length()){
      //if data length is more we 
      return data.substr(0, length);
    }
    else{
      for (int i = length - data.length(); i > 0; i--){
        data += fillChar;
      }
    }
  }
  else{
    if (length <= data.length()){
      return data.substr(data.length() - length, length);
    }
    else{
      for (int i = length - data.length(); i > 0; i--){
        data = fillChar + data;
      }
    }
  }
  return data;
}
int hexadecimalStringToInt(string x){
  //taken a hexadecimal input and returns its value as an unsigned long long integer
  return stoul(x, nullptr, 16);
}

string stringToHexString(const string &input){
  static const char *const all_hex = "0123456789ABCDEF";
  int len = input.length();
  string output_string;
  //each char is 8 bits, so it can be represented as two hexadecimal digits, hence we need to reserve twice the space as the size of input string for the output string
  output_string.reserve(2 * len);
  for (size_t i = 0; i < len; ++i){
    const unsigned char c = input[i];
    //for each char it extracts the four MSBs and LSBs bits of its binary representation and then pushes its hexadecimal value into the string
    output_string.push_back(all_hex[c >> 4]);
    output_string.push_back(all_hex[c & 15]);
  }
  return output_string;
}

bool checkWhiteSpace(char x){
  //takes a character input and check if its a white space or a tab
  if (x == ' ' || x == '\t'){
    return true;
  }
  return false;
}

bool checkCommentLine(string line){
  //check if the line is a comment, if its not a comment then its an instruction
  if (line[0] == '.'){
    return true;
  }
  return false;
}

bool checkIfAllDigits(string x){
  //check if all the characters in the string are digits or not, returns true if they are
  bool ans = true;
  int i = 0;
  while (ans && (i < x.length())){
    ans &= isdigit(x[i++]);
  }
  return ans;
}

void readFirstNonWhiteSpace(string line, int &index, bool &status, string &data, bool readTillEnd = false){
  data = "";
  status = true;
  if (readTillEnd){
    data = line.substr(index, line.length() - index);
    if (data == ""){
      status = false;
    }
    return;
  }
  while (index < line.length() && !checkWhiteSpace(line[index])){ 
    // If no whitespace then data
    data += line[index];
    index++;
  }

  if (data == ""){
    status = false;
  }

  while (index < line.length() && checkWhiteSpace(line[index])){ 
    // Increase index to pass all whitespace
    index++;
  }
}

void readByteOperand(string line, int &index, bool &status, string &data){
  data = "";
  status = true;
  if (line[index] == 'C'){
    data += line[index++];
    char identifier = line[index++];
    data += identifier;
    while (index < line.length() && line[index] != identifier){ 
      // Copy all data until next identifier regardless of whitespace
      data += line[index];
      index++;
    }
    data += identifier;
    index++; 
    // Shift to next of identifier
  }
  else{
    while (index < line.length() && !checkWhiteSpace(line[index])){ 
      // In no whitespace then data
      data += line[index];
      index++;
    }
  }
  if (data == ""){
    status = false;
  }

  while (index < line.length() && checkWhiteSpace(line[index])){ 
    // Increase index to pass all whitespace
    index++;
  }
}

void writeToFile(ofstream &file, string data, bool newline = true){
  //this function writes the data to the output file
  if (newline){
    file << data << endl;
  }
  else{
    file << data;
  }
}

string getActualOpcode(string opcode){
  //returns the actual opcode after removing the first character if the character is a + or @
  if (opcode[0] == '+' || opcode[0] == '@'){
    return opcode.substr(1, opcode.length() - 1);
  }
  return opcode;
}

char getFlagFormat(string data){
  //this function returns the flagformat if one of these values matches
  if (data[0] == '#' || data[0] == '+' || data[0] == '@' || data[0] == '='){
    return data[0];
  }
  return ' ';
}


//this class and its methods are used for evaluating the arithmetic operations
class EvaluateString{
public:
  int getResult();
  EvaluateString(string data);

private:
  string storedData;//holds the expression to be evaluated
  int index;
  char peek();
  char get();
  int term();
  int factor();
  int number();
};

EvaluateString::EvaluateString(string data){
  storedData = data; index = 0;
}

//the getresult function calls the term function to get the value of first term in the expression
int EvaluateString::getResult(){
  int result = term();

  //it repeatedly checks in a loop if the next operator is '+' or '-'
  while (peek() == '+' || peek() == '-'){
    if (get() == '+'){
      result += term();
    }
    else{
      result -= term();
    }
  }
  return result;
}

// the term function gets the value of the first factor in the expression
int EvaluateString::term(){
  int result = factor();

  // it repeatedly checks if the next operator is a '*' or a '/'
  while (peek() == '*' || peek() == '/'){
    if (get() == '*'){
      result *= factor();
    }
    else{
      result /= factor();
    }
  }
  return result;
}

// the factor function handles factors in the expression, it checks if the factor is a number, a sub expression enclosed in parantheses or a negative number
int EvaluateString::factor(){
  if (peek() >= '0' && peek() <= '9'){
    return number();
  }
  else if (peek() == '('){
    get();
    int result = getResult();
    get();
    return result;
  }
  else if (peek() == '-'){
    get();
    return -factor();
  }
  return 0;
}

//this function reads the digits of the number and computes its value
int EvaluateString::number(){
  int result = get() - '0';
  while (peek() >= '0' && peek() <= '9'){
    result = 10 * result + get() - '0';
  }
  return result;
}

//returns the char at the current position and icrements its value
char EvaluateString::get(){
  return storedData[index++];
}


//returns char at current  position without incrementing the value
char EvaluateString::peek(){
  return storedData[index];
}