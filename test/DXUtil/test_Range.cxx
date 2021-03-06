// test_Range.cxx

// David Adams
// September 2015
//
// Test script for Range.

#include "DXUtil/Range.h"

#include <string>
#include <iostream>
#include <cassert>

using std::string;
using std::cout;
using std::endl;

typedef Range<int> IntRange;
int main() {
  const string myname = "test_Range: ";
  cout << myname << "Starting test" << endl;
  IntRange rng(1,10);
  assert( *rng.begin() == 1 );
  assert( *rng.end() == 11 );
  assert( rng.first() == 1 );
  assert( rng.last() == 10 );
  assert( rng.size() == 10 );
  int count = 0;
  for ( auto i : rng ) {
    cout << myname << "  " << i << endl;
    ++count;
  }
  assert( count == 10 );

  IntRange rng2(2,2);
  assert( *rng2.begin() == 2 );
  assert( *rng2.end() == 3 );
  assert( rng2.first() == 2 );
  assert( rng2.last() == 2 );
  assert( rng2.size() == 1 );

  cout << myname << "Done." << endl;
  return 0;
}
