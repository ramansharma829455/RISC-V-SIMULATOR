#include <bits/stdc++.h>

using namespace std;

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

    void displayBlock(int tagPlusIndex)
    {
        cout << st << " " << tag << " " << counter << endl;
        for (int i = 0; i < data.size(); i++)
        {
            cout << tagPlusIndex + i << " " << data[i] << endl;
        }
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
    Cache(int cacheSize, int blockSize, int way);
    int readFromCache(int address);
    void writeIntoCache(int address, int data);
    int evictionAlgorithm(vector<CacheBlock> &currBlocks);
    CacheBlock selectBlock(CacheBlock, int, int, int, int);
    void closeMem();
};
Cache::Cache(int cacheSize, int blockSize, int way)
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

    for (int i = 0; i < way; i++)
        currBlocks[i].displayBlock(((currBlocks[i].tag) * numberOfSets + reqIndex) * numberPerBlock);
    cout << endl;

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

    for (int i = 0; i < way; i++)
        currBlocks[i].displayBlock(((currBlocks[i].tag) * numberOfSets + reqIndex) * numberPerBlock);
    cout << endl;
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

    if(address!=-1)
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

int main()
{
    Cache cache((16 << 10), 32, 4);
    cache.writeIntoCache(4, 40000);
    cout << cache.readFromCache(4) << endl;
    cache.writeIntoCache(8, 80000);
    cache.closeMem();
    return 0;
}