#include <bits/stdc++.h>

using namespace std;

struct Instruction
{
    string f3, f7, opcode, rs1, rs2, rd, instr;
};

class Machine
{
    const unordered_set<string> r_type = {"ADD", "SUB", "AND", "OR"};
    const unordered_set<string> i_type = {"ADDI", "ANDI", "ORI"};
    const unordered_set<string> l_type = {"LW"};
    const unordered_set<string> s_type = {"SW"};
    const unordered_set<string> b_type = {"BEQ", "BGE", "BLT", "BNE"};
    const unordered_set<string> j_type = {"JAL"};

    unordered_map<string, string> f7_map =
        {
            {"ADD", "0000000"},
            {"SUB", "0100000"},
            {"AND", "0000000"},
            {"OR", "0000000"}};

    unordered_map<string, string> f3_map =
        {
            {"ADD", "000"},
            {"SUB", "000"},
            {"AND", "111"},
            {"OR", "110"},

            {"ADDI", "000"},
            {"ANDI", "111"},
            {"ORI", "110"},
            {"LW", "010"},
            {"SW", "010"},

            {"BEQ", "000"},
            {"BGE", "101"},
            {"BLT", "100"},
            {"BNE","001"}};
    unordered_map<char, string> opcode_map =
        {
            {'R', "0110011"},
            {'I', "0010011"},
            {'L', "0000011"},
            {'S', "0100011"},
            {'B', "1100011"},
            {'J', "1101111"},
    };

    Instruction I;
    char status;
    vector<string> machine_code;
    queue<string> q;

public:
    void helperFunction(vector<int> &);
    string pushRType();
    string pushIType();
    string pushLType();
    string pushSType();
    string pushBType();
    string pushJType();

    void pushInstruction();
    bool isInstruction(string);
    void pushIntoQueue(string);

    void convertToMachineCode(string);
    void printMachineCode();
    vector<string> getMachineCode();
};

bool Machine::isInstruction(string s)
{
    if (r_type.find(s) != r_type.end() || b_type.find(s) != b_type.end() || s_type.find(s) != s_type.end() || i_type.find(s) != i_type.end() || j_type.find(s) != j_type.end() || l_type.find(s) != l_type.end())
        return true;

    return false;
}
string Machine::pushRType()
{
    status = 'R';
    vector<int> val;
    helperFunction(val);

    I.rd = bitset<5>(val[0]).to_string();
    I.rs1 = bitset<5>(val[1]).to_string();
    I.rs2 = bitset<5>(val[2]).to_string();

    I.instr = I.f7 + I.rs2 + I.rs1 + I.f3 + I.rd + I.opcode;
    return I.instr;
}
string Machine::pushIType()
{
    status = 'I';
    string imm;
    vector<int> val;

    helperFunction(val);

    I.rd = bitset<5>(val[0]).to_string();
    I.rs1 = bitset<5>(val[1]).to_string();
    imm = bitset<12>(val[2]).to_string();
    I.instr = imm + I.rs1 + I.f3 + I.rd + I.opcode;
    return I.instr;
}
string Machine::pushBType()
{
    status = 'B';
    string imm;
    vector<int> val;

    helperFunction(val);

    I.rs2 = bitset<5>(val[0]).to_string();
    I.rs1 = bitset<5>(val[1]).to_string();
    imm = bitset<12>(val[2] / 2).to_string();
    I.instr = imm.substr(0, 1) + imm.substr(2, 6) + I.rs2 + I.rs1 + I.f3 + imm.substr(8, 4) + imm.substr(1, 1) + I.opcode;

    return I.instr;
}
string Machine::pushLType()
{
    status = 'L';
    string imm;
    vector<int> val;

    helperFunction(val);

    I.rd = bitset<5>(val[0]).to_string();
    I.rs1 = bitset<5>(val[2]).to_string();
    imm = bitset<12>(val[1]).to_string();
    I.instr = imm + I.rs1 + I.f3 + I.rd + I.opcode;

    return I.instr;
}
string Machine::pushSType()
{
    status = 'S';
    string imm;
    vector<int> val;

    helperFunction(val);

    I.rs2 = bitset<5>(val[0]).to_string();
    I.rs1 = bitset<5>(val[2]).to_string();
    imm = bitset<12>(val[1]).to_string();
    I.instr = imm.substr(0, 7) + I.rs2 + I.rs1 + I.f3 + imm.substr(7, 5) + I.opcode;

    return I.instr;
}
string Machine::pushJType()
{
    status = 'J';
    string imm;
    vector<int> val;

    helperFunction(val);

    I.rd = bitset<5>(val[0]).to_string();
    imm = bitset<20>(val[1]).to_string();
    I.instr = imm + I.rd + I.opcode;

    return I.instr;
}
void Machine::pushIntoQueue(string s)
{
    q.push(s);
}
void Machine::pushInstruction()
{
    if (q.empty())
        return;

    string res;
    string s = q.front();

    if (r_type.find(s) != r_type.end())
        res = pushRType();
    else if (i_type.find(s) != i_type.end())
        res = pushIType();
    else if (b_type.find(s) != b_type.end())
        res = pushBType();
    else if (s_type.find(s) != s_type.end())
        res = pushSType();
    else if (j_type.find(s) != j_type.end())
        res = pushJType();
    else if (l_type.find(s) != l_type.end())
        res = pushLType();
    else
    {
        while (!q.empty())
            q.pop();
        return;
    }

    machine_code.push_back(res);

    while (!q.empty())
        q.pop();
}
void Machine::helperFunction(vector<int> &val)
{
    bool breakFlag = 0;
    string s = q.front();
    q.pop();

    I.f7 = f7_map[s];
    I.f3 = f3_map[s];
    I.opcode = opcode_map[status];

    string curr_val = "";
    while (!q.empty())
    {
        string s = q.front();
        q.pop();

        size_t pos = s.find("//");
        if (pos != string::npos)
        {
            breakFlag = 1;
            s = s.substr(0, pos);
        }
        for (const auto x : s)
        {
            if (x == ',' || x == '(')
            {
                if (curr_val.size() && curr_val[0] == '-')
                    val.push_back(-stoi(curr_val.substr(1)));
                else
                    val.push_back(stoi(curr_val));
                curr_val = "";
            }

            else if (x != 'x' && x != ')')
                curr_val = curr_val + x;
        }
        if (breakFlag)
            break;
    }
    if (curr_val.size() && curr_val[0] == '-')
        val.push_back(-stoi(curr_val.substr(1)));
    else
        val.push_back(stoi(curr_val));
    curr_val = "";
}
void Machine::convertToMachineCode(string fileName)
{
    ifstream in;
    string str;

    in.open(fileName);
    while (in.eof() == 0)
    {
        in >> str;
        if (isInstruction(str))
            pushInstruction();
        pushIntoQueue(str);
    }
    pushInstruction();
    in.close();
}
void Machine::printMachineCode()
{
    ofstream out("machine.txt", ios::app);

    if (!out)
    {
        cout << "ERROR!" << endl;
        return;
    }

    for (const auto &x : machine_code)
    {
        out << x << endl;
    }
    out.close();
}
vector<string> Machine::getMachineCode()
{
    return machine_code;
}
