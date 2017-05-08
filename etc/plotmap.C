{

  TCanvas *c = new TCanvas("c","",0,0,700,700);
  gStyle->SetPalette(1);
  TH2D * h  = new TH2D("h", "", 128, 0, 128, 128, 0, 128);

  std::string filename = "debug.txt";

  std::ifstream pxfile(filename);
  if(!pxfile.is_open()) {
    std::cout << "Could not open matrix file \"" << filename << "\"" << std::endl;
  }
  else {
    int masks = 0;
    std::string line = "";
    while(std::getline(pxfile, line)) {
      std::cout << "Read line: \"" << line << "\"" << std::endl;

      std::istringstream pxline(line);
      int column, row, mask;
      if(pxline >> row >> column >> mask) {
	std::cout << row << " " << column << " " << mask << "\n";
	h->SetBinContent(column+1, row+1, mask);
	if(mask) masks++;
      }
    }
    std::cout << masks << " masked pixels" << std::endl;
    h->Draw("colz");
  }
}
