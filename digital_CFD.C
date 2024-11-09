#include <TFile.h>
#include <TTree.h>
#include <TArrayS.h>
#include <TH2D.h>
#include <iostream>
#include <vector>
#include <TCanvas.h>
#include <TMath.h>
#include <TString.h>


int riseTime(vector<unsigned short> wf){ // Calculate the rise time of a signal 
    float dTh = 5;
    float rt;
    float d[(int)(wf.size())];

    // Derivative
    d[0] = 0;
    for(int i=1;i<wf.size()-1;i++){
        d[i] = (wf[i+1] - wf[i-1])/2.;
    }

    // Calculate rise time
    int l0,l1 = 0;
    while(d[l0] > -dTh && l0 < wf.size()-1){
        l0++;
    }
    l1 = l0;
    while(d[l1] < -dTh && l1 < wf.size()-1){
        l1++;
    }
    return l1-l0;
}

int peakHeight(vector<unsigned short> wf){ // Calculate the peak height of a signal
    short min = wf[0];
    for(int i=1;i<wf.size()-10;i++){
        if(wf[i] < min){
            min = wf[i];
        }
    }
    return wf[0] - min;
}

int energy(vector<unsigned short> wf){ // Integrate the signal to 
    int sum = 0;
    for(int i=1;i<wf.size();i++){
        sum += wf[i] - wf[0];
    }
    return -sum;
}

vector<float> cfd(vector<unsigned short> wf, int D, float F){ // DIgital CFD function

    vector<float> nwf;

    for(int i=0;i<wf.size();i++){
        if(i<D){
            nwf.push_back( (float)(wf[i] - wf[0])*F);
        }
        else{
            nwf.push_back( (float)(wf[i] - wf[0])*F - (float)(wf[i-D] - wf[0]));
        }
    }

    return nwf;
}

float zcpFind(vector<float> wf0){ // digital TAC, from 2 cfd signals calculate the delta time
    
    float T0;
    int th = 20;

    // Find the zero crossing point of channel 0
    for(int i=0;i<wf0.size()-1;i++){
        if( wf0[i+1]*wf0[i] < 0 && wf0[i+1]-wf0[i] > th ){
            T0 = (float)i - wf0[i] / ( wf0[i+1] - wf0[i] );
            break;
        }
    }

    return T0;
}

void digital_CFD(bool Opt) {
    // Open the file containing the TTree
    TFile *file = TFile::Open("/home/valo/Desktop/G29_light_waves_RIGHT_1/RAW/DataR_G29_light_waves_RIGHT_1.root", "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error opening file." << std::endl;
        return;
    }

    // Get the TTree
    TTree *tree = (TTree*)file->Get("Data_R");  // Replace "tree_name" with the name of your TTree
    if (!tree) {
        std::cerr << "Error: TTree not found in file." << std::endl;
        file->Close();
        return;
    }

    // Create a pointer to hold the entries of the TTree
    TArrayS *samples = nullptr;
    ULong64_t timest;
    unsigned short channel;
    tree->SetBranchAddress("Timestamp", &timest);
    tree->SetBranchAddress("Channel", &channel);
    tree->SetBranchAddress("Samples", &samples);

    int ent0[(int)(tree->GetEntries()/4)];
    int ent1[(int)(tree->GetEntries()/4)];

    int i0,i1 = 0;

    // Load the waveforms inside vectors

    vector<vector<unsigned short>> wf0;
    vector<vector<unsigned short>> wf1;

    std::cout << '\n';

    for(int i=0;i<tree->GetEntries();i++){
        
        tree->GetEntry(i);
        if(channel==0){
            vector<unsigned short> wApp;
            for(int j=0;j<samples->GetSize();j++){
                wApp.push_back(samples->At(j));
            }
            wf0.push_back(wApp);
        } else if(channel==1){
            vector<unsigned short> wApp;
            for(int j=0;j<samples->GetSize();j++){
                wApp.push_back(samples->At(j));
            }
            wf1.push_back(wApp);
        }
        
        if(i%1000==0){
            std::cout << "\rLoading event: " << 100*i/tree->GetEntries() <<"%   ";
        }
    }

    std::cout << "\rThe root file contains: " << tree->GetEntries()/4 << " entries\n";

    // Build histograms of the wave form parameters

    TH1F *spec0 = new TH1F("Energy_ch0","Energy spectrum CH0", 250, 0, 25000);
    TH1F *spec1 = new TH1F("Energy_ch1","Energy spectrum CH1", 250, 0, 25000);
    TH1F *rTime0 = new TH1F("RT_ch0","Rising times CH0", 10, 0.5, 10.5);
    TH1F *rTime1 = new TH1F("RT_ch1","Rising times CH1", 10, 0.5, 10.5);
    TH1F *pkHt0 = new TH1F("PH_ch0","Peack heights CH0", 50, 0, 1000);
    TH1F *pkHt1 = new TH1F("PH_ch1","Peack heights CH1", 50, 0, 1000);
    TH1F *dTimes = new TH1F("dT","Delta times", 200, 8, 18);

    // Set the digital CFD parameters

    int D = 6;
    float F = 0.2;

    for(int i=0;i<tree->GetEntries()/4;i++){
        float t0,t1;

        spec0->Fill(energy(wf0[i]));
        rTime0->Fill(riseTime(wf0[i]));
        pkHt0->Fill(peakHeight(wf0[i]));
        t0 = zcpFind(cfd(wf0[i],D,F));

        spec1->Fill(energy(wf1[i]));
        rTime1->Fill(riseTime(wf1[i]));
        pkHt1->Fill(peakHeight(wf1[i]));
        t1 = zcpFind(cfd(wf1[i],D,F));

        dTimes->Fill(t0-t1);

        if(i%100==0){
            std::cout << "\rBuilding histograms: " << 100*i/(tree->GetEntries()/4) <<"%";
        }
    }

    std::cout<<"\rHistograms built!              \n";

    // Choose a specific event to draw its waveform

    int wfId = 0;

    // Build the histogram
    TH1F *wf_ch0 = new TH1F("wf_ch0","Wave profile CH0", wf0[wfId].size(), 0, wf0[wfId].size());

    for (int i = 0; i < wf0[wfId].size(); i++) {
        wf_ch0->SetBinContent(i, wf0[wfId][i]);
    }

    TH1F *wf_ch0_cfd = new TH1F("wf_ch0_cfd","CFD profile CH0 ", wf0[wfId].size(), 0, wf0[wfId].size());

    for (int i = 0; i < wf0[wfId].size(); i++) {
        wf_ch0_cfd->SetBinContent(i, cfd(wf0[wfId],D,F)[i]);
    }

    tree->GetEntry(ent1[wfId]);

    TH1F *wf_ch1 = new TH1F("wf_ch1","Wave profile CH1", wf1[wfId].size(), 0, wf1[wfId].size());

    for (int i = 0; i < wf1[wfId].size(); i++) {
        wf_ch1->SetBinContent(i, wf1[wfId][i]);
    }

    wf_ch0->SetLineColor(kRed);  // Set the line color of the histogram to red
    wf_ch1->SetLineColor(kBlue); // Set the fill color of the histogram to blue

    TFile* oFile = new TFile("/home/valo/Desktop/Histograms.root", "RECREATE");

    spec0->Write();
    spec1->Write();
    rTime0->Write();
    rTime1->Write();
    pkHt0->Write();
    pkHt1->Write();
    dTimes->Write();
    wf_ch0->Write();
    wf_ch1->Write();

    if(Opt){
        // Search for the optimal values of the digital CFD:
        int Dmin = 4;
        int Dmax = 7;
        float Fmin = 0.1;
        float Fmax = 0.4;
        float dF = 0.05;

        vector<vector<float>> sig(Dmax-Dmin, vector<float> ((int)((Fmax-Fmin)/dF), 0.));
        vector<vector<float>> stdev(Dmax-Dmin, vector<float> ((int)((Fmax-Fmin)/dF), 0.));
        vector<vector<float>> qval(Dmax-Dmin, vector<float> ((int)((Fmax-Fmin)/dF), 0.));

        TH2D* sHm = new TH2D("sHm", "Gaussian sigma heatmap", (int)((Fmax-Fmin)/dF), Fmin, Fmax, Dmax-Dmin, Dmin, Dmax);
        TH2D* sdHm = new TH2D("sdHm", "Standard deviation heatmap", (int)((Fmax-Fmin)/dF), Fmin, Fmax, Dmax-Dmin, Dmin, Dmax);
        TH2D* qHm = new TH2D("qHm", "Quality heatmap", (int)((Fmax-Fmin)/dF), Fmin, Fmax, Dmax-Dmin, Dmin, Dmax);

        TH1F *dTa = new TH1F("dTa","Delta times", 200, 8, 18);
        TCanvas* ctemp = new TCanvas("ctemp", "2D Heatmap", 800, 600);

        for(int i=0;i<Dmax-Dmin;i++){
            for(int j=0;j<(int)((Fmax-Fmin)/dF);j++){
                dTa->Reset();
                TString histName = TString::Format("histogram_%d", i);
                TString histTitle = TString::Format("Histogram %d", i);

                // Create the histogram
                TH1F *hist = new TH1F(histName, histTitle, 100, 0, 10);
                for(int k=0;k<tree->GetEntries()/4;k++){
                    float t0,t1;
                    t0 = zcpFind(cfd(wf0[k],Dmin + i,Fmin + j*dF));
                    t1 = zcpFind(cfd(wf1[k],Dmin + i,Fmin + j*dF));

                    dTa->Fill(t0-t1);
                }

                TF1 *gausFit = new TF1("gausFit", "gaus", 8, 18); // Adjust the fit range as needed
                dTa->Fit(gausFit, "Q");
                sig[i][j] = gausFit->GetParameter(2);
                stdev[i][j] = dTa->GetStdDev();
                qval[i][j] = gausFit->GetChisquare() / gausFit->GetNDF();
                std::cout << "D = " << Dmin + i << "; F = " << Fmin + j*dF << "; sig = " << sig[i][j] << "; stdev = " << stdev[i][j] << "; q = " << qval[i][j] << '\n';
                dTa->Draw();
                gausFit->Draw("SAME");
                ctemp->Update();
            }
        }

        for (int row = 0; row < Dmax-Dmin; ++row) {
            for (int col = 0; col < (int)((Fmax-Fmin)/dF); ++col) {
                sHm->SetBinContent(col + 1, row + 1, TMath::Log(sig[row][col]));
                sdHm->SetBinContent(col + 1, row + 1, TMath::Log(stdev[row][col]));
                qHm->SetBinContent(col + 1, row + 1, TMath::Log(qval[row][col]));
            }
        }
        sHm->Write();
        sdHm->Write();
        qHm->Write();
    }
}