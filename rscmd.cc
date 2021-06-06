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
  argparse::ArgumentParser program("rsmcd");

  program.add_argument("message")
    .required()
    .help("message to encode");

  program.add_argument("parity")
    .default_value<string>("")
    .help("parity encoded in hex, for decoding");

  
  program.add_argument("-n", "--nroots")
  .default_value(32)
  .required()
  .action([](const std::string& value) { return std::stoi(value); })
  .help("specify number of roots (parity symbols)");

  program.add_argument("-p", "--poly")
    .default_value<std::vector<int>>({0, 1, 2, 7, 8 })
    .append()
    .action([](const std::string& value) { return std::stoi(value); })        
    .help("degrees of polynomial");

  program.add_argument("-e", "--erase")
    .default_value<std::vector<int>>({})
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

  program.add_argument("--exercise")
  .help("try parameters lots of times")
  .default_value(false)
  .implicit_value(true);

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

  auto poly = program.get<std::vector<int>>("--poly");
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
  
  
  RSCodec rsc(poly, fcr, prim, nroots);
  cout<<"polyval: ";
  printf("0x%02x\n", rsc.getPoly());
  cout<<"(N,K) = ("<<rsc.d_N<<","<<rsc.d_K<<")\n"; // XX WRONG
  
  string msg(program.get<string>("message"));
  string rawpar(program.get<string>("parity"));
  string orig=msg;

  if (program["--exercise"] == true) {
    cout<<"Attempting randomly modified messages & maximum damage"<<endl;
    int n=0;
    
    for(; n < 10000; ++n) {
      string newmsg=msg+to_string(random());
      string neworig(newmsg);

      rsc.encode(newmsg);
      string out;
      for(int n = 0 ; n < nroots/2 ; ++n)
	newmsg[random() % newmsg.size()]=0;
      int res= rsc.decode(newmsg, out);
      out.resize(neworig.size());
      if(out != neworig) {
	cerr<<"Oops - decode did not equal original message"<<endl;
	return EXIT_FAILURE;
      }
      //     cout<<"res = "<<res<<endl;
    }
    cout<<"Tried out "<<n<<" randomly modified messages & maximum damage. Worked!"<<endl;
    return EXIT_SUCCESS;
  }
  
  if(rawpar.empty()) { // encode
    rsc.encode(msg);
    
    cout<<"The "<<rsc.d_N -rsc.d_K<<" parity bytes for \""<<orig<<"\": ";
    for(int n=rsc.d_K; n < rsc.d_N; ++n)
      printf("%02x", (int)(uint8_t)msg.at(n));
    cout<<"\n";
  }
  else {
    string parity=rawpar;
    if(parity.size() != nroots*2) {
      cerr<<"Incorrect number of parity symbols for nroots="<<nroots<<endl;
      return EXIT_FAILURE;
    }
    msg.append(rsc.d_K - msg.size(), ' ');
    for(int n=0; n < nroots; ++n) {
      unsigned int v;
      sscanf(&parity.at(2*n), "%02x", &v);
      msg.append(1, (char)v);
    }
    std::string out;
    vector<int> corrections = program.get<vector<int>>("--erase");
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
	  
    out.resize(orig.size());
    cout<<"Recovered: "<< out <<endl;
  }
}
catch(std::exception& e) {
   cerr<<"Fatal error: "<<e.what()<<endl;
   return EXIT_FAILURE;
}
