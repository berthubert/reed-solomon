#include <iostream>
#include <vector>
#include <stdexcept>
#include <string.h>
#include <chrono>
#include <fstream>
#include "rs.hh"
#include "ext/argparse.hpp"
using namespace std;



int main(int argc, char** argv)
try
{
  argparse::ArgumentParser program("ipprotect");

  program.add_argument("ipaddr")
    .required()
    .help("IP address to encode/decode");

  program.add_argument("-n", "--nroots")
  .default_value(2)
  .required()
  .action([](const std::string& value) { return std::stoi(value); })
  .help("specify number of roots (parity symbols)");

  program.add_argument("-p", "--poly")
    .default_value<std::vector<unsigned int>>({0, 1, 2, 7, 8 })
    .append()
    .action([](const std::string& value) { return std::stoi(value); })        
    .help("degrees of polynomial");

  program.add_argument("-e", "--erase")
    .default_value<std::vector<unsigned int>>({})
    .append()
    .action([](const std::string& value) { return std::stoi(value); })        
    .help("tell decoder about an erased position");

  
  program.add_argument("--prim")
    .default_value<int>(11)
    .action([](const std::string& value) { return std::stoi(value); })    
    .help("the primitive element in the Galois field used to generate the Reed Solomon code generator polynomial");

  program.add_argument("--fcr")
    .default_value<int>(121)
    .action([](const std::string& value) { return std::stoi(value); })    
    .help("the first consecutive root of the Reed Solomon code generator polynomial");


  try {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error& err) {
    std::cout << err.what() << std::endl;
    std::cout << program;
    exit(0);
  }

  auto nroots = program.get<int>("--nroots");
  std::cout << "nroots: "<<nroots <<endl;;
  auto prim = program.get<int>("--prim");
  auto fcr = program.get<int>("--fcr");
  cout<<"prim: "<<prim<<endl;
  cout<<"fcr: "<<fcr<<endl;

  auto poly = program.get<std::vector<unsigned int>>("--poly");
  std::cout<< "poly: ";
  bool first=true;
  for(const auto& p : poly) {
    if(!first)
      cout<<" + ";
    else
      first=false;
    if(p==0)
      cout<<"1";
    else if(p==1)
      cout<<"x";
    else
      cout<<"x^"<<p;
  }
  cout<<endl;
  
  
  RSCodecInt rsc(poly, fcr, prim, nroots, 249, 8);
  cout<<"polyval: ";
  printf("0x%02x\n", rsc.getPoly());
  cout<<"(N,K) = ("<<rsc.d_N<<","<<rsc.d_K<<")\n";
  
  string ipaddr(program.get<string>("ipaddr"));

  unsigned int i1, i2, i3, i4, p1, p2;
  int count=sscanf(ipaddr.c_str(), "%u.%u.%u.%u.%u.%u", &i1, &i2, &i3, &i4, &p1, &p2);
    
  if(count==4) { // encode
    vector<unsigned int> msg({i1,i2,i3,i4});
    rsc.encode(msg);
    
    cout<<"The "<<rsc.d_N -rsc.d_K<<" parity bytes for \""<<i1<<"."<<i2<<"."<<i3<<"."<<i4<<"\": ";
    for(int n=rsc.d_K; n < rsc.d_N; ++n)
      printf("%02x", (int)(uint8_t)msg.at(n));
    cout<<"\n";
    bool first=true;
    for(const auto& i : msg) {
      if(!first) {
	cout<<".";
      }
      else
	first=false;

      cout<<i;;
    }
    cout<<endl;
    
  }
  else if(count==6) {
    vector<unsigned int> msg({i1,i2,i3,i4,p1,p2});
    vector<unsigned int> out;
    vector<unsigned int> corrections = program.get<vector<unsigned int>>("--erase");
    if(!corrections.empty()) {
      cout<<"Informing decoder about "<<corrections.size()<<" erasures: ";
      for(const auto& e : corrections)
	cout<<" "<< e;
      cout<<endl;
    }
    int result=rsc.decode(msg, out, &corrections);
    cout<<"Fixed "<<result<<" corruptions";
    if(result) {
      cout<<" in positions:";
      for(const auto& c : corrections)
	cout<<" "<<c;
    }
    cout<<endl;
	  
    cout<<"Recovered: ";
    bool first=true;
    for(const auto& i : out) {
      if(!first) {
	cout<<".";
      }
      else
	first=false;

      cout<<i;
    }
    cout<<endl;
  }
}
catch(std::exception& e) {
   cerr<<"Fatal error: "<<e.what()<<endl;
   return EXIT_FAILURE;
}
