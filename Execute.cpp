#include "Assembler.hpp"

int binSTI(string s)
{
    return stoi(s,nullptr,2);
}
int binSTI(string s,int a)
{
    return stoi(s.substr(a),nullptr,2);
}
int binSTI(string s,int a,int b)
{
    return stoi(s.substr(a, b), nullptr, 2);
}

class RegFile
{
    vector<int> registers;

public:
    RegFile();
    int readReg(int);
    void writeReg(int, int);
    void displayFile();
};
RegFile::RegFile()
{
    registers = vector<int>(32, 0);
}
int RegFile::readReg(int rsl)
{
    return registers[rsl];
}
void RegFile::writeReg(int rdl, int value)
{
    registers[rdl] = value;
}
void RegFile::displayFile()
{
    fstream out("./Results/RegisterFile.txt", ios::out | ios::trunc);
    for (int i = 0; i <= 31; i++)
    {
        out << i << " " << registers[i];
        if (i != 31)
            out << "\n";
    }
    out.close();
}

class InstructionMemory
{
    vector<string> instrs;

public:
    InstructionMemory();
    pair<int, int> loadInstructions(int,string);
    string getInstr(int);
    void closeInstrMem();
};
InstructionMemory::InstructionMemory()
{
    instrs = vector<string>(1e5 + 10, string(32, '0'));
    ifstream in;
    in.open("./Results/InstructionMemory.txt");

    while (in.eof() == 0)
    {
        string x, y;
        in >> x >> y;
        instrs[stoi(x)] = y;
    }
    in.close();
}
pair<int, int> InstructionMemory::loadInstructions(int start,string fileName)
{
    Machine m;
    m.convertToMachineCode(fileName);
    vector<string> code = m.getMachineCode();

    int sz = code.size();
    for (int i = 0; i < sz; i++)
        instrs[i + start] = code[i];

    return {start, start + sz - 1};
}
string InstructionMemory::getInstr(int pc)
{
    return instrs[pc];
}
void InstructionMemory::closeInstrMem()
{
    fstream out("./Results/InstructionMemory.txt", ios::out | ios::trunc);
    for (int i = 1; i <= 1e5; i++)
    {
        out << i << " " << instrs[i];
        if (i != 1e5)
            out << "\n";
    }
    out.close();
}

class DataMemory
{
    vector<int> data;

public:
    DataMemory();
    int readMem(int);
    void writeMem(int, int);
    void closeMem();
    void displayMemory();
};
DataMemory::DataMemory()
{
    data = vector<int>(1e5 + 10, 0);
    ifstream in;

    in.open("./Results/DataMemory.txt");

    while (in.eof() == 0)
    {
        string x, y;
        in >> x >> y;
        if (y[0] == '-')
            data[stoi(x)] = -stoi(y.substr(1));
        else
            data[stoi(x)] = stoi(y);
    }
    in.close();
}
int DataMemory::readMem(int ea)
{
    return data[ea];
}
void DataMemory::writeMem(int ea, int value)
{
    data[ea] = value;
}
void DataMemory::closeMem()
{
    fstream out("./Results/DataMemory.txt", ios::out | ios::trunc);
    for (int i = 1; i <= 1e5; i++)
    {
        out << i << " " << data[i];
        if (i != 1e5)
            out << "\n";
    }
    out.close();
}
void DataMemory::displayMemory()
{
    for (int i = 1; i <= 1e3; i++)
    {
        cout << i << " " << readMem(i);
        if (i != 1e3)
            cout << "\n";
    }
}

enum CacheBlockState
{
    I,
    V,
    M
};
class CacheBlock
{
public:
    CacheBlockState st;
    int tag;
    vector<int> data;
    int counter;

    CacheBlock(int size)
    {
        st = I;
        tag = -1;
        data = vector<int>(size, 0);
        counter = 0;
    }
};
class Cache
{
    vector<vector<CacheBlock>> sets;
    int cacheSize;
    int blockSize;
    int way;
    int numberOfSets;
    int numberPerBlock;
    DataMemory dataMem;

public:
    void initialize(int cacheSize, int blockSize, int way);
    int readFromCache(int address);
    void writeIntoCache(int address, int data);
    int evictionAlgorithm(vector<CacheBlock> &currBlocks);
    CacheBlock selectBlock(CacheBlock, int, int, int, int);
    void closeMem();
};
void Cache::initialize(int cacheSize, int blockSize, int way)
{
    this->cacheSize = cacheSize;
    this->blockSize = blockSize;
    this->way = way;

    numberOfSets = cacheSize / blockSize / way;
    numberPerBlock = blockSize / 4;
    sets = vector<vector<CacheBlock>>(numberOfSets, vector<CacheBlock>(way, CacheBlock(numberPerBlock)));
}
int Cache::readFromCache(int address)
{
    int blockOffset = address % numberPerBlock;
    int tagPlusIndex = address / numberPerBlock;
    int reqIndex = tagPlusIndex % numberOfSets;
    int reqTag = tagPlusIndex / numberOfSets;
    bool found = 0;
    int ans;

    vector<CacheBlock> &currBlocks = sets[reqIndex];
    for (auto &block : currBlocks)
    {
        if (block.tag == reqTag)
        {
            if (block.st == V || block.st == M)
            {
                block.counter = 0;
                ans = (block.data)[blockOffset];
                found = 1;
            }
        }
        else
            block.counter++;
    }
    if (found)
        return ans;

    int pos = evictionAlgorithm(currBlocks);
    CacheBlock newBlock = selectBlock(currBlocks[pos], reqTag, reqIndex, -1, -1);
    currBlocks[pos] = newBlock;

    return (newBlock.data)[blockOffset];
}
void Cache::writeIntoCache(int address, int data)
{
    int blockOffset = address % numberPerBlock;
    int tagPlusIndex = address / numberPerBlock;
    int reqIndex = tagPlusIndex % numberOfSets;
    int reqTag = tagPlusIndex / numberOfSets;

    vector<CacheBlock> &currBlocks = sets[reqIndex];
    for (auto &block : currBlocks)
    {
        if (block.tag == reqTag)
        {
            if (block.st == V || block.st == M)
            {
                block.data[blockOffset] = data;
                block.st = M;
                block.counter = 0;
                return;
            }
        }
        else
            block.counter++;
    }

    int pos = evictionAlgorithm(currBlocks);
    CacheBlock newBlock = selectBlock(currBlocks[pos], reqTag, reqIndex, address, data);
    currBlocks[pos] = newBlock;
}
int Cache::evictionAlgorithm(vector<CacheBlock> &currBlocks)
{
    int maxCount = currBlocks[0].counter;
    int maxPos = 0;
    for (int i = 1; i < currBlocks.size(); i++)
    {
        if (currBlocks[i].counter > maxCount)
        {
            maxCount = currBlocks[i].counter;
            maxPos = i;
        }
    }
    return maxPos;
}
CacheBlock Cache::selectBlock(CacheBlock oldBlock, int reqTag, int reqIndex, int address, int data)
{
    CacheBlock newBlock(numberPerBlock);
    newBlock.tag = reqTag;
    newBlock.st = V;
    int tagPlusIndex = reqTag * numberOfSets + reqIndex;

    if (address != -1)
        dataMem.writeMem(address, data);

    for (int i = 0; i < numberPerBlock; i++)
        (newBlock.data)[i] = dataMem.readMem(tagPlusIndex * numberPerBlock + i);

    if (oldBlock.tag != -1 && oldBlock.st == M)
    {
        int tagPlusIndexOld = (oldBlock.tag) * numberOfSets + reqIndex;
        for (int i = 0; i < numberPerBlock; i++)
            dataMem.writeMem(tagPlusIndexOld * numberPerBlock + i, (oldBlock.data)[i]);
    }

    return newBlock;
}
void Cache::closeMem()
{
    // dataMem.displayMemory();
    dataMem.closeMem();
}

//Arithmetic and Logical Unit
class ArithmeticLogicUnit
{
    int input1;
    int input2;

public:
    void setInput1(int);
    void setInput2(int);
    int performOperation(int);
    bool checkBoolean(int, int);
};
void ArithmeticLogicUnit::setInput1(int input1)
{
    this->input1 = input1;
}
void ArithmeticLogicUnit::setInput2(int input2)
{
    this->input2 = input2;
}
int ArithmeticLogicUnit::performOperation(int aluSelect)
{
    if (aluSelect == 0)
        return input1 & input2;

    if (aluSelect == 1)
        return input1 | input2;

    if (aluSelect == 2)
        return input1 + input2;

    if (aluSelect == 6)
        return input1 - input2;

    return 0;
}
bool ArithmeticLogicUnit::checkBoolean(int funct3, int aluRes)
{
    if (funct3 == 0)
    {
        if (aluRes == 0)
            return 1;
        return 0;
    }

    if (funct3 == 5)
    {
        if (aluRes <= 0)
            return 1;
        return 0;
    }

    if (funct3 == 4)
    {
        if (aluRes > 0)
            return 1;
        return 0;
    }
    if (funct3 == 1)
    {
        if (aluRes != 0)
            return 1;
        return 0;
    }

    cout << aluRes << endl;
    return 0;
}

class ControlUnit
{
    bool regWrite_, regRead_, aluSrc_, branch_, memRead_, memWrite_;
    int8_t aluOp_, memToReg_;

public:
    void initialize(int);
    int aluSelect(int, int, int);

    bool regWrite();
    bool regRead();
    bool aluSrc();
    bool branch();
    bool memRead();
    bool memWrite();
    bool memToReg();
    int8_t aluOp();
};
void ControlUnit::initialize(int opcode)
{
    // R-Type
    if (opcode == 51)
    {
        regRead_=1;
        aluSrc_ = 0;
        memToReg_ = 0;
        regWrite_ = 1;
        memRead_ = 0;
        memWrite_ = 0;
        branch_ = 0;
        aluOp_ = 2;
    }

    // B-Type
    else if (opcode == 99)
    {
        regRead_=1;
        aluSrc_ = 0;
        memToReg_ = -1;
        regWrite_ = 0;
        memRead_ = 0;
        memWrite_ = 0;
        branch_ = 1;
        aluOp_ = 1;
    }

    // S-Type
    else if (opcode == 35)
    {
        regRead_=1;
        aluSrc_ = 1;
        memToReg_ = -1;
        regWrite_ = 0;
        memRead_ = 0;
        memWrite_ = 1;
        branch_ = 0;
        aluOp_ = 0;
    }

    // L-Type
    else if (opcode == 3)
    {
        regRead_=1;
        aluSrc_ = 1;
        memToReg_ = 1;
        regWrite_ = 1;
        memRead_ = 1;
        memWrite_ = 0;
        branch_ = 0;
        aluOp_ = 0;
    }

    // I-Type
    else if (opcode == 19)
    {
        regRead_=1;
        aluSrc_ = 1;
        memToReg_ = 0;
        regWrite_ = 1;
        memRead_ = 0;
        memWrite_ = 0;
        branch_ = 0;
        aluOp_ = 2;
    }
}
int ControlUnit::aluSelect(int funct3, int funct7, int opcode)
{
    if (aluOp_ == 0)
        return 2;

    if (aluOp_ == 1)
        return 6;

    if (aluOp_ == 2)
    {
        if (funct3 == 7)
            return 0;

        if (funct3 == 0)
        {
            if (funct7 == 1 && opcode != 19)
                return 6;
            else
                return 2;
        }

        if (funct3 == 6)
            return 1;
    }
    return 0;
}
bool ControlUnit::regWrite()
{
    return regWrite_;
}
bool ControlUnit::regRead()
{
    return regRead_;
}
bool ControlUnit::aluSrc()
{
    return aluSrc_;
}
bool ControlUnit::branch()
{
    return branch_;
}
bool ControlUnit::memRead()
{
    return memRead_;
}
bool ControlUnit::memWrite()
{
    return memWrite_;
}
bool ControlUnit::memToReg()
{
    return memToReg_;
}
int8_t ControlUnit::aluOp()
{
    return aluOp_;
}

class Execute
{
    RegFile regFile;
    Cache cache;
    InstructionMemory instrMem;

    ArithmeticLogicUnit alu;
    ControlUnit cu;

    int pc;
    int begin;
    int end;

public:
    Execute(int,string);
    void execute();
};
Execute::Execute(int start,string fileName)
{
    auto rangeOfPC = instrMem.loadInstructions(start,fileName);
    begin = rangeOfPC.first;
    end = rangeOfPC.second;
    pc = start;
    cache.initialize((16<<10),32,4);
}
void Execute::execute()
{
    int rs1 = -1, rs2 = -1, bpc = -1, ldRes = -1;

    while (begin <= pc && pc <= end)
    {
        // Instruction Fetch
        string instr = instrMem.getInstr(pc);
        cout << pc << " ";
        int npc = pc + 1;

        // Instruction Decode
        string imm1 = instr.substr(0, 12);
        string imm2 = instr.substr(20, 5);
        int funct7 = binSTI(instr,1,1); // Only using the 2nd bit from left side.
        int rsl2 = binSTI(instr,7,5);
        int rsl1 = binSTI(instr,12,5);
        int funct3 = binSTI(instr,17,3);
        int rdl = binSTI(instr,20,5);
        int opcode = binSTI(instr,25,7);

        cu.initialize(opcode);
        if (cu.regRead())
        {
            rs1 = regFile.readReg(rsl1);

            if (cu.aluSrc())
            {
                if (opcode == 35)
                {
                    if (imm1[0] == '1')
                        rs2 = -(1 << 11) + binSTI(imm1,1,6) + binSTI(imm2);
                    else
                        rs2 = binSTI(imm1,0,7) + binSTI(imm2);
                }
                else
                {
                    if (imm1[0] == '1')
                        rs2 = -(1 << 11) + binSTI(imm1,1);
                    else
                        rs2 = binSTI(imm1);
                }
            }
            else
                rs2 = regFile.readReg(rsl2);
        }

        // Execution
        alu.setInput1(rs1);
        alu.setInput2(rs2);
        cout << rs1 << " " << rs2 << endl;
        int aluSelect = cu.aluSelect(funct3, funct7, opcode);
        int aluRes = alu.performOperation(aluSelect);
        bool condition = alu.checkBoolean(funct3, aluRes);

        if (cu.branch() && condition)
        {
            if (imm1[0] == '1')
            {
                bpc = (-(1 << 11) + (binSTI(imm2,4,1) << 10) + (binSTI(imm1,1,6) << 4) + (binSTI(imm2,0,4) << 0)) / 2;
            }
            else
            {
                bpc = ((binSTI(imm2,4,1) << 10) + (binSTI(imm1,1,6) << 4) + (binSTI(imm2,0,4) << 0)) / 2;
            }
            pc = pc + bpc;
        }
        else
            pc = npc;

        // Memory Operations
        if (cu.memRead())
            ldRes = cache.readFromCache(aluRes);
        if (cu.memWrite())
            cache.writeIntoCache(aluRes, regFile.readReg(rsl2));

        // WriteBack Operations
        if (cu.regWrite())
        {
            if (cu.memToReg() == 1)
                regFile.writeReg(rdl, ldRes);
            else
                regFile.writeReg(rdl, aluRes);
        }
    }

    instrMem.closeInstrMem();
    cache.closeMem();
    regFile.displayFile();
}

int main()
{
    Execute e(800, "./Problems/GCDLCM.txt");
    e.execute();

    return 0;
}