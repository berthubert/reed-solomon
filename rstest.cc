#include <iostream>
#include <vector>
#include <stdexcept>
#include <string.h>
#include <chrono>
#include <fstream>
#include "rs.hh"

using namespace std;

//        void *init_rs_char(int symsize,int gfpoly,int fcr,int prim,
//            int nroots,int pad);



int main()
{

  ofstream rescsv("results.csv");
  rescsv<<"fcr,prim,success,usec"<<endl;
  
  srandom(time(0));
  int usec;
  for(int fcr=1; fcr < 255; ++fcr) {
    for(int prim=1; prim < 100 ; ++prim) {
      // 1 + X + X^2 + X^7 + X^8
      RSCodec rsc({0, 1, 2, 7, 8}, fcr, prim, 32);
      
      // 1 + x^2 + x^3 + x^4 + x^8
      //RSCodec rsc({0, 2, 3, 4, 8}, fcr, prim, 32);
      auto start = chrono::steady_clock::now();

      for(int i=0 ; i < 100; ++i) {
	string msg="Hello, world "+std::to_string(i);
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
	recovered2.resize(rsc.d_K);
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
