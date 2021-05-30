extern "C" {
#include <fec.h>
}
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string.h>
#include <chrono>
#include <fstream>
using namespace std;

//        void *init_rs_char(int symsize,int gfpoly,int fcr,int prim,
//            int nroots,int pad);


// symsize == 8
class RSCodec
{
public:
  RSCodec(const std::initializer_list<int>& roots, int fcr, int prim, int nroots, int pad=0)
    : d_K(256 - pad -1), d_N(d_K - nroots), d_nroots(nroots)
  {
    unsigned int gfpoly=0;
    for(const auto& r : roots)
      gfpoly |= (1<<r);
    //    printf("gfpoly = 0x%x\n", gfpoly);
    d_rs = init_rs_char(8, gfpoly, fcr, prim, nroots, pad);
    if(!d_rs)
      throw std::runtime_error("Unable to initialize RS codec");
    //    cout<<"(N,K) = ("<<d_N<<", "<<d_K<<") - adding "<<d_nroots<<" bytes of parity\n";
  }

  void encode(std::string& msg)
  {

    if(msg.size() > d_N)
      throw std::runtime_error("Can't encode message longer than "+std::to_string(d_K)+" bytes");
    msg.append(d_N - msg.size(), ' ');
    //    cout<<"msg.length(): "<<msg.length()<<endl;
    //    void encode_rs_char(void *rs,unsigned char *data,
    //            unsigned char *parity);
    uint8_t parity[d_nroots];
    encode_rs_char(d_rs, (uint8_t*)msg.c_str(), parity);
    msg.append((char*)&parity[0], (char*)&parity[d_nroots]);
  }

  int decode(const std::string& in, std::string& out)
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
    if(ret) {
      cout<<"Corrected positions: ";
      for(int n=0; n < ret; ++n)
	cout<<eras_pos[n]<<" ";
      cout<<endl;
    }
    
    out.assign((char*) data, (char*)data + d_K);
    return ret;

  }
  
  ~RSCodec()
  {
    if(d_rs)
      free_rs_char(d_rs);
  }
private:
  void* d_rs{0};
public:
  const int d_K, d_N, d_nroots;
};

int main()
{

  ofstream rescsv("results.csv");
  rescsv<<"fcr,prim,success,usec"<<endl;
  
  srandom(time(0));
  int usec;
  for(int fcr=1; fcr < 255; ++fcr) {
    for(int prim=1; prim < 100 ; ++prim) {
      // 1 + X + X^2 + X^7 + X^8
      //  RSCodec rsc({0, 1, 2, 7, 8}, 112, 11, 32);
      
      // 1 + x^2 + x^3 + x^4 + x^8
      RSCodec rsc({0, 2, 3, 4, 8}, fcr, prim, 32);
      auto start = chrono::steady_clock::now();

      for(int i=0 ; i < 100; ++i) {
	string msg="Hello, world";
	string orig=msg;
	rsc.encode(msg);
	
	cout<<"Corrupting: ";
	for(int n = 0; n < 16;++n) {
	  
	  int corpos = random() % msg.size();
	  cout<<corpos<<" ";
	  msg[corpos]='!';
	}
	cout<<endl;
	
	string recovered2;
	int result;
	try {
	  result = rsc.decode(msg, recovered2);
	}
	catch(...) {
	  goto fail;
	}
	cout<<"Result code: "<<result<<endl;
	recovered2.resize(rsc.d_N);
	//    cout<<"'"<<orig <<"' -> '"<< recovered2 <<"'"<<endl;
	recovered2.resize(orig.size());
	if(orig == recovered2)
	  cout<<"OK!"<<endl;
	else {
	  cerr<<"ERROR!"<<endl;
	  //	  cerr<< "'"<<orig <<"'"<<endl;
	  //	  cerr<< "'"<<recovered2 <<"'"<<endl;
	  goto fail;
	}
      }
      usec = std::chrono::duration_cast<std::chrono::microseconds>(chrono::steady_clock::now()- start).count();
      rescsv<<fcr<<","<<prim<<",1,"<<usec<<"\n";
      cout<<fcr <<", "<<prim<<" worked, took "<<
	usec<<" usec"<<endl;
      continue;
    fail:
      cout<<fcr <<", "<<prim<<" FAILED!"<<endl;
      rescsv<<fcr<<","<<prim<<",0,-1\n";
    }
  }
}
