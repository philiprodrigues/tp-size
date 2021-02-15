#include <algorithm>
#include <fstream>
#include <vector>
#include <iostream>

#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"
#include "TBranchElement.h"

#include "TPClasses.h"

struct BranchInfo
{
    std::string name;
    long realSize, zipSize;
    long nentries;
    
    bool operator<(const BranchInfo& rhs) const
    {
        return zipSize > rhs.zipSize;
    }
};

void printBranchSizes(TTree* t)
{
    TBranchElement* topbranch=(TBranchElement*)t->GetListOfBranches()->At(0);
    TObjArray* branches=topbranch->GetListOfBranches();
    TIter it(branches);

    std::vector<BranchInfo> branchInfos;
    
    TObject* obj;

    while(( obj=it() )){
        TBranch* b=(TBranch*)obj;
        BranchInfo bi;
        bi.name=b->GetName();
        bi.nentries=b->GetEntries();
        bi.realSize=b->GetTotBytes("*");
        bi.zipSize=b->GetZipBytes("*");

        branchInfos.push_back(bi);
    }
    BranchInfo bi;
    bi.name="Overall";
    bi.nentries=t->GetEntries();
    bi.realSize=t->GetTotBytes();
    bi.zipSize=t->GetZipBytes();

    branchInfos.push_back(bi);
    
    printf("       Uncompressed        |       Compressed          |  Factor    | Field\n");
    printf(" total (MB)  per-entry (B) | total (MB)  per-entry (B) |            |\n");
    printf("=====================================================================================\n");
    // std::sort(branchInfos.begin(), branchInfos.end());
    for(uint i=0; i<branchInfos.size(); ++i){
        if(i==branchInfos.size()-1) printf("------------------------------------------------------------------------------\n");
        const BranchInfo& bi=branchInfos[i];
        printf("   % 8.1f       % 8.1f |   % 8.1f       % 8.1f |  % 6.1f    | %s\n", bi.realSize/(1024.*1024),  bi.realSize/double(bi.nentries),
                                                                                    bi.zipSize/(1024.*1024), bi.zipSize/double(bi.nentries),
                                                                                    double(bi.realSize)/bi.zipSize, bi.name.c_str());
    }
    printf("\n\n");
}

int main(int argc, char** argv)
{
    int compression_level=-1;
    if(argc==2){
        compression_level=atoi(argv[1]);
        printf("Using compression level %d\n", compression_level);
    }
    else{
        printf("Using default compression level\n");
    }
    
    std::ifstream fin("/data/lar/dunedaq/rodrigues/hit-dumps/neutron-source-runs-2020-07-09/felix100_off.txt");
    std::vector<TPFromPTMP> ptmp_tps;
    TPFromPTMP ptmptp;
    while(fin >> ptmptp){
        ptmp_tps.push_back(ptmptp);
    }
    std::cout << "Read " << ptmp_tps.size() << " tps" << std::endl;
    TP* tp=nullptr;

    {
        printf("Default:\n\n");
        TFile f("default-tps.root", "RECREATE");
        if(compression_level>0) f.SetCompressionLevel(compression_level);
        TTree* t=new TTree("foo", "bar");
        t->Branch("tp", &tp);
        for(auto& p : ptmp_tps){
            *tp=p;
            t->Fill();
        }
        t->Write();

        printBranchSizes(t);
    }

    {
        printf("Store differences of time_start:\n\n");
        // Sort the TPs by time
        std::sort(ptmp_tps.begin(), ptmp_tps.end(), [](TPFromPTMP const& a, TPFromPTMP const& b) { return a.timestamp<b.timestamp; });
        tp=nullptr;
        
        TFile f("time-delta-tps.root", "RECREATE");
        if(compression_level>0) f.SetCompressionLevel(compression_level);
        TTree* t=new TTree("foo", "bar");
        t->Branch("tp", &tp);
        uint64_t prev_timestamp=0;
        for(auto& p : ptmp_tps){
            *tp=p;
            tp->time_start-=prev_timestamp;
            prev_timestamp=p.timestamp;
            t->Fill();
        }
        t->Write();

        printBranchSizes(t);
    }
}

// Local Variables:
// c-basic-offset: 4
// End:
