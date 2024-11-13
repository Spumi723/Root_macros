#include <TCanvas.h>
#include <TH2F.h>
#include <TColor.h>
#include <TRandom.h>
#include <TStyle.h>
#include <TMath.h>
#include <iostream>

void hist2d() {
    // Access the file
    TFile *file = TFile::Open("/home/valo/Desktop/Histograms.root", "READ");

    // Create a canvas
    TCanvas *c = new TCanvas("c", "2D Histogram with Custom Colormap", 800, 600);

    // Retrieve the 2D histogram
    TH2D *hist = dynamic_cast<TH2D*>(file->Get("sdHm"));

    // Define the color gradient for a smoother colormap
    const Int_t numColors = 200;  // Increase this number for a finer gradient
    Int_t colors[numColors];

    // Define stops and colors for the gradient (from blue to red through green)
    Double_t stops[] = {0.00, 0.05, 0.15, 1.00};      // Position of colors in the gradient
    Double_t red[]   = {1.00, 1.00, 0.00, 0.00};      // Red component for each stop  // Yellow component for each step
    Double_t green[] = {0.00, 1.00, 1.00, 0.00};      // Green component for each stop
    Double_t blue[]  = {0.00, 0.00, 0.00, 1.00};      // Blue component for each stop

    // Create the gradient color table with the specified stops
    Int_t colorBase = TColor::CreateGradientColorTable(4, stops, red, green, blue, numColors);

    for(int i=0;i<55;i++){
        for(int j=0;j<7;j++){
            double app = hist->GetBinContent(i+1,j+1);
            hist->SetBinContent(i+1,j+1,TMath::Exp(app));
        }
    }

    // Fill the color array with the generated colors
    for (int i = 0; i < numColors; i++) {
        colors[i] = colorBase + i;
    }

    // Set the color palette for the canvas
    gStyle->SetPalette(numColors, colors);

    // Set number of contour levels for a smoother appearance
    gStyle->SetNumberContours(numColors);

    // Draw the histogram with "COLZ" to enable color mapping
    hist->Draw("COLZ");

    // Draw the canvas
    c->Draw();
}
