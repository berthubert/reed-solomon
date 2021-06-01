#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ext/doctest.h"
#include "rs.hh"
using namespace std;

TEST_CASE("basic test") {
  RSCodec rsc({0,1,2,7,8}, 121, 11, 32);
  string msg("Hello, world");
  string orig(msg);
  rsc.encode(msg);

  string out;
  CHECK(rsc.decode(msg, out) == 0);

  string recover=out;
  out.resize(orig.size());

  CHECK(out == orig);

  msg[1]='x';
  msg[3]='x';

  CHECK(rsc.decode(msg, out) == 2);
  out.resize(orig.size());
  CHECK(out == orig);
}

TEST_CASE("erasure test") {
  RSCodec rsc({0,1,2,7,8}, 121, 11, 32);
  string msg("Hello world, this is a somewhat longer story. It needs to be 64 characters long! At least.");
  string orig(msg);
  rsc.encode(msg);

  string out;

  vector<int> erased;
  for(int n=0; n < 32; ++n) {
    int pos = n*2;
    msg[pos]='x';
    erased.push_back(pos);
  }
    

  CHECK(rsc.decode(msg, out, &erased) >= 0);
  out.resize(orig.size());
  CHECK(out == orig);
}
