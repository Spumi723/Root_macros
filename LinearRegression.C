#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "TGraphErrors.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStyle.h"

void LinearRegression(const char* filename) {
    // Vectors to store data from the .csv file
    std::vector<double> xValues, yValues, yErrors;

    // Open the CSV file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    // Read the CSV file line by line
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        double x, y, yErr;

        // Read the x, y, and y-error values from the line
        if (ss >> x) {
            if (ss.peek() == ',') ss.ignore();
            if (ss >> y) {
                if (ss.peek() == ',') ss.ignore();
                if (ss >> yErr) {
                    // Store values in vectors
                    xValues.push_back(x);
                    yValues.push_back(y);
                    yErrors.push_back(yErr);
                }
            }
        }
    }
    file.close();

    // Check if data was loaded
    int n = xValues.size();
    if (n == 0) {
        std::cerr << "No data found in the file." << std::endl;
        return;
    }

    // Create TGraphErrors for x, y, and y-errors
    TGraphErrors *graph = new TGraphErrors(n, &xValues[0], &yValues[0], nullptr, &yErrors[0]);
    graph->SetTitle("TAC calibration;D [ns];m");
    graph->SetMarkerStyle(5);  // Example: 21 is a square marker
    graph->SetMarkerSize(2);  // Optionally, adjust the marker size
    graph->SetMarkerColor(kBlue);


    // Define a linear fit function
    TF1 *linearFit = new TF1("linearFit", "[0] + [1]*x", xValues.front(), xValues.back());
    linearFit->SetParameters(0, 1);  // Initial guesses for intercept and slope

    // Perform the linear fit
    graph->Fit(linearFit, "Q");  // "Q" option suppresses fit output on the console

    // Draw the graph and the fit result
    TCanvas *c1 = new TCanvas("c1", "Linear Regression", 800, 600);
    graph->Draw("AP");
    linearFit->Draw("same");

    // Print the fit results (slope and intercept with errors)
    double intercept = linearFit->GetParameter(0);
    double slope = linearFit->GetParameter(1);
    double interceptError = linearFit->GetParError(0);
    double slopeError = linearFit->GetParError(1);
    std::cout << "Fit Results:\n";
    std::cout << "Intercept = " << intercept << " ± " << interceptError << std::endl;
    std::cout << "Slope = " << slope << " ± " << slopeError << std::endl;
}
