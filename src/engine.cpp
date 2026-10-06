// OpenBabel 中文工作台, GPL-2.0-only. Uses the supplied Open Babel 3.2.1 source.
#include <openbabel/babelconfig.h>
#include <openbabel/obconversion.h>
#include <openbabel/mol.h>
#include <openbabel/atom.h>
#include <openbabel/bond.h>
#include <openbabel/obiter.h>
#include <openbabel/oberror.h>
#include <openbabel/op.h>
#include <openbabel/forcefield.h>
#include <openbabel/chargemodel.h>
#include <openbabel/descriptor.h>
#include <openbabel/elements.h>
#include <openbabel/generic.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <cmath>
#include <algorithm>
#include <stdexcept>
using namespace OpenBabel;
using namespace std;
using Options=map<string,string>;
static string value(const Options& o,const string& k,const string& d=""){auto it=o.find(k);return it==o.end()?d:it->second;}
static bool yes(const Options& o,const string& k){return value(o,k)=="1";}
static void check(bool ok,const char* code){if(!ok)throw runtime_error(code);}
static string csv(const string& s){string v="\"";if(!s.empty()&&string("=+-@\t\r").find(s.front())!=string::npos)v+='\'';for(char c:s){if(c=='\"')v+='\"';v+=c;}return v+'\"';}
static void preview(OBMol mol){
  mol.DeleteHydrogens();
  if(mol.NumAtoms()>200)return;
  auto* op=OBOp::FindType("gen2D");
  if(!op||!op->Do(&mol))return;
  ofstream out("preview.txt");
  out<<mol.NumAtoms()<<' '<<mol.NumBonds()<<'\n';
  FOR_ATOMS_OF_MOL(a,mol) out<<OBElements::GetSymbol(a->GetAtomicNum())<<' '<<a->GetX()<<' '<<a->GetY()<<'\n';
  FOR_BONDS_OF_MOL(b,mol) out<<b->GetBeginAtomIdx()<<' '<<b->GetEndAtomIdx()<<' '<<b->GetBondOrder()<<'\n';
}
static string descriptor(OBMol& m,const char* id){
  auto* d=OBDescriptor::FindType(id);if(!d)return "";
  double n=d->Predict(&m);if(!isfinite(n))return "";
  ostringstream s;s<<setprecision(8)<<n;return s.str();
}
static void properties(ofstream& out,OBMol& m,int i){
  OBConversion can;can.SetOutFormat("can");can.AddOption("n",OBConversion::OUTOPTIONS);
  string smi=can.WriteString(&m,true);
  out<<i<<','<<csv(m.GetTitle())<<','<<csv(m.GetFormula())<<','<<setprecision(10)
     <<m.GetMolWt()<<','<<m.GetExactMass()<<','<<m.NumAtoms()<<','<<m.NumHvyAtoms()<<','
     <<m.NumBonds()<<','<<m.NumRotors()<<','<<m.GetTotalCharge()<<','
     <<descriptor(m,"HBD")<<','<<descriptor(m,"HBA1")<<','<<descriptor(m,"logP")<<','
     <<descriptor(m,"TPSA")<<','<<csv(smi)<<"\n";
}
static void transform(OBMol& m,const Options& o){
  if(yes(o,"largest"))m.StripSalts();
  string h=value(o,"hydrogen","keep");
  if(h=="remove")check(m.DeleteHydrogens(),"HYDROGEN");
  if(h=="all")check(m.AddHydrogens(),"HYDROGEN");
  if(h=="polar")check(m.AddHydrogens(true),"HYDROGEN");
  if(h=="ph"){
    double ph=stod(value(o,"ph","7.4"));check(isfinite(ph)&&ph>=0&&ph<=14,"PH");
    m.DeleteHydrogens();check(m.AddHydrogens(false,true,ph),"HYDROGEN");
  }
  string coord=value(o,"coordinates","keep");
  if(coord=="2d"||coord=="3d"){
    check(m.NumHvyAtoms()<=500,"SIZE");
    auto* p=OBOp::FindType(coord=="2d"?"gen2D":"gen3D");
    string speed=value(o,"quality","fast");
    check(p&&p->Do(&m,coord=="3d"?speed.c_str():""),"COORDINATES");
    check(coord!="3d"||m.Has3D(),"COORDINATES");
  }
  if(yes(o,"minimize")){
    check(m.NumHvyAtoms()<=500,"SIZE");check(m.Has3D(),"NEED3D");
    m.AddHydrogens();
    string name=value(o,"forcefield","MMFF94");auto* ff=OBForceField::FindForceField(name.c_str());
    check(ff&&ff->Setup(m),"FORCEFIELD");
    int steps=stoi(value(o,"steps","500"));check(steps>=1&&steps<=100000,"STEPS");
    ff->SetLogLevel(OBFF_LOGLVL_NONE);ff->ConjugateGradients(steps,1e-6);
    check(ff->GetCoordinates(m),"MINIMIZE");
    double energy=ff->Energy();check(isfinite(energy),"MINIMIZE");
    auto* e=new OBPairData;e->SetAttribute("Energy");e->SetValue(to_string(energy));m.SetData(e);
  }
  // Coordinate generation and force fields add explicit H internally.
  // Apply an explicit removal/polar-only choice again to the final structure.
  if(h=="remove")m.DeleteHydrogens();
  if(h=="polar")m.DeleteNonPolarHydrogens();
  string charge=value(o,"charge","keep");
  if(charge!="keep"){
    auto* p=OBChargeModel::FindType(charge.c_str());check(p&&p->ComputeCharges(m),"CHARGE");
    FOR_ATOMS_OF_MOL(a,m) check(isfinite(a->GetPartialCharge()),"CHARGE");
  }
  if(value(o,"outformat")=="pdbqt")check(m.Has3D(),"NEED3D");
}
static int run(const Options& o){
  string inf=value(o,"informat"),outf=value(o,"outformat","sdf");
  bool props=yes(o,"properties"),split=yes(o,"split");
  OBConversion reader,writer;
  check(reader.SetInFormat(inf.c_str()),"INPUT_FORMAT");
  if(!props){
    check(writer.SetOutFormat(outf.c_str()),"OUTPUT_FORMAT");
    if(yes(o,"rigid")){writer.AddOption("r",OBConversion::OUTOPTIONS);writer.AddOption("c",OBConversion::OUTOPTIONS);}
    if(yes(o,"preserve_h"))writer.AddOption("h",OBConversion::OUTOPTIONS);
    if(yes(o,"preserve_names")){writer.AddOption("n",OBConversion::OUTOPTIONS);writer.AddOption("p",OBConversion::OUTOPTIONS);}
  }
  ifstream in("input.dat",ios::binary);check(bool(in),"READ");
  ofstream out;
  if(props){
    out.open("result.csv",ios::binary);check(bool(out),"WRITE");
    out<<"\xEF\xBB\xBF序号,名称,分子式,相对分子质量,精确质量,显式原子数,重原子数,显式键数,可旋转键数,总形式电荷,氢键供体数,氢键受体数（HBA1）,logP,拓扑极性表面积（平方埃）,规范SMILES\n";
  } else if(!split){out.open("result."+outf,ios::binary);check(bool(out),"WRITE");writer.SetOutStream(&out);}
  reader.SetInStream(&in);
  OBMol current,next;
  unsigned errors=obErrorLog.GetErrorMessageCount();
  bool have=reader.Read(&current);check(have&&current.NumAtoms()>0,"EMPTY");
  check(obErrorLog.GetErrorMessageCount()==errors,"PARSE");
  int count=0;
  while(have){
    check(current.NumAtoms()>0,"EMPTY");
    errors=obErrorLog.GetErrorMessageCount();
    next.Clear();bool more=reader.Read(&next);
    check(obErrorLog.GetErrorMessageCount()==errors,"PARSE");
    transform(current,o);
    check(obErrorLog.GetErrorMessageCount()==errors,"PROCESS");
    if(!props&&!split&&more&&((writer.GetOutFormat()->Flags()&WRITEONEONLY)||outf=="mol"))throw runtime_error("SINGLE_ONLY");
    ++count;
    if(props)properties(out,current,count);
    else if(split){
      writer.SetLast(true);writer.SetOutputIndex(1);
      check(writer.WriteFile(&current,"result_"+to_string(count)+"."+outf),"WRITE");writer.CloseOutFile();
    } else {writer.SetLast(!more);writer.SetOutputIndex(count);check(writer.Write(&current),"WRITE");}
    check(obErrorLog.GetErrorMessageCount()==errors,"PROCESS");
    if(count==1){
      ofstream summary("summary.txt");
      summary<<current.GetFormula()<<'\n'<<fixed<<setprecision(4)<<current.GetMolWt()<<'\n'
             <<current.NumHvyAtoms()<<'\n'<<current.NumRotors()<<'\n';
      preview(current);
    }
    cout<<"PROGRESS\t"<<count<<endl;
    have=more;if(have)current=next;
  }
  if(out.is_open()){out.flush();check(bool(out),"WRITE");out.close();}
  cout<<"OK\t"<<count<<'\t'<<obErrorLog.GetWarningMessageCount()<<endl;
  return 0;
}
int main(int argc,char** argv){
  try {
    if(argc>1&&string(argv[1])=="--formats"){
      OBConversion c;auto ins=c.GetSupportedInputFormat(),outs=c.GetSupportedOutputFormat();
      map<string,pair<bool,bool>> formats;
      for(auto s:ins)formats[s.substr(0,s.find(' '))].first=true;
      for(auto s:outs)formats[s.substr(0,s.find(' '))].second=true;
      for(auto& p:formats)cout<<p.first<<'\t'<<p.second.first<<'\t'<<p.second.second<<'\n';
      return 0;
    }
    check(argc==3&&string(argv[1])=="--job","ARGUMENTS");
    ifstream f(argv[2]);check(bool(f),"CONFIG");Options opts;string s;
    while(getline(f,s)){if(!s.empty()&&s.back()=='\r')s.pop_back();auto p=s.find('=');if(p!=string::npos)opts[s.substr(0,p)]=s.substr(p+1);}
    return run(opts);
  }catch(const exception& e){cout<<"ERROR\t"<<e.what()<<endl;return 2;}
  catch(...){cout<<"ERROR\tUNKNOWN"<<endl;return 3;}
}
