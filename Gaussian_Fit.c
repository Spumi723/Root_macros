#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>
#include <iostream>

void Gaussian_Fit(const char *filename, float gmin, float gmax) {
    // Open the ROOT file containing the TTree
    TFile *file = TFile::Open(filename); // Replace "your_file.root" with the actual file name
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Could not open the file!" << std::endl;
        return;
    }

    // Get the TTree from the file
    TTree *tree = (TTree*)file->Get("Data_R"); // Replace "your_tree" with the actual TTree name
    if (!tree) {
        std::cerr << "Error: Could not find the TTree in the file!" << std::endl;
        return;
    }

    // Define variables to hold branch values
    unsigned short energy;
    unsigned short channel;

    // Set branch addresses
    tree->SetBranchAddress("Energy", &energy);   // Replace "energy" with the actual energy branch name
    tree->SetBranchAddress("Channel", &channel); // Replace "channel" with the actual channel branch name

    // Create a histogram for the energy values in channel 2
    TH1F *hEnergy = new TH1F("hEnergy", "Energy Distribution for Channel 2", 40, 10, 50); // Adjust the binning as needed

    // Loop over the entries in the tree
    Long64_t nEntries = tree->GetEntries();
    for (Long64_t i = 0; i < nEntries; i++) {
        tree->GetEntry(i);
        
        // Only fill the histogram if the channel number is 2
        if (channel == 2) {
            hEnergy->Fill(energy);
        }
    }

    // Create a canvas to display the histogram
    TCanvas *c1 = new TCanvas("c1", "Energy Histogram with Gaussian Fit", 800, 600);
    hEnergy->Draw();

    TF1 *gausWithOffset = new TF1("gausWithOffset", "[0] + [1]*exp(-0.5*((x-[2])/[3])**2)", gmin, gmax);
    

    // Fit a Gaussian function to the histogram
    TF1 *gausFit = new TF1("gausFit", "gaus", gmin, gmax); // Adjust the fit range as needed
    hEnergy->Fit(gausFit, "R");

    // Print fit parameters
    std::cout << "Gaussian Fit Parameters:" << std::endl;
    std::cout << "Mean = " << gausFit->GetParameter(1) << "+-" << gausFit->GetParError(1) << std::endl;
    std::cout << "Sigma = " << gausFit->GetParameter(2) << "+-" << gausFit->GetParError(2) << std::endl;

    // Update the canvas to show the fit
    
    c1->Update();
}
