// dxopen.h

#ifndef dxopen_H
#define dxopen_H

class TFile;

// Default DXDisplay file.
extern TFile* gDXFile;

// Opens a file and set gFile nd gDXFile to point to
// that file iff it is a DXDisplay file.
int dxopen(string ifname);

#endif
