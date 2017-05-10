{

  TCanvas *c = new TCanvas("c","",0,0,700,700);
  TCanvas *c2 = new TCanvas("c2","",0,0,700,700);
  gStyle->SetPalette(1);
  TH2D * h  = new TH2D("h", "", 128, 0, 128, 128, 0, 128);
  TH1D * h2  = new TH1D("h", "", 256, 0, 255);

  std::string filename = "thrscan_threshold_MSB.csv";

  std::ifstream pxfile(filename);
  if(!pxfile.is_open()) {
    std::cout << "Could not open matrix file \"" << filename << "\"" << std::endl;
  }
  else {
    std::string line = "";
    std::map<int, std::map<int, bool> > pixel_seen;
    int pixels = 0;
    
    while(std::getline(pxfile, line)) {
      if(!line.length() || '#' == line.at(0))
	continue;

      std::stringstream ss(line);
      std::string value;
      int thr, column, row, flag, tot, toa;

      int i = 0;
      while(getline(ss,value,',')) {
	if(i == 0) thr = atoi(value.c_str());
	else if(i == 1) column = atoi(value.c_str());
	else if(i == 2) row = atoi(value.c_str());
	else if(i == 3) flag = atoi(value.c_str());
	else if(i == 4) tot = atoi(value.c_str());
	else if(i == 5) toa = atoi(value.c_str());
	i++;
      }
      
      if(!pixel_seen[column][row]) {
	std::cout << "Seen pixel " << column << "," << row << std::endl;
	h->SetBinContent(column+1, row+1, thr);
	h2->Fill(thr,1);
	pixel_seen[column][row] = true;
	pixels++;
      }
    }

    std::cout << pixels << " pixels responded" << std::endl;
    h->GetXaxis()->SetTitle("column");
    h->GetZaxis()->SetTitle("1st response at thr_MSB");
    h->GetYaxis()->SetTitle("row");
    c->cd();
    h->Draw("colz");
    c2->cd();
    h2->GetXaxis()->SetTitle("1st response at threshold_MSB");
    h2->GetYaxis()->SetTitle("# pixels");
    h2->Draw();
  }
}
