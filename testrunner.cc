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


TEST_CASE("hex test") {
  // code word is 2^4 = 15 symbols of 4 bits each, 48 bits
  //                          nroots  pad  symbol
  RSCodec rsc({0,1,4}, 12, 7, 6,      1,    4);
  // 10 symbols payload, 6 symbols protection, no padding
  string msg({0xc, 0x0, 0xa, 0x8, 0x0, 0x01, 0x0, 0x1}); // 192.168.1.1
  cout<<msg.size()<<endl;
  
  string orig(msg);
  rsc.encode(msg);
  for(int n=0; n < 14; n+=2) {
    cout<<(int)(16*msg[n] + msg[n+1]) << ".";
  }
  cout<<(int)msg[14]<<endl;
  for(const auto& e : msg) {
    printf("%1x", (unsigned int) e);
  }
  cout<<endl;

  string out;
  CHECK(rsc.decode(msg, out) == 0);

  string recover=out;
  out.resize(orig.size());

  CHECK(out == orig);

  msg[1]=7;
  msg[3]=2;

  CHECK(rsc.decode(msg, out) == 2);
  out.resize(orig.size());
  CHECK(out == orig);
}

TEST_CASE("hex test int") {
  // code word is 2^4 = 15 symbols of 4 bits each, 48 bits
  //                          nroots  pad  symbol
  RSCodecInt rsc({0,1,4}, 12, 7, 6,      1,    4);
  // 10 symbols payload, 6 symbols protection, no padding
  vector<unsigned int>msg({0xc, 0x0, 0xa, 0x8, 0x0, 0x01, 0x0, 0x1}); // 192.168.1.1
  
  auto orig(msg);
  rsc.encode(msg);
  cout<<"msg.size() now "<<msg.size()<<endl;
  for(int n=0; n < 14; n+=2) {
    cout<<(int)(16*msg[n] + msg[n+1]) << ".";
  }
  cout<<(int)msg[14]<<endl;
  for(const auto& e : msg) {
    printf("%1x", (unsigned int) e);
  }
  cout<<endl;

  vector<unsigned int> out;
  CHECK(rsc.decode(msg, out) == 0);

  auto recover=out;
  out.resize(orig.size());

  CHECK(out == orig);

  msg[1]=7;
  msg[3]=2;

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

  vector<unsigned int> erased;
  for(int n=0; n < 32; ++n) {
    int pos = n*2;
    msg[pos]='x';
    erased.push_back(pos);
  }
    

  CHECK(rsc.decode(msg, out, &erased) >= 0);
  out.resize(orig.size());
  CHECK(out == orig);
}
