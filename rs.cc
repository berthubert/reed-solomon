#include "rs.hh"
#include <stdexcept>
#include <string.h>
#include <iostream>
extern "C" {
#include <fec.h>
}
using namespace std;

RSCodec::RSCodec(const std::vector<int>& roots, int fcr, int prim, int nroots, int pad)
    : d_N(256 - pad -1), d_K(256 - pad - 1 - nroots), d_nroots(nroots)
{

  for(const auto& r : roots)
    d_gfpoly |= (1<<r);
  //    printf("gfpoly = 0x%x\n", gfpoly);
  d_rs = init_rs_char(8, d_gfpoly, fcr, prim, nroots, pad);
  if(!d_rs)
    throw std::runtime_error("Unable to initialize RS codec");
  //    cout<<"(N,K) = ("<<d_N<<", "<<d_K<<") - adding "<<d_nroots<<" bytes of parity\n";
}

void RSCodec::encode(std::string& msg)
{
  if(msg.size() > d_K)
    throw std::runtime_error("Can't encode message longer than "+std::to_string(d_K)+" bytes");
  msg.append(d_K - msg.size(), ' ');
  //    cout<<"msg.length(): "<<msg.length()<<endl;
  //    void encode_rs_char(void *rs,unsigned char *data,
  //            unsigned char *parity);
  uint8_t parity[d_nroots];
  encode_rs_char(d_rs, (uint8_t*)msg.c_str(), parity);
  msg.append((char*)&parity[0], (char*)&parity[d_nroots]);
}

int RSCodec::decode(const std::string& in, std::string& out, vector<int>* corrs)
{
  // int decode_rs_char(void *rs,unsigned char *data,int *eras_pos,
  //        int no_eras);
  
  unsigned char data[in.length()];
  memcpy(data, in.c_str(), in.length());
  int eras_pos[d_nroots];
  int ret = decode_rs_char(d_rs, data, eras_pos, 0);
  /*
    The decoder corrects the symbols "in place", returning the number of symbols in error. If the codeword is uncorrectable, -1 is returned and the data  block  is  unchanged.  If
    eras_pos  is non-null, it is used to return a list of corrected symbol positions, in no particular order.  This means that the array passed through this parameter must have at
    least nroots elements to prevent a possible buffer overflow.
  */
  if(ret < 0)
    throw std::runtime_error("Could not correct message");
  if(corrs)
    corrs->clear();
  if(ret && corrs) {
    for(int n=0; n < ret; ++n)
      corrs->push_back(eras_pos[n]);
  }
  
  out.assign((char*) data, (char*)data + d_N);
  return ret;
  
}
  
RSCodec::~RSCodec()
{
  if(d_rs)
    free_rs_char(d_rs);
}
