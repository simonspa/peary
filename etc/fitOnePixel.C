{

  TFile * outf = new TFile("noise_curves.root", "RECREATE");

  TH1D * means  = new TH1D("means", "", 16, 0, 15);

  Double_t trim[16], mean[16], width[16];
  Int_t n = 16;

  int r = 13, c = 13;
  bool longc = false;

  std::stringstream p;
  p << "Pixel " << c << "," << r;

  for(int i = 0; i < 16; i++) {

    std::stringstream h;
    h << setfill('0') << setw(2) << c << "_"
      << setfill('0') << setw(2) << r << "_trim"
      << setfill('0') << setw(2) << i;

    std::stringstream s;
    s << "thrscan_LSB_px" << h.str() << ".csv";

    std::string name = "noise_" + h.str();
    TH1D* pixel_scurve = new TH1D(name.c_str(),"", 256, 0, 255);
    
    std::ifstream pxfile(s.str());
    if(!pxfile.is_open()) {
      std::cout << "Could not open matrix file \"" << s.str() << "\"" << std::endl;
      continue;
    }
    else {
      std::cout << "Open matrix file \"" << s.str() << "\"" << std::endl;
    }

    std::string line = "";
    while(std::getline(pxfile, line)) {
      if(!line.length() || '#' == line.at(0))
	continue;

      std::stringstream ss(line);
      std::string value;
      int thr, column, row, flag, cnt, longcnt;

      int i = 0;
      while(getline(ss,value,',')) {
	if(i == 0) thr = atoi(value.c_str());
	else if(i == 1) column = atoi(value.c_str());
	else if(i == 2) row = atoi(value.c_str());
	else if(i == 3) flag = atoi(value.c_str());
	else if(i == 4) cnt = atoi(value.c_str());
	else if(i == 5) longcnt = atoi(value.c_str());
	i++;
      }

      // Proper entry
      if(i == 5) {
	longc = true;
	if(row != r || column != c) continue;
	pixel_scurve->SetBinContent(thr,cnt);
      }
      else if(i == 6) {
	longc = false;
	if(row != r || column != c) continue;
	pixel_scurve->SetBinContent(thr,longcnt);
      }

    }

    pixel_scurve->GetXaxis()->SetTitle("threshold_LSB");
    
    if(longc) pixel_scurve->GetYaxis()->SetTitle("pixel counter 13-bit");
    else pixel_scurve->GetYaxis()->SetTitle("pixel counter 8-bit");

    pixel_scurve->SetTitle(h.str().c_str());


    pixel_scurve->Fit("gaus");

    TF1 *fit1 = pixel_scurve->GetFunction("gaus");
    trim[i] = i;

    double m  = fit1->GetParameter(1);
    double w = fit1->GetParameter(2);

    mean[i] = m;
    width[i] = w;
    
    std::cout << fit1->GetParameter(0) << ", "
	      << fit1->GetParameter(1) << ", "
	      << fit1->GetParameter(2) << std::endl;
    std::cout << "Mean: " << trim[i] << endl;

    TCanvas *c1 = new TCanvas("c","",0,0,700,700);
    pixel_scurve->Draw();
    fit1->Draw("SAME");

    std::string pname = name + ".png";
    c1->Print(pname.c_str());
  }

  TCanvas *canv = new TCanvas("c","",0,0,700,700);
  TGraph* gr = new TGraph(n,trim,mean);
  std::string ti = p.str();
  gr->SetTitle(ti.c_str());
  gr->GetXaxis()->SetTitle("trim bits");
  gr->GetYaxis()->SetTitle("mean");
  gr->Draw("AC*");

  TCanvas *canv2 = new TCanvas("c2","",0,0,700,700);
  TGraph* gr2 = new TGraph(n,trim,width);
  gr2->SetTitle(ti.c_str());
  gr2->GetXaxis()->SetTitle("trim bits");
  gr2->GetYaxis()->SetTitle("width");
  gr2->Draw("AC*");

  outf->Write();

}
