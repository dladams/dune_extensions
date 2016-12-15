// DrawResult.cxx

#include "DrawResult.h"
#include "howStuck.h"
#include <string>
#include <iostream>
#include <sstream>
#include "TH2.h"

using std::string;
using std::cout;
using std::endl;

//**********************************************************************

DrawResult::DrawResult(int atmin, int atmax)
: tmin(atmin), tmax(atmax) {
  if ( tmax <= tmin ) tmax = tmin = 0;
}

//**********************************************************************

TH2* DrawResult::time() const {
  return hdraw;
}

//**********************************************************************

TH1* DrawResult::channels() const {
  return hdrawy;
}

//**********************************************************************

TH1* DrawResult::ticks() const {
  return hdrawx;
}

//**********************************************************************

TH1* DrawResult::timeChannel(unsigned int chan) const {
  if ( chan >= hdrawxChan.size() ) return 0;
  return hdrawxChan[chan];
}

//**********************************************************************

TH1* DrawResult::signalChannel(unsigned int chan, TH1** pphstuckRange, TH1** pphsameRange) {
  const string myname = "DrawResult::signalChannel: ";
  if ( chan >= hdrawxChan.size() ) return 0;
  if ( hsigChan.size() <= chan ) {
    hsigChan.resize(chan+1, nullptr);
    hstuckRange.resize(chan+1, nullptr);
    hsameRange.resize(chan+1, nullptr);
  }
  TH1*& phsig = hsigChan[chan];
  TH1*& phstuckRange = hstuckRange[chan];
  TH1*& phsameRange = hsameRange[chan];
  int maxStuck = 50;
  if ( phsig == nullptr ) {
    TH1* phtim = hdrawxChan[chan];
    if ( phtim == nullptr ) {
      cout << myname << "Time spectrum not found for channel " << chan << endl;
      return nullptr;
    }
    string hname = string(phtim->GetName()) + "_signal";
    string hsname = string(phtim->GetName()) + "_stuckrange";
    string hrname = string(phtim->GetName()) + "_samerange";
    string htitl = "Signal for " + string(phtim->GetName());
    string hstitl = "Stuck-bit ranges for " + string(phtim->GetName());
    string hrtitl = "Same-value ranges for " + string(phtim->GetName());
    int nbin = nsig;
    double xmin = sigmin;
    double xmax = sigmax;
    if ( nbin == -1 ) {
      double dsig = sigmin;
      if ( dsig <= 0.0 ) dsig = 1.0;
      xmin = phtim->GetMinimum();
      xmax = phtim->GetMaximum() + dsig;
      nbin = (xmax - xmin)/dsig;
    }
    phsig = new TH1F(hname.c_str(), htitl.c_str(), nbin, xmin, xmax);
    phsig->GetXaxis()->SetTitle(phtim->GetYaxis()->GetTitle());
    phsig->GetYaxis()->SetTitle("Count");
    phstuckRange = new TH1F(hsname.c_str(), hstitl.c_str(), maxStuck, 0, maxStuck);
    phstuckRange->GetXaxis()->SetTitle("# contiguous ticks with stuck bits");
    phstuckRange->GetYaxis()->SetTitle("Count");
    phsameRange = new TH1F(hrname.c_str(), hrtitl.c_str(), maxStuck, 0, maxStuck);
    phsameRange->GetXaxis()->SetTitle("# contiguous ticks with same value");
    phsameRange->GetYaxis()->SetTitle("Count");
    int nstuck = 0;
    bool isStuck = false;
    int ntbin = phtim->GetNbinsX();
    double xadcLast = -1.e20;
    unsigned int nsame = 0;
    for ( int ibin=1; ibin<=ntbin; ++ibin ) {
      bool isLast = ibin == ntbin;
      double xadc = phtim->GetBinContent(ibin);
      phsig->Fill(xadc);
      unsigned short iadc = xadc + 0.01;
      bool wasStuck = isStuck;
      if ( ! wasStuck ) nstuck = 0;
      isStuck = howStuck(iadc);
      if ( isStuck ) ++nstuck;    // # consecutive sticks
      if ( wasStuck ) {
        if ( ! isStuck || isLast ) {
          phstuckRange->Fill(nstuck, nstuck);
        }
      }
      bool isSame = xadc == xadcLast;
      // If value has changed, fill for the last bin.
      if ( nsame > 0 ) {
        if ( !isSame ) {
          for ( unsigned int ifil=0; ifil<nsame; ++ifil ) phsameRange->Fill(nsame);
        }
      }
      nsame = isSame ? nsame+1 : 1;
      // If this is the last, fill for this bin.
      if ( isLast ) phsameRange->Fill(nsame, nsame);
      xadcLast = xadc;
    } 
  }
  if ( pphstuckRange != nullptr ) *pphstuckRange = phstuckRange;
  if ( pphsameRange != nullptr ) *pphsameRange = phsameRange;
  return phsig;
}

//**********************************************************************

TH2* DrawResult::freq() {
  const string myname = "DrawResult::freq: ";
  if ( pfft == nullptr ) {
    if ( hdraw == nullptr ) {
      cout << myname << "ERROR: Unable to find FFT or time histogram." << endl;
      return nullptr;
    }
    cout << myname << "Performing FFT." << endl;
    pfft = new FFTHist(hdraw, tmin, tmax, 2000);
  }
  return pfft->hfreq;
}

//**********************************************************************

TH2* DrawResult::power() {
  const string myname = "DrawResult::power: ";
  if ( freq() == nullptr ) return nullptr;
  return pfft->hpower;
}

//**********************************************************************

TH1* DrawResult::freqChannel(unsigned int chan) {
  const string myname = "DrawResult::freqChannel: ";
  TH2* phfreq = freq();
  if ( phfreq == nullptr ) {
    cout << myname << "ERROR: Unable to find FFT spectrum." << endl;
    return nullptr;
  }
  unsigned int nchan = phfreq->GetNbinsY();
  if ( chan >= nchan ) {
    cout << myname << "Invalid channel number: " << chan << endl;
    return nullptr;
  }
  if ( hfreqChan.size() < nchan ) hfreqChan.resize(nchan, nullptr);
  TH1*& ph = hfreqChan[chan];
  if ( ph == nullptr ) {
    ostringstream sshname;
    sshname << phfreq->GetName() << "_chan";
    if ( chan < 100 ) sshname << "0";
    if ( chan < 10 ) sshname << "0";
    sshname << chan;
    string hname = sshname.str();
    ph = phfreq->ProjectionX(hname.c_str(), chan+1, chan+1);
    ph->SetStats(0);
    ostringstream sshtitle;
    sshtitle << ph->GetTitle() << " channel " << chan;
    ph->SetTitle(sshtitle.str().c_str());
  }
  return ph;
}

//**********************************************************************

TH1* DrawResult::powerChannel(unsigned int chan) {
  const string myname = "DrawResult::powerChannel: ";
  TH2* phfpwr = power();
  if ( phfpwr == nullptr ) {
    cout << myname << "ERROR: Unable to find FFT spectrum." << endl;
    return nullptr;
  }
  unsigned int nchan = phfpwr->GetNbinsY();
  if ( chan >= nchan ) {
    cout << myname << "Invalid channel number: " << chan << endl;
    return nullptr;
  }
  if ( hfpwrChan.size() < nchan ) hfpwrChan.resize(nchan, nullptr);
  TH1*& ph = hfpwrChan[chan];
  if ( ph == nullptr ) {
    ostringstream sshname;
    sshname << phfpwr->GetName() << "_chan";
    if ( chan < 100 ) sshname << "0";
    if ( chan < 10 ) sshname << "0";
    sshname << chan;
    string hname = sshname.str();
    ph = phfpwr->ProjectionX(hname.c_str(), chan+1, chan+1);
    ph->SetStats(0);
    ostringstream sshtitle;
    sshtitle << ph->GetTitle() << " channel " << chan;
    ph->SetTitle(sshtitle.str().c_str());
  }
  return ph;
}

//**********************************************************************

TH1* DrawResult::timePowerChannel(unsigned int chan) {
  const string myname = "DrawResult::powerChannel: ";
  TH2* phtpwr = time();
  if ( phtpwr == nullptr ) {
    cout << myname << "ERROR: Unable to find time spectrum." << endl;
    return nullptr;
  }
  unsigned int nchan = phtpwr->GetNbinsY();
  if ( chan >= nchan ) {
    cout << myname << "Invalid channel number: " << chan << endl;
    return nullptr;
  }
  if ( htpwrChan.size() < nchan ) htpwrChan.resize(nchan, nullptr);
  TH1*& ph = htpwrChan[chan];
  if ( ph == nullptr ) {
    ostringstream sshname;
    sshname << phtpwr->GetName() << "_chan";
    if ( chan < 100 ) sshname << "0";
    if ( chan < 10 ) sshname << "0";
    sshname << chan;
    string hname = sshname.str();
    TH1* pht = timeChannel(chan);
    ph = new TH1F(hname.c_str(), pht->GetTitle(), tmax-tmin, tmin, tmax);
    for ( int ibin=1; ibin<=ph->GetNbinsX(); ++ibin ) {
      double sig = ph->GetBinContent(ibin);
      ph->SetBinContent(ibin, sig*sig);
    }
    ph->SetStats(0);
    ostringstream sshtitle;
    sshtitle << ph->GetTitle() << " channel " << chan;
    ph->SetTitle(sshtitle.str().c_str());
  }
  return ph;
}

//**********************************************************************

TH1* DrawResult::mean() {
  const string myname = "DrawResult::mean: ";
  if ( hmean == nullptr ) {
    if ( hdraw == nullptr ) return nullptr;
    unsigned int ncha = hdrawxChan.size();
    string hname = string(hdraw->GetName()) + "_mean";
    string htitl = string(hdraw->GetTitle()) + " mean ADC count";
    hmean = new TH1F(hname.c_str(), htitl.c_str(), ncha, 0, ncha);
    hname = string(hdraw->GetName()) + "_rms";
    htitl = string(hdraw->GetTitle()) + " RMS ADC count";
    hrms = new TH1F(hname.c_str(), htitl.c_str(), ncha, 0, ncha);
    for ( unsigned int icha=0; icha<hdrawxChan.size(); ++icha ) {
      TH1* ph = signalChannel(icha);
      double mean = ph->GetMean();
      double rms = ph->GetRMS();
      hmean->SetBinContent(icha+1, mean);
      hrms->SetBinContent(icha+1, rms);
    }
  }
  return hmean;
}

//**********************************************************************

TH1* DrawResult::rms() {
  const string myname = "DrawResult::ran: ";
  if ( hrms == nullptr ) mean();
  return hrms;
}

//**********************************************************************
