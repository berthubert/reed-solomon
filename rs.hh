#pragma once
#include <string>
#include <vector>

// symsize == 8



class RSCodec
{
public:
  RSCodec(const std::vector<int>& roots, int fcr, int prim, int nroots, int pad=0);
  void encode(std::string& msg);
  
  int decode(const std::string& in, std::string& out, std::vector<int>* corrections=0);
  int getPoly() // the representation as a number
  {
    return d_gfpoly;
  }
  ~RSCodec();
private:
  void* d_rs{0};
  unsigned int d_gfpoly{0};
public:
  const int d_K, d_N, d_nroots;
};
