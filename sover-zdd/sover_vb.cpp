#include "tdzdd/DdSpec.hpp"
#include "tdzdd/DdSpecOp.hpp"
#include "tdzdd/DdStructure.hpp"
#include "tdzdd/DdEval.hpp"
#include <iostream>
#include <vector>
using namespace std;

const int MAX_ITEM_NUM = 8;
const int NODE_NUM = 8 * MAX_ITEM_NUM + 1;

const int MOD = 1e9 + 7;
const int MAX_WIDTH = MAX_ITEM_NUM;
const int MAX_HEIGHT = 5;
const int MAX_BITSET_SIZE = (MAX_HEIGHT + 1) * MAX_WIDTH;

long long modpow(long long a, long long n) {
    long long res = 1;
    while(n > 0){
        if(n & 1) res = res * a % MOD;
        a = a * a % MOD;
        n >>= 1;
    }
    return res;
}

class Mino{
    int height; 
    int width;
    int id;
    vector<vector<bool> > mino;

public:
    //初期化
    Mino(){
        height = 0;
        width = 0;
        id = -1;
        vector<vector<bool> > mino;
    }
    Mino(int h, int w, int id,  vector<vector<bool> > mino ) : height(h), width(w), id(id), mino(mino){
    }

    //左に回転
    Mino rotate(){

        //vector<string> newMino(width, string(height, '.'));
        vector<vector<bool> > newMino(width, vector<bool>(height));

        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                newMino[width - j - 1][i] = mino[i][j];
            }
        }
        return Mino(width, height, id, newMino);
    }

    //i回左に回転
    Mino rotate(int i){
        Mino newMino = Mino(height, width, id, mino);
        i %= 4;
        while(i){
            newMino = newMino.rotate();
            i--;
        }
        return newMino;
    }

    //反転する
    Mino flip(){
        vector<vector<bool> > newMino = mino;
        for(int i = 0; i < height; i++){
            reverse(newMino[i].begin(), newMino[i].end());
        }
        return Mino(height, width, id, newMino);
    }

    bool get(int x, int y){
        return mino[x][y];
    }

    int getHeight(){
        return height;
    }

    int getWidth(){
        return width;
    }

    int getId(){
        return id;
    }

    void print(){
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                cout << mino[i][j];
            }
            cout << endl;
        }
    }
};

class Katamino{
    int height;
    int width;
    vector<vector<bool> > board;
    int itemNum;
    vector<bool> used;//ピースが使われたか
    int count;

public:
    Katamino(){
        height = 0; 
        width = 0;
        itemNum = 0;
        count = 0;
    }

    Katamino(int h, int w, int n): height(h), width(w), itemNum(n){
        //board.assign(h, string(w, '.'));
        board.assign(h, vector<bool>(w, 0));

        used.assign(n, false);
        count = 0;
    }

    //ボードの初期化
    void init(int h, int w, int n){
        height = h;
        width = w;
        itemNum = n;
        count = 0;
        //board.assign(h, string(w, '.'));

        //ボード配列
        board.assign(h, vector<bool>(w, 0));
        used.assign(n, false);
    }

    //ボードの大きさを越えているか
    bool hasOverlapOutside(int x, int y, Mino m){
        if(m.getHeight() + x > height) return true;
        if(m.getWidth() + y > width) return true;
        return false;
    }

    //既に置かれているか
    bool hasOverlap(int x, int y, Mino m){
        for(int i = 0; i < m.getHeight(); i++){
            for(int j = 0; j < m.getWidth(); j++){
                if(m.get(i, j) && board[x+i][y+j]){
                    return true;
                }
            }
        }

        return false;
    }

    //ピースを置く
    bool addMino(int x, int y, Mino m){

        if(used[m.getId()]) return false;
        if(hasOverlapOutside(x, y, m)) return false;
        if(hasOverlap(x, y, m)) return false;

        for(int i = 0; i < m.getHeight(); i++){
            for(int j = 0; j < m.getWidth(); j++){
                if(m.get(i, j)){
                    board[x + i][y + j] = true;
                }
            }
        }

        count++;
        used[m.getId()] = true;
        return true;
    }

    int getHeight() const {
        return height;
    }

    int getWidth() const {
        return width;
    }

    int getItemNum() const {
        return itemNum;
    }

    //(x,y)が既に埋まっているかどうか
    bool isCovered(int x, int y) const {
        assert(0 <= x && x < height);
        assert(0 <= y && y < width);
        return (board[x][y]);
    }

    bool isUsed(int item) const {
        return used[item];
    }

    int countMinos() const {
        return count;
    }

    void print(){
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                cout << board[i][j];
            }
            cout << endl;
        }
    }

    
    //boardの状態を文字列に変換
    string toString() const {
        string r = "";
        for(int i = 0; i < height; i++){
            for(int j = 0; j < width; j++){
                if(board[i][j]) r += '#';
                else r += '.'; 
            }
            r += "\n";
        }
        return r;
    }
};

std::ostream& operator<<(std::ostream& o, const Katamino& state){
    o << state.toString();
    return o;
}

class KataminoZDD: public tdzdd::DdSpec <KataminoZDD, Katamino, NODE_NUM>{
    int itemNum;
    int height;
    int width;
    vector<Mino> minos;

public:
    KataminoZDD(int n, int h, int w, vector<Mino> minos): itemNum(n), height(h), width(w), minos(minos){

    }

    bool equalTo(Katamino const& l, Katamino const& r) const{
        for(int i = 0; i < l.getItemNum(); ++i){
            if(l.isUsed(i) != r.isUsed(i)) return false;
        }
        for(int i = 0; i < l.getHeight(); ++i){
            for(int j = 0; j < l.getWidth(); ++j){
                if(l.isCovered(i, j) != r.isCovered(i, j)) return false;
            }
        }
        return true;
    }

    size_t hashCode(Katamino const& k) const{
        size_t code = 0;
        for(int i = 0; i < itemNum; ++i) if(k.isUsed(i)) code = (code + modpow(3, i * itemNum)) % MOD;
        
        for(int i = 0; i < k.getHeight(); ++i){
            for(int j = 0; j < k.getWidth(); ++j){
                if(k.isCovered(i, j)) code = (code + modpow(2, i * k.getWidth() + j)) % MOD;
            }
        }
        return code;
    }

    int getRoot(Katamino& katamino) const {
        katamino.init(height, width, itemNum);
        return height * width;
    }

    int getChild(Katamino& katamino, int level, int value) const {

        //levelからピースを置く場所(x,y)の決定
        int maxlevel = height * width;
        int x = (maxlevel - level) % height;
        int y = (maxlevel - level) / height;

        if(value != 0){
            if(value > 8 * itemNum) return 0;
            int itemItr = (value - 1) / 8;
            int itemDir = (value - 1) % 8;

            Mino m = minos[itemItr];

            //枝刈り1-ピースmが既に使われている
            if(katamino.isUsed(m.getId())) return 0;

            if(itemDir >= 4) m = m.flip();
            m = m.rotate(itemDir);
            //枝刈り2-ピースmを(x, y)に置いたときボードの枠を超える
            if(katamino.hasOverlapOutside(x, y, m)) return 0;

            //枝刈り3-(x,y)に既にピースが置かれている
            if(katamino.hasOverlap(x, y, m)) return 0;

            //(x, y)にピースmを置く
            katamino.addMino(x, y, m); 

            //if(!katamino.isCovered(x,y)) return 0;

        }

        //何も埋まらなかった場合に枝刈り
        if(!katamino.isCovered(x,y)) return 0;
        if(level == 1){
            if(katamino.countMinos() == itemNum){
                return -1;
            }
            return 0;
        }

        level--;
        return level;
    }
};





int main(int argc, char **argv){
    int width, height;
    cin >> width >> height;

    int itemNum;
    cin >> itemNum;

    vector<Mino> items(itemNum);
    for(int t = 0; t < itemNum; ++t){
        int w, h;
        cin >> w >> h;

        //vector<vector<int> > mino(h);
        vector<vector<bool> > mino(h, vector<bool>(w, 0));

    
        for(int i = 0; i < h; ++i){
            for(int j = 0; j < w; ++j){
                char x;
                cin >> x;
                
                if(x == '#') mino[i][j] = true;
            }
        }
        
        Mino item(h, w, t, mino);
        items[t] = item;
    }
    
    KataminoZDD zdd(itemNum, height, width, items);
    tdzdd::DdStructure<NODE_NUM> dd(zdd);
    dd.zddReduce();

    std::cout << dd.zddCardinality() << ", size:" << dd.size() << std::endl;

    if(argc > 1 && argv[1] == std::string("p")){
        std::ofstream out("testvec.dot");
        zdd.dumpDot(out);
        out.close();
    }

}

