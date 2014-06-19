#pragma once

#include "utils.h"

namespace recognition 
{

// DPマッチング用クラス(XY座標のみ)
class DP {
    
 private:
    std::vector<Signature> RefList;
    Signature Test;
	std::vector<int **> RefShapeContextList;
	int **TestShapeContext;
    int MinID;
    double *DPDistanceList;
 public:
    DP();
    ~DP();

    // リファレンス(ref)とテスト(test)をマッチングさせてDP距離を返す
    static double Match(const Signature &ref, const Signature &test);
    static double LocalDistance(const Signature &ref, const Signature &test, int i, int j, const int prev_match[]=NULL);
    static double LocalDistance_ED(const Signature &ref, const Signature &test, int i, int j);
    static double LocalDistance_DV(const Signature &ref, const Signature &test, int i, int j);
    static double LocalDistance_DPW(const Signature &ref, const Signature &test, int i, int j, const int prev_match[]);
    static double LocalDistance_SC(const Signature &ref, const Signature &test, int i, int j);
    
    // ファイルリストのファイル全てを取り込む
    void LoadRefFiles(const std::vector<std::string> &path_list);
    
    // クラス内部変数に test をセットする
    void LoadTest(Signature test);

    // 取り込んだリファレンス全てとマッチングを行い結果を内部変数に格納する
    void MatchAll();

    // マッチング結果からDP距離が最小となるIDを返す
    int GetMinID();

    // マッチング結果からID番目リファレンスとテストとのDP距離を返す
    double GetDPDistance(int refdata_id);

};


} // end-namespace
