// OpenBabel 中文工作台 1.0, GPL-2.0-only.
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <thread>
#include <atomic>
#include <cmath>
namespace fs=std::filesystem;
using namespace std;
enum {
  NAV=100,FILEMODE=110,TEXTMODE,ADD,CLEAR,FILES,SMILES,INFMT,OUTFMT,COORDINATES,QUALITY,HYDROGEN,PH,FORCEFIELD,STEPS,MINIMIZE,CHARGE,LARGEST,DOCKMODE,SPLIT,OUTDIR,BROWSE,RUN,CANCEL,OPEN,HELP,EXAMPLE,LOG,PROGRESS,PREVIEW,SUMMARY,KEEPH,DIAG,LABINF=180,LABOUT,LABCOORD,LABQUALITY,LABH,LABPH,LABFF,LABSTEPS,LABCHARGE,LABDOCK,INPUTNOTE,OUTNOTE
};
const UINT WM_EVENT=WM_APP+10;
const COLORREF INK=RGB(26,47,62),MUTED=RGB(106,121,133),ACCENT=RGB(15,117,111),BG=RGB(244,247,249),SIDE=RGB(19,42,58),WHITE=RGB(255,255,255),BORDER=RGB(221,229,234);
HINSTANCE instance;
HWND mainWindow;
map<int,HWND> ctl;
HFONT fn,fsml,ft,fb,fmono;
HBRUSH whiteBrush,bgBrush;
int dpi=96,page=0,cw=1200,ch=710;
bool textMode=false,busy=false;
atomic<bool> cancelled(false);
thread jobThread;
fs::path baseDir,lastOutput,configFile;
vector<fs::path> inputFiles;
vector<pair<string,wstring>> inputFormats,outputFormats;
struct Atom {
  wstring symbol;
  double x,y;
};
struct Bond {
  int a,b,order;
};
vector<Atom> atoms;
vector<Bond> bonds;
struct Event {
  int kind,index,total;
  wstring text;
  fs::path path;
};
struct Settings {
  int page;
  bool text;
  vector<fs::path> files;
  wstring smiles;
  fs::path output;
  map<string,string> options;
};
int px(int x){
  return MulDiv(x,dpi,96);
}
wstring wide(const string& s){
  if(s.empty())return L"";
  int n=MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),nullptr,0);
  wstring w(n,0);
  MultiByteToWideChar(CP_UTF8,0,s.data(),(int)s.size(),w.data(),n);
  return w;
}
string utf8(const wstring& w){
  if(w.empty())return "";
  int n=WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),nullptr,0,nullptr,nullptr);
  string s(n,0);
  WideCharToMultiByte(CP_UTF8,0,w.data(),(int)w.size(),s.data(),n,nullptr,nullptr);
  return s;
}
wstring get(int id){
  int n=GetWindowTextLengthW(ctl[id]);
  wstring s(n+1,0);
  GetWindowTextW(ctl[id],s.data(),n+1);
  s.resize(n);
  return s;
}
void set(int id,const wstring& s){
  SetWindowTextW(ctl[id],s.c_str());
}int sel(int id){
  return (int)SendMessageW(ctl[id],CB_GETCURSEL,0,0);
}void select(int id,int n){
  SendMessageW(ctl[id],CB_SETCURSEL,n,0);
}
bool checked(int id){
  return SendMessageW(ctl[id],BM_GETCHECK,0,0)==BST_CHECKED;
}void tick(int id,bool b){
  SendMessageW(ctl[id],BM_SETCHECK,b?BST_CHECKED:BST_UNCHECKED,0);
}void enable(int id,bool b){
  EnableWindow(ctl[id],b);
}void show(int id,bool b){
  ShowWindow(ctl[id],b?SW_SHOW:SW_HIDE);
}
void post(int k,const wstring& s=L"",int i=0,int n=0,fs::path p={
}){
  auto* e=new Event{
    k,i,n,s,p
  };
  if(!PostMessageW(mainWindow,WM_EVENT,0,(LPARAM)e))delete e;
}
string readFile(const fs::path& p){
  ifstream f(p,ios::binary);
  return string(istreambuf_iterator<char>(f),{
  });
}void writeFile(const fs::path& p,const string& s){
  ofstream f(p,ios::binary);
  f.write(s.data(),s.size());
  if(!f)throw runtime_error("WRITE");
}
wstring errorText(const string& code){
  static const map<string,wstring> e={
    {
      "INPUT_FORMAT",L"无法读取输入格式，请检查扩展名或手动指定。"
    },{
      "OUTPUT_FORMAT",L"当前引擎不支持所选输出格式。"
    },{
      "EMPTY",L"未读取到有效分子，请检查文件内容或 SMILES。"
    },
    {
      "PARSE",L"输入存在解析错误，未导出不完整结果。"
    },{
      "PROCESS",L"引擎报告分子处理错误，请查看诊断文件。"
    },{
      "READ",L"无法读取输入文件。"
    },{
      "WRITE",L"无法写入结果，请检查目录权限和磁盘空间。"
    },
    {
      "FORCEFIELD",L"力场无法为该分子分配参数，可尝试 UFF 或关闭优化。"
    },{
      "COORDINATES",L"坐标生成失败，请检查结构或调整三维质量。"
    },{
      "NEED3D",L"需要三维坐标。小分子可生成三维坐标；受体请提供已有三维结构。"
    },
    {
      "CHARGE",L"部分电荷计算失败，请更换模型或检查结构。"
    },{
      "MINIMIZE",L"结构优化失败或能量异常。"
    },{
      "HYDROGEN",L"氢原子处理失败。"
    },{
      "SIZE",L"坐标生成和优化限于 500 个重原子以内，大分子请保留原坐标。"
    },
    {
      "PH",L"酸碱度应在 0–14 之间。"
    },{
      "STEPS",L"优化步数应为 1–100000 的整数。"
    },{
      "SINGLE_ONLY",L"目标格式只支持单个分子，请勾选“每个分子单独成文件”。"
    },
    {
      "TIMEOUT",L"单个文件超过 10 分钟，已停止。请缩小批量或降低计算量。"
    },{
      "CANCEL",L"任务已取消。"
    },{
      "LAUNCH",L"无法启动引擎，请完整解压程序包。"
    },{
      "DATA",L"找不到计算参数文件，请完整解压程序包。"
    },{
      "CRASH",L"计算进程异常结束，未保存本文件结果。"
    }
  };
  auto i=e.find(code);
  return i==e.end()?L"处理未完成，请查看输出目录中的诊断记录。":i->second;
}
void log(const wstring& s){
  SYSTEMTIME t;
  GetLocalTime(&t);
  wchar_t a[32];
  swprintf(a,32,L"%02d:%02d:%02d  ",t.wHour,t.wMinute,t.wSecond);
  wstring line=a+s+L"\r\n";
  SendMessageW(ctl[LOG],EM_SETSEL,-1,-1);
  SendMessageW(ctl[LOG],EM_REPLACESEL,0,(LPARAM)line.c_str());
  SendMessageW(ctl[LOG],EM_SCROLLCARET,0,0);
}
HWND make(int id,const wchar_t* cls,const wchar_t* title,DWORD style=0,DWORD ex=0){
  HWND h=CreateWindowExW(ex,cls,title,WS_CHILD|WS_VISIBLE|style,0,0,10,10,mainWindow,(HMENU)(INT_PTR)id,instance,nullptr);
  ctl[id]=h;
  SendMessageW(h,WM_SETFONT,(WPARAM)fn,TRUE);
  return h;
}
void button(int id,const wchar_t* s){
  make(id,L"BUTTON",s,BS_OWNERDRAW|WS_TABSTOP);
}void label(int id,const wchar_t* s){
  make(id,L"STATIC",s,SS_LEFT);
}void edit(int id,const wchar_t* s,DWORD x=0){
  make(id,L"EDIT",s,WS_TABSTOP|ES_AUTOHSCROLL|x,WS_EX_CLIENTEDGE);
}void checkbox(int id,const wchar_t* s){
  make(id,L"BUTTON",s,BS_AUTOCHECKBOX|WS_TABSTOP);
}
void combo(int id,const vector<wstring>& list){
  make(id,L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP);
  for(auto& s:list)SendMessageW(ctl[id],CB_ADDSTRING,0,(LPARAM)s.c_str());
  select(id,0);
  SendMessageW(ctl[id],CB_SETDROPPEDWIDTH,px(265),0);
}
void move(int id,int x,int y,int w,int h){
  MoveWindow(ctl[id],px(x),px(y),px(w),px(h),TRUE);
}
wstring formatLabel(const string& f){
  static const map<string,wstring> names={
    {
      "sdf",L"结构数据文件"
    },{
      "mol",L"分子连接表"
    },{
      "smi",L"线性分子表示"
    },{
      "can",L"规范线性表示"
    },{
      "pdb",L"蛋白质结构"
    },{
      "pdbqt",L"对接结构"
    },{
      "mol2",L"原子类型与电荷"
    },{
      "xyz",L"笛卡尔坐标"
    },{
      "inchi",L"国际化学标识"
    },{
      "inchikey",L"化学标识键"
    },{
      "svg",L"二维矢量结构图"
    },{
      "cif",L"晶体信息"
    },{
      "cdx",L"化学绘图"
    },{
      "gjf",L"量化计算输入"
    },{
      "cml",L"化学标记语言"
    },{
      "fps",L"指纹文本"
    },{
      "report",L"分子详细报告"
    }
  };
  auto i=names.find(f);
  return wide(f)+L" · "+(i==names.end()?L"化学文件格式":i->second);
}
void loadFormats(){
  vector<string> reads,writes;
  istringstream ss(readFile(baseDir/L"engine"/L"formats.tsv"));
  string f;
  int r,w;
  while(ss>>f>>r>>w){
    if(r)reads.push_back(f);
    if(w)writes.push_back(f);
  }vector<string> common={
    "sdf","mol","smi","can","pdb","pdbqt","mol2","xyz","inchi","inchikey","svg","cif","cdx","gjf"
  };
  inputFormats={
    {
      "auto",L"自动识别扩展名"
    }
  };
  auto fill=[&](vector<string>& a,vector<pair<string,wstring>>& t){
    for(auto& s:common){
      auto i=find(a.begin(),a.end(),s);
      if(i!=a.end()){
        t.push_back({
          s,formatLabel(s)
        });
        a.erase(i);
      }
    }for(auto& s:a)t.push_back({
      s,formatLabel(s)
    });
  };
  fill(reads,inputFormats);
  fill(writes,outputFormats);
  for(auto& a:inputFormats)SendMessageW(ctl[INFMT],CB_ADDSTRING,0,(LPARAM)a.second.c_str());
  for(auto& a:outputFormats)SendMessageW(ctl[OUTFMT],CB_ADDSTRING,0,(LPARAM)a.second.c_str());
  SendMessageW(ctl[OUTFMT],CB_ADDSTRING,0,(LPARAM)L"csv · 中文分子属性表");
  select(INFMT,0);
  select(OUTFMT,0);
}
void chooseOutput(const string& f){
  for(size_t i=0;
  i<outputFormats.size();
  i++)if(outputFormats[i].first==f){
    select(OUTFMT,(int)i);
    return;
  }
}
void refreshEnabled(){
  for(auto& p:ctl)if(p.first!=LOG&&p.first!=PREVIEW&&p.first!=SUMMARY)EnableWindow(p.second,!busy);
  enable(CANCEL,busy&&!cancelled);
  enable(OPEN,!lastOutput.empty());
  enable(DIAG,!lastOutput.empty());
  enable(INFMT,!busy&&!textMode);
  enable(ADD,!busy&&!textMode);
  enable(CLEAR,!busy&&!textMode);
  bool prop=page==3;
  for(int id:{
    COORDINATES,QUALITY,HYDROGEN,PH,FORCEFIELD,STEPS,MINIMIZE,CHARGE,LARGEST,DOCKMODE,SPLIT,KEEPH,OUTFMT
  })enable(id,!busy&&!prop);
  enable(QUALITY,!busy&&!prop&&sel(COORDINATES)==2);
  enable(PH,!busy&&!prop&&sel(HYDROGEN)==4);
  enable(STEPS,!busy&&!prop&&checked(MINIMIZE));
  enable(FORCEFIELD,!busy&&!prop&&checked(MINIMIZE));
  int n=sel(OUTFMT);
  bool pdbqt=n>=0&&n<(int)outputFormats.size()&&outputFormats[n].first=="pdbqt";
  enable(DOCKMODE,!busy&&!prop&&pdbqt);
  enable(KEEPH,!busy&&!prop&&pdbqt);
}
void setPage(int n){
  page=n;
  select(COORDINATES,0);
  select(HYDROGEN,0);
  select(CHARGE,0);
  select(DOCKMODE,0);
  tick(MINIMIZE,false);
  tick(LARGEST,false);
  tick(SPLIT,false);
  tick(KEEPH,false);
  if(n==0)chooseOutput("sdf");
  if(n==1){
    chooseOutput("sdf");
    select(COORDINATES,2);
    select(HYDROGEN,1);
    tick(MINIMIZE,true);
  }if(n==2){
    chooseOutput("pdbqt");
    select(HYDROGEN,2);
    select(CHARGE,1);
  }if(n==3)select(OUTFMT,(int)outputFormats.size());
  refreshEnabled();
  InvalidateRect(mainWindow,nullptr,TRUE);
}
void setTextMode(bool b){
  textMode=b;
  tick(FILEMODE,!b);
  tick(TEXTMODE,b);
  show(FILES,!b);
  show(SMILES,b);
  refreshEnabled();
}
void refreshFiles(){
  ListView_DeleteAllItems(ctl[FILES]);
  for(size_t i=0;
  i<inputFiles.size();
  i++){
    LVITEMW item{
    };
    wstring name=inputFiles[i].filename().wstring();
    item.mask=LVIF_TEXT;
    item.iItem=(int)i;
    item.pszText=name.data();
    SendMessageW(ctl[FILES],LVM_INSERTITEMW,0,(LPARAM)&item);
    wstring s=L"待处理";
    ListView_SetItemText(ctl[FILES],(int)i,1,s.data());
  }
}
void addFiles(const vector<fs::path>& files){
  for(auto& p:files){
    error_code ec;
    if(fs::is_regular_file(p,ec)&&find(inputFiles.begin(),inputFiles.end(),p)==inputFiles.end())inputFiles.push_back(p);
  }setTextMode(false);
  refreshFiles();
  set(INPUTNOTE,L"已选择 "+to_wstring(inputFiles.size())+L" 个文件 · 可继续拖入文件");
}
void pickFiles(){
  vector<wchar_t> buf(65536);
  OPENFILENAMEW of{
  };
  of.lStructSize=sizeof(of);
  of.hwndOwner=mainWindow;
  of.lpstrFilter=L"化学结构文件\0*.sdf;*.mol;*.smi;*.smiles;*.pdb;*.pdbqt;*.mol2;*.xyz;*.cif;*.inchi\0全部文件\0*.*\0";
  of.lpstrFile=buf.data();
  of.nMaxFile=(DWORD)buf.size();
  of.lpstrTitle=L"选择分子文件（可多选）";
  of.Flags=OFN_EXPLORER|OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR;
  if(!GetOpenFileNameW(&of))return;
  vector<fs::path> paths;
  wstring first=buf.data();
  wchar_t* p=buf.data()+first.size()+1;
  if(!*p)paths.push_back(first);
  else while(*p){
    paths.push_back(fs::path(first)/p);
    p+=wcslen(p)+1;
  }addFiles(paths);
}
void pickFolder(){
  IFileDialog* d=nullptr;
  if(FAILED(CoCreateInstance(CLSID_FileOpenDialog,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&d))))return;
  DWORD flags=0;
  d->GetOptions(&flags);
  d->SetOptions(flags|FOS_PICKFOLDERS|FOS_FORCEFILESYSTEM|FOS_NOCHANGEDIR);
  d->SetTitle(L"选择结果保存位置");
  d->SetOkButtonLabel(L"选择此文件夹");
  if(SUCCEEDED(d->Show(mainWindow))){
    IShellItem* item=nullptr;
    if(SUCCEEDED(d->GetResult(&item))){
      PWSTR p=nullptr;
      if(SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH,&p))){
        set(OUTDIR,p);
        CoTaskMemFree(p);
      }item->Release();
    }
  }d->Release();
}
void layout(){
  int x=194,w=cw-194-302-42,rx=x+w+18,h=ch;
  for(int i=0;
  i<4;
  i++)move(NAV+i,14,111+i*51,152,40);
  move(HELP,18,h-130,144,32);
  move(EXAMPLE,18,h-90,144,32);
  move(ADD,x+w-202,99,106,29);
  move(CLEAR,x+w-87,99,70,29);
  move(FILEMODE,x+18,133,116,24);
  move(TEXTMODE,x+140,133,142,24);
  move(LABINF,x+w-245,137,70,22);
  move(INFMT,x+w-174,132,156,260);
  move(FILES,x+18,165,w-36,80);
  move(SMILES,x+18,165,w-36,80);
  move(INPUTNOTE,x+18,250,w-36,18);
  int c2=x+w/2+6,f1=x+94,f2=c2+82,w1=w/2-116,w2=w/2-106;
  move(LABCOORD,x+18,329,76,22);
  move(COORDINATES,f1,324,w1,250);
  move(LABH,c2,329,80,22);
  move(HYDROGEN,f2,324,w2,230);
  move(LABQUALITY,x+18,361,76,22);
  move(QUALITY,f1,356,w1,230);
  move(LABPH,c2,361,80,22);
  move(PH,f2,356,w2,25);
  move(LABFF,x+18,393,76,22);
  move(FORCEFIELD,f1,388,w1,220);
  move(LABSTEPS,c2,393,80,22);
  move(STEPS,f2,388,w2,25);
  move(MINIMIZE,x+18,425,235,24);
  move(LABCHARGE,c2,425,80,22);
  move(CHARGE,f2,420,w2,230);
  move(LARGEST,x+18,457,242,24);
  move(LABDOCK,c2,457,80,22);
  move(DOCKMODE,f2,452,w2,220);
  move(KEEPH,x+18,485,252,20);
  move(LABOUT,x+18,536,76,22);
  move(OUTFMT,x+94,531,w/2-116,300);
  move(SPLIT,c2,534,w/2-22,23);
  move(OUTDIR,x+18,567,w-118,28);
  move(BROWSE,x+w-91,566,74,29);
  move(OUTNOTE,x+18,599,w-36,16);
  move(RUN,x+18,h-79,160,35);
  move(CANCEL,x+190,h-79,92,35);
  move(OPEN,x+w-168,h-79,151,35);
  move(PREVIEW,rx+12,129,278,170);
  move(SUMMARY,rx+18,362,266,66);
  move(DIAG,rx+198,466,87,25);
  move(LOG,rx+16,503,270,h-550);
  move(PROGRESS,194,h-25,cw-218,5);
  ListView_SetColumnWidth(ctl[FILES],0,px(w-157));
  ListView_SetColumnWidth(ctl[FILES],1,px(102));
  InvalidateRect(mainWindow,nullptr,TRUE);
}
void text(HDC dc,const wstring& s,int x,int y,int w,int h,COLORREF c,HFONT f,UINT flags=DT_LEFT|DT_VCENTER|DT_SINGLELINE){
  SelectObject(dc,f);
  SetTextColor(dc,c);
  SetBkMode(dc,TRANSPARENT);
  RECT r{
    px(x),px(y),px(x+w),px(y+h)
  };
  DrawTextW(dc,s.c_str(),-1,&r,flags);
}
void card(HDC dc,int x,int y,int w,int h){
  HPEN p=CreatePen(PS_SOLID,1,BORDER);
  auto op=SelectObject(dc,p),ob=SelectObject(dc,whiteBrush);
  RoundRect(dc,px(x),px(y),px(x+w),px(y+h),px(12),px(12));
  SelectObject(dc,op);
  SelectObject(dc,ob);
  DeleteObject(p);
}
void paint(HDC dc){
  RECT r;
  GetClientRect(mainWindow,&r);
  FillRect(dc,&r,bgBrush);
  RECT sidebar{
    0,0,px(180),r.bottom
  };
  HBRUSH b=CreateSolidBrush(SIDE);
  FillRect(dc,&sidebar,b);
  DeleteObject(b);
  DrawIconEx(dc,px(20),px(24),(HICON)LoadImageW(instance,MAKEINTRESOURCEW(1),IMAGE_ICON,px(34),px(34),LR_DEFAULTCOLOR),px(34),px(34),0,nullptr,DI_NORMAL);
  text(dc,L"分子工具箱",62,23,111,32,WHITE,fb);
  text(dc,L"Open Babel 3.2.1",21,66,150,20,RGB(161,186,195),fsml);
  text(dc,L"本地运行 · 无需联网",20,ch-40,146,20,RGB(151,178,188),fsml);
  const wchar_t* titles[]={
    L"格式转换",L"三维结构与优化",L"对接格式准备",L"分子属性分析"
  };
  const wchar_t* subtitles[]={
    L"从常用格式到专业格式，批量完成分子文件转换。",L"生成坐标、处理氢原子与电荷，选择力场进行结构优化。",L"导出柔性配体或刚性受体的 PDBQT 文件。",L"输出中文属性表，可直接使用 Excel 打开。"
  };
  int x=194,w=cw-538,rx=x+w+18;
  text(dc,titles[page],x,20,w,35,INK,ft);
  text(dc,subtitles[page],x,59,cw-x-24,22,MUTED,fn);
  card(dc,x,90,w,186);
  card(dc,x,292,w,218);
  card(dc,x,526,w,ch-564);
  card(dc,rx,90,302,235);
  card(dc,rx,341,302,99);
  card(dc,rx,456,302,ch-494);
  text(dc,L"01  输入分子",x+18,102,240,24,INK,fb);
  text(dc,L"02  处理参数",x+18,301,w-36,24,INK,fb);
  text(dc,L"二维结构预览",rx+18,102,264,25,INK,fb);
  text(dc,L"首个成功分子 · 预览不改写输出坐标",rx+18,301,267,17,MUTED,fsml);
  text(dc,L"首个分子的基本信息",rx+18,347,264,20,MUTED,fsml);
  text(dc,L"任务记录",rx+18,467,180,24,INK,fb);
}
LRESULT CALLBACK previewProc(HWND h,UINT msg,WPARAM wp,LPARAM lp){
  if(msg==WM_PAINT){
    PAINTSTRUCT ps;
    HDC dc=BeginPaint(h,&ps);
    RECT r;
    GetClientRect(h,&r);
    FillRect(dc,&r,whiteBrush);
    SetBkMode(dc,TRANSPARENT);
    if(atoms.empty()){
      SelectObject(dc,fn);
      SetTextColor(dc,MUTED);
      r.top+=px(50);
      DrawTextW(dc,L"完成任务后显示结构\n预览限于 200 个重原子以内",-1,&r,DT_CENTER|DT_WORDBREAK);
    }else{
      double minx=atoms[0].x,maxx=minx,miny=atoms[0].y,maxy=miny;
      for(auto& a:atoms){
        minx=min(minx,a.x);
        maxx=max(maxx,a.x);
        miny=min(miny,a.y);
        maxy=max(maxy,a.y);
      }double s=min((r.right-px(52))/max(1.,maxx-minx),(r.bottom-px(38))/max(1.,maxy-miny));
      s=min(s,(double)px(43));
      vector<POINT> pts;
      for(auto& a:atoms)pts.push_back({
        (LONG)((a.x-(minx+maxx)/2)*s+r.right/2),(LONG)(-(a.y-(miny+maxy)/2)*s+r.bottom/2)
      });
      HPEN pen=CreatePen(PS_SOLID,px(2),INK);
      auto old=SelectObject(dc,pen);
      for(auto& b:bonds)if(b.a>0&&b.b>0&&b.a<=(int)pts.size()&&b.b<=(int)pts.size()){
        POINT a=pts[b.a-1],c=pts[b.b-1];
        double dx=c.x-a.x,dy=c.y-a.y,len=max(1.,sqrt(dx*dx+dy*dy));
        int order=b.order>=1&&b.order<=3?b.order:1;
        for(int j=0;
        j<order;
        j++){
          double d=(j-(order-1)/2.)*px(4);
          MoveToEx(dc,(int)(a.x-dy/len*d),(int)(a.y+dx/len*d),nullptr);
          LineTo(dc,(int)(c.x-dy/len*d),(int)(c.y+dx/len*d));
        }
      }SelectObject(dc,old);
      DeleteObject(pen);
      SelectObject(dc,fb);
      for(size_t i=0;
      i<atoms.size();
      i++){
        auto& a=atoms[i];
        if(a.symbol==L"C"&&atoms.size()>1)continue;
        auto p=pts[i];
        RECT t{
          p.x-px(13),p.y-px(10),p.x+px(14),p.y+px(11)
        };
        FillRect(dc,&t,whiteBrush);
        SetTextColor(dc,a.symbol==L"O"?RGB(201,72,59):a.symbol==L"N"?RGB(56,101,190):a.symbol==L"S"?RGB(160,122,25):ACCENT);
        DrawTextW(dc,a.symbol.c_str(),-1,&t,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
      }
    }EndPaint(h,&ps);
    return 0;
  }return DefWindowProcW(h,msg,wp,lp);
}
void drawButton(DRAWITEMSTRUCT* d){
  int id=(int)d->CtlID;
  bool disabled=d->itemState&ODS_DISABLED,pressed=d->itemState&ODS_SELECTED,nav=id>=NAV&&id<NAV+4,selected=nav&&id-NAV==page,primary=id==RUN;
  COLORREF bg=nav?(selected?RGB(37,72,88):SIDE):primary?ACCENT:WHITE,fg=nav||primary?WHITE:INK;
  if(disabled){
    fg=RGB(154,165,170);
    if(!nav)bg=RGB(239,243,245);
  }if(pressed&&!disabled)bg=nav?RGB(38,80,98):RGB(211,234,232);
  HBRUSH br=CreateSolidBrush(bg);
  HPEN pen=CreatePen(PS_SOLID,1,nav||primary?bg:BORDER);
  auto ob=SelectObject(d->hDC,br),op=SelectObject(d->hDC,pen);
  RoundRect(d->hDC,d->rcItem.left,d->rcItem.top,d->rcItem.right,d->rcItem.bottom,px(8),px(8));
  SelectObject(d->hDC,ob);
  SelectObject(d->hDC,op);
  DeleteObject(br);
  DeleteObject(pen);
  SetBkMode(d->hDC,TRANSPARENT);
  SetTextColor(d->hDC,fg);
  SelectObject(d->hDC,primary?fb:fn);
  wstring s=get(id);
  RECT r=d->rcItem;
  DrawTextW(d->hDC,s.c_str(),-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
  if(d->itemState&ODS_FOCUS){
    InflateRect(&r,-px(4),-px(4));
    DrawFocusRect(d->hDC,&r);
  }
}
wstring quote(const wstring& s){
  wstring q=L"\"";
  int slashes=0;
  for(wchar_t c:s){
    if(c==L'\\'){
      slashes++;
      continue;
    }if(c==L'\"'){
      q.append(slashes*2+1,L'\\');
      q+=c;
    }else{
      q.append(slashes,L'\\');
      q+=c;
    }slashes=0;
  }q.append(slashes*2,L'\\');
  return q+L"\"";
}
int runWorker(const fs::path& work){
  SECURITY_ATTRIBUTES sa{
    sizeof(sa),nullptr,TRUE
  };
  HANDLE output=CreateFileW((work/L"status.txt").c_str(),GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr),errors=CreateFileW((work/L"diagnostics.txt").c_str(),GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  if(output==INVALID_HANDLE_VALUE||errors==INVALID_HANDLE_VALUE){
    if(output!=INVALID_HANDLE_VALUE)CloseHandle(output);
    if(errors!=INVALID_HANDLE_VALUE)CloseHandle(errors);
    throw runtime_error("WRITE");
  }HANDLE input=CreateFileW(L"NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
  STARTUPINFOW si{
  };
  si.cb=sizeof(si);
  si.dwFlags=STARTF_USESTDHANDLES;
  si.hStdOutput=output;
  si.hStdError=errors;
  si.hStdInput=input;
  PROCESS_INFORMATION pi{
  };
  fs::path exe=baseDir/L"engine"/L"obengine.exe";
  wstring cmd=quote(exe.wstring())+L" --job job.ini";
  BOOL ok=CreateProcessW(exe.c_str(),cmd.data(),nullptr,nullptr,TRUE,CREATE_NO_WINDOW,nullptr,work.c_str(),&si,&pi);
  CloseHandle(output);
  CloseHandle(errors);
  if(input!=INVALID_HANDLE_VALUE)CloseHandle(input);
  if(!ok)throw runtime_error("LAUNCH");
  ULONGLONG start=GetTickCount64();
  string stop;
  while(WaitForSingleObject(pi.hProcess,60)==WAIT_TIMEOUT){
    if(cancelled){
      stop="CANCEL";
      break;
    }if(GetTickCount64()-start>600000){
      stop="TIMEOUT";
      break;
    }
  }if(!stop.empty()){
    TerminateProcess(pi.hProcess,9);
    WaitForSingleObject(pi.hProcess,5000);
  }DWORD code=0;
  GetExitCodeProcess(pi.hProcess,&code);
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
  if(!stop.empty())throw runtime_error(stop);
  return (int)code;
}
string inferFormat(const fs::path& p){
  string ext=utf8(p.extension().wstring());
  if(!ext.empty())ext.erase(0,1);
  transform(ext.begin(),ext.end(),ext.begin(),[](unsigned char c){
    return (char)tolower(c);
  });
  if(ext=="sd")return "sdf";
  if(ext=="smiles")return "smi";
  if(ext=="ent")return "pdb";
  return ext;
}
wstring stamp(){
  SYSTEMTIME t;
  GetLocalTime(&t);
  wchar_t s[80];
  swprintf(s,80,L"任务_%04d%02d%02d_%02d%02d%02d_%03d",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
  return s;
}
void work(Settings settings){
  fs::path temp,output;
  int successes=0,failures=0,molecules=0;
  bool first=true;
  wstring report;
  try{
    wchar_t buf[32768];
    GetTempPathW(32768,buf);
    temp=fs::path(buf)/(L"OpenBabelCN_"+to_wstring(GetCurrentProcessId())+L"_"+to_wstring(GetTickCount64()));
    fs::create_directories(temp);
    auto data=baseDir/L"engine"/L"data";
    if(!fs::is_directory(data))throw runtime_error("DATA");
    fs::copy(data,temp/L"data",fs::copy_options::recursive);
    output=settings.output/stamp();
    fs::create_directories(output);
    post(5,L"",0,0,output);
    size_t total=settings.text?1:settings.files.size();
    for(size_t i=0;
    i<total&&!cancelled;
    i++){
      auto task=temp/(L"job_"+to_wstring(i));
      fs::create_directories(task);
      wstring stem=settings.text?L"SMILES输入":settings.files[i].stem().wstring();
      wchar_t prefix[24];
      swprintf(prefix,24,L"%03d_",(int)i+1);
      stem=prefix+stem;
      post(0,L"正在处理："+stem);
      post(1,L"处理中",(int)i,(int)total);
      map<string,string> opts=settings.options;
      try{
        if(settings.text){
          writeFile(task/L"input.dat",utf8(settings.smiles)+"\n");
          opts["informat"]="smi";
        }else{
          fs::copy_file(settings.files[i],task/L"input.dat");
          if(opts["informat"]=="auto")opts["informat"]=inferFormat(settings.files[i]);
        }ostringstream ini;
        for(auto& p:opts)ini<<p.first<<'='<<p.second<<'\n';
        writeFile(task/L"job.ini",ini.str());
        int code=runWorker(task);
        string statusText=readFile(task/L"status.txt");
        auto err=statusText.rfind("ERROR\t");
        if(err!=string::npos){
          string e=statusText.substr(err+6);
          e=e.substr(0,e.find_first_of("\r\n"));
          throw runtime_error(e);
        }auto ok=statusText.rfind("OK\t");
        if(code!=0||ok==string::npos)throw runtime_error("CRASH");
        int count=0,warnings=0;
        istringstream status(statusText.substr(ok+3));
        status>>count>>warnings;
        if(count<=0)throw runtime_error("EMPTY");
        vector<fs::path> results;
        for(auto& entry:fs::directory_iterator(task)){
          auto name=entry.path().filename().wstring();
          if(name.rfind(L"result",0)==0&&entry.is_regular_file()&&entry.file_size()>0)results.push_back(entry.path());
        }if(results.empty())throw runtime_error("WRITE");
        // Stage each complete input before publishing it. Failed and cancelled inputs never replace results.
        auto staging=output/(L".处理中_"+to_wstring(i));
        fs::create_directories(staging);
        for(auto& p:results){
          wstring name=stem+p.filename().wstring().substr(6);
          fs::copy_file(p,staging/name);
        }auto finalFolder=output/stem;
        fs::rename(staging,finalFolder);
        successes++;
        molecules+=count;
        wstring message=L"完成："+stem+L"，"+to_wstring(count)+L" 个分子";
        if(warnings>0){
          message+=L"（有提示，请查看诊断）";
          writeFile(finalFolder/L"引擎提示.txt",string("\xEF\xBB\xBF")+"以下为 Open Babel 原始诊断信息：\r\n"+readFile(task/L"diagnostics.txt"));
        }post(0,message);
        post(1,L"完成 · "+to_wstring(count),(int)i,(int)total);
        report+=message+L"\r\n";
        if(first){
          for(const wchar_t* name:{
            L"preview.txt",L"summary.txt"
          })if(fs::exists(task/name))fs::copy_file(task/name,output/name);
          post(2,L"",0,0,output);
          first=false;
        }
      }catch(const exception& e){
        if(string(e.what())=="CANCEL"){
          post(1,L"已取消",(int)i,(int)total);
          break;
        }failures++;
        wstring message=L"失败："+stem+L"。"+errorText(e.what());
        post(0,message);
        post(1,L"失败",(int)i,(int)total);
        report+=message+L"\r\n";
        error_code ec;
        fs::remove_all(output/(L".处理中_"+to_wstring(i)),ec);
        writeFile(output/(stem+L"_诊断.txt"),string("\xEF\xBB\xBF")+utf8(message)+"\r\n\r\nOpen Babel 原始记录：\r\n"+readFile(task/L"diagnostics.txt"));
      }
    }
    wstring summary=(cancelled?L"已停止。":L"任务结束。")+wstring(L"成功 ")+to_wstring(successes)+L" 个文件，失败 "+to_wstring(failures)+L" 个文件，共 "+to_wstring(molecules)+L" 个分子。";
    writeFile(output/L"任务汇总.txt",string("\xEF\xBB\xBF")+utf8(summary+L"\r\n\r\n"+report));
    post(0,summary);
    post(3,summary);
  }catch(const exception& e){
    post(0,errorText(e.what()));
    post(3,L"任务未完成。"+errorText(e.what()));
  }error_code ec;
  if(!temp.empty())fs::remove_all(temp,ec);
}
void start(){
  if(busy)return;
  if(!fs::exists(baseDir/L"engine"/L"obengine.exe")){
    MessageBoxW(mainWindow,L"未找到内置引擎，请完整解压后再运行。",L"无法启动",MB_OK|MB_ICONWARNING);
    return;
  }if(textMode&&get(SMILES).find_first_not_of(L" \t\r\n")==wstring::npos){
    MessageBoxW(mainWindow,L"请先输入 SMILES，每行一个分子。",L"需要输入",MB_OK);
    return;
  }if(!textMode&&inputFiles.empty()){
    pickFiles();
    if(inputFiles.empty())return;
  }if(get(OUTDIR).empty()){
    MessageBoxW(mainWindow,L"请选择保存位置。",L"需要输出目录",MB_OK);
    return;
  }
  Settings s;
  s.page=page;
  s.text=textMode;
  s.smiles=get(SMILES);
  s.files=inputFiles;
  s.output=get(OUTDIR);
  auto& o=s.options;
  int out=sel(OUTFMT),in=sel(INFMT);
  if(page!=3&&(out<0||out>=(int)outputFormats.size())){
    MessageBoxW(mainWindow,L"属性表请在“分子属性”中生成。",L"请选择结构格式",MB_OK);
    return;
  }o["informat"]=in>=0?inputFormats[in].first:"auto";
  o["outformat"]=page==3?"csv":outputFormats[out].first;
  const vector<string> coords={
    "keep","2d","3d"
  },hs={
    "keep","all","polar","remove","ph"
  },charges={
    "keep","gasteiger","mmff94","qeq"
  },quality={
    "fast","medium","best"
  },ffs={
    "MMFF94","MMFF94s","UFF","GAFF","Ghemical"
  };
  o["coordinates"]=coords[max(0,sel(COORDINATES))];
  o["hydrogen"]=hs[max(0,sel(HYDROGEN))];
  o["charge"]=charges[max(0,sel(CHARGE))];
  o["quality"]=quality[max(0,sel(QUALITY))];
  o["forcefield"]=ffs[max(0,sel(FORCEFIELD))];
  o["ph"]=utf8(get(PH));
  o["steps"]=utf8(get(STEPS));
  o["minimize"]=checked(MINIMIZE)?"1":"0";
  o["largest"]=checked(LARGEST)?"1":"0";
  o["split"]=checked(SPLIT)?"1":"0";
  o["properties"]=page==3?"1":"0";
  o["rigid"]=sel(DOCKMODE)==1?"1":"0";
  o["preserve_h"]=checked(KEEPH)?"1":"0";
  o["preserve_names"]=sel(DOCKMODE)==1?"1":"0";
  try{
    if(o["hydrogen"]=="ph"){
      size_t p;
      double n=stod(o["ph"],&p);
      if(p!=o["ph"].size()||!isfinite(n)||n<0||n>14)throw runtime_error("PH");
    }if(o["minimize"]=="1"){
      size_t p;
      int n=stoi(o["steps"],&p);
      if(p!=o["steps"].size()||n<1||n>100000)throw runtime_error("STEPS");
    }
  }catch(...){
    MessageBoxW(mainWindow,L"请检查酸碱度（0–14）及优化步数（1–100000 的整数）。",L"参数有误",MB_OK|MB_ICONWARNING);
    return;
  }
  if(page!=3&&o["coordinates"]=="2d"&&(o["minimize"]=="1"||o["outformat"]=="pdbqt")){
    MessageBoxW(mainWindow,L"二维坐标不能用于三维优化或对接输出，请生成三维坐标或保留已有三维坐标。",L"参数冲突",MB_OK|MB_ICONWARNING);
    return;
  }
  if(page==3){
    o["coordinates"]="keep";
    o["hydrogen"]="keep";
    o["charge"]="keep";
    o["minimize"]="0";
    o["largest"]="0";
    o["split"]="0";
  }
  error_code ec;
  fs::create_directories(configFile.parent_path(),ec);
  if(!fs::exists(configFile,ec)){
    ofstream settingsFile(configFile,ios::binary);
    settingsFile.write("\xFF\xFE",2); // UTF-16 profile preserves Chinese paths on any system locale.
  }
  WritePrivateProfileStringW(L"设置",L"输出目录",get(OUTDIR).c_str(),configFile.c_str());
  if(jobThread.joinable())jobThread.join();
  cancelled=false;
  busy=true;
  lastOutput.clear();
  set(LOG,L"");
  atoms.clear();
  bonds.clear();
  set(SUMMARY,L"等待结果…");
  InvalidateRect(ctl[PREVIEW],nullptr,TRUE);
  refreshFiles();
  refreshEnabled();
  SendMessageW(ctl[PROGRESS],PBM_SETMARQUEE,TRUE,35);
  log(L"开始任务。结果保存到独立文件夹。二维预览不会改写输出坐标。");
  jobThread=thread(work,std::move(s));
}
void loadPreview(const fs::path& p){
  atoms.clear();
  bonds.clear();
  ifstream f(p/L"preview.txt");
  int n=0,m=0;
  f>>n>>m;
  if(n>0&&n<=200&&m>=0&&m<=800){
    for(int i=0;
    i<n;
    i++){
      string s;
      double x,y;
      if(!(f>>s>>x>>y))break;
      atoms.push_back({
        wide(s),x,y
      });
    }for(int i=0;
    i<m;
    i++){
      Bond b;
      if(!(f>>b.a>>b.b>>b.order))break;
      bonds.push_back(b);
    }
  }istringstream summary(readFile(p/L"summary.txt"));
  string formula,mass,heavy,rotors;
  getline(summary,formula);
  getline(summary,mass);
  getline(summary,heavy);
  getline(summary,rotors);
  set(SUMMARY,L"分子式   "+wide(formula)+L"\r\n相对分子质量   "+wide(mass)+L"\r\n重原子   "+wide(heavy)+L"      可旋转键   "+wide(rotors));
  InvalidateRect(ctl[PREVIEW],nullptr,TRUE);
}
void openPath(const fs::path& p){
  if(!p.empty())ShellExecuteW(mainWindow,L"open",p.c_str(),nullptr,nullptr,SW_SHOWNORMAL);
}
void createControls(){
  const wchar_t* nav[]={
    L"格式转换",L"三维与优化",L"对接格式",L"分子属性"
  };
  for(int i=0;
  i<4;
  i++)button(NAV+i,nav[i]);
  button(HELP,L"使用说明");
  button(EXAMPLE,L"载入示例");
  button(ADD,L"添加文件");
  button(CLEAR,L"清空");
  make(FILEMODE,L"BUTTON",L"文件输入",BS_AUTORADIOBUTTON|WS_TABSTOP|WS_GROUP);
  make(TEXTMODE,L"BUTTON",L"粘贴 SMILES",BS_AUTORADIOBUTTON|WS_TABSTOP);
  make(FILES,WC_LISTVIEWW,L"",LVS_REPORT|LVS_SINGLESEL|LVS_SHOWSELALWAYS|WS_TABSTOP,WS_EX_CLIENTEDGE);
  ListView_SetExtendedListViewStyle(ctl[FILES],LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);
  for(int i=0;
  i<2;
  i++){
    LVCOLUMNW c{
    };
    c.mask=LVCF_TEXT|LVCF_WIDTH;
    c.cx=px(260);
    c.pszText=(LPWSTR)(i==0?L"文件名":L"处理结果");
    SendMessageW(ctl[FILES],LVM_INSERTCOLUMNW,i,(LPARAM)&c);
  }make(SMILES,L"EDIT",L"",ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL|WS_TABSTOP|ES_WANTRETURN,WS_EX_CLIENTEDGE);
  SendMessageW(ctl[SMILES],EM_SETLIMITTEXT,4*1024*1024,0);
  SendMessageW(ctl[SMILES],WM_SETFONT,(WPARAM)fmono,TRUE);
  label(LABINF,L"输入格式");
  combo(INFMT,{
  });
  label(INPUTNOTE,L"支持批量文件或粘贴 SMILES，每行一个分子。");
  SendMessageW(ctl[INPUTNOTE],WM_SETFONT,(WPARAM)fsml,TRUE);
  label(LABCOORD,L"坐标处理");
  combo(COORDINATES,{
    L"保留原坐标",L"生成二维坐标",L"生成三维坐标"
  });
  label(LABH,L"氢原子");
  combo(HYDROGEN,{
    L"保持不变",L"添加全部氢",L"添加极性氢",L"去除氢",L"按酸碱度加氢"
  });
  label(LABQUALITY,L"三维质量");
  combo(QUALITY,{
    L"快速",L"均衡",L"精细（较慢）"
  });
  label(LABPH,L"酸碱度");
  edit(PH,L"7.4");
  label(LABFF,L"优化力场");
  combo(FORCEFIELD,{
    L"MMFF94",L"MMFF94s",L"UFF",L"GAFF",L"Ghemical"
  });
  label(LABSTEPS,L"优化步数");
  edit(STEPS,L"500",ES_NUMBER);
  checkbox(MINIMIZE,L"能量最小化（自动补氢）");
  label(LABCHARGE,L"部分电荷");
  combo(CHARGE,{
    L"保持不变",L"Gasteiger",L"MMFF94",L"QEq"
  });
  checkbox(LARGEST,L"仅保留最大连通片段（去盐）");
  label(LABDOCK,L"对接模式");
  combo(DOCKMODE,{
    L"柔性配体",L"刚性受体"
  });
  checkbox(KEEPH,L"对接输出保留全部显式氢");
  SendMessageW(ctl[KEEPH],WM_SETFONT,(WPARAM)fsml,TRUE);
  label(LABOUT,L"输出格式");
  combo(OUTFMT,{
  });
  checkbox(SPLIT,L"每个分子单独成文件");
  edit(OUTDIR,L"");
  button(BROWSE,L"浏览…");
  label(OUTNOTE,L"建立独立任务目录，保留原文件；按输入文件归档。");
  SendMessageW(ctl[OUTNOTE],WM_SETFONT,(WPARAM)fsml,TRUE);
  button(RUN,L"开始处理");
  button(CANCEL,L"取消任务");
  button(OPEN,L"打开结果目录");
  button(DIAG,L"诊断目录");
  make(LOG,L"EDIT",L"",ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL|WS_VSCROLL);
  SendMessageW(ctl[LOG],EM_SETLIMITTEXT,1024*1024,0);
  SendMessageW(ctl[LOG],WM_SETFONT,(WPARAM)fsml,TRUE);
  make(PREVIEW,L"MoleculePreviewCN",L"");
  make(SUMMARY,L"STATIC",L"分子式   —\r\n相对分子质量   —\r\n重原子   —      可旋转键   —",SS_LEFT);
  make(PROGRESS,PROGRESS_CLASSW,L"",PBS_MARQUEE);
  SendMessageW(ctl[PROGRESS],PBM_SETBARCOLOR,0,ACCENT);
  loadFormats();
  wchar_t path[32768];
  GetPrivateProfileStringW(L"设置",L"输出目录",L"",path,32768,configFile.c_str());
  if(*path)set(OUTDIR,path);
  else{
    PWSTR doc=nullptr;
    if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents,0,nullptr,&doc))){
      set(OUTDIR,(fs::path(doc)/L"OpenBabel输出").wstring());
      CoTaskMemFree(doc);
    }
  }setTextMode(false);
  setPage(0);
  log(L"内置 Open Babel 3.2.1 已就绪。请选择文件或粘贴 SMILES。");
}
LRESULT CALLBACK windowProc(HWND h,UINT msg,WPARAM wp,LPARAM lp){
  switch(msg){
    case WM_CREATE:mainWindow=h;
    createControls();
    DragAcceptFiles(h,TRUE);
    return 0;
    case WM_SIZE:cw=MulDiv(LOWORD(lp),96,dpi);
    ch=MulDiv(HIWORD(lp),96,dpi);
    if(!ctl.empty())layout();
    return 0;
    case WM_GETMINMAXINFO:{
      auto* m=(MINMAXINFO*)lp;
      m->ptMinTrackSize={
        px(1120),px(735)
      };
      return 0;
    }case WM_ERASEBKGND:return 1;
    case WM_PAINT:{
      PAINTSTRUCT ps;
      HDC dc=BeginPaint(h,&ps);
      paint(dc);
      EndPaint(h,&ps);
      return 0;
    }case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:{
      HDC dc=(HDC)wp;
      SetTextColor(dc,INK);
      SetBkColor(dc,WHITE);
      return (LRESULT)whiteBrush;
    }case WM_DRAWITEM:drawButton((DRAWITEMSTRUCT*)lp);
    return TRUE;
    case WM_DROPFILES:{
      HDROP drop=(HDROP)wp;
      if(!busy){
        UINT count=DragQueryFileW(drop,0xFFFFFFFF,nullptr,0);
        vector<fs::path> paths;
        for(UINT i=0;
        i<count;
        i++){
          UINT n=DragQueryFileW(drop,i,nullptr,0);
          wstring p(n+1,0);
          DragQueryFileW(drop,i,p.data(),n+1);
          p.resize(n);
          paths.emplace_back(p);
        }addFiles(paths);
      }DragFinish(drop);
      return 0;
    }
    case WM_COMMAND:{
      int id=LOWORD(wp),code=HIWORD(wp);
      if(id>=NAV&&id<NAV+4&&!busy){
        setPage(id-NAV);
        return 0;
      }if(code==CBN_SELCHANGE||code==BN_CLICKED){
        if(id==FILEMODE)setTextMode(false);
        if(id==TEXTMODE)setTextMode(true);
        if(id==ADD)pickFiles();
        if(id==CLEAR){
          inputFiles.clear();
          refreshFiles();
          set(INPUTNOTE,L"支持批量文件或粘贴 SMILES，每行一个分子。");
        }if(id==BROWSE)pickFolder();
        if(id==RUN)start();
        if(id==CANCEL){
          cancelled=true;
          enable(CANCEL,false);
          log(L"正在取消当前计算…");
        }if(id==OPEN||id==DIAG)openPath(lastOutput);
        if(id==HELP)openPath(baseDir/L"使用说明.html");
        if(id==EXAMPLE){
          setTextMode(true);
          set(SMILES,L"CCO 乙醇\r\nCC(=O)Oc1ccccc1C(=O)O 阿司匹林\r\nCn1c(=O)c2c(ncn2C)n(C)c1=O 咖啡因");
          log(L"已载入 3 个示例分子，选择功能后点击“开始处理”。");
        }if(id==COORDINATES||id==HYDROGEN||id==MINIMIZE||id==OUTFMT)refreshEnabled();
        if(id==DOCKMODE&&sel(DOCKMODE)==1){
          select(COORDINATES,0);
          tick(MINIMIZE,false);
          tick(LARGEST,false);
          log(L"刚性受体模式保留输入坐标。请预先完成受体修复及配体、水分子选择。");
          refreshEnabled();
        }
      }return 0;
    }
    case WM_EVENT:{
      auto* e=(Event*)lp;
      if(e->kind==0)log(e->text);
      if(e->kind==1&&!textMode&&e->index<(int)inputFiles.size())ListView_SetItemText(ctl[FILES],e->index,1,e->text.data());
      if(e->kind==2)loadPreview(e->path);
      if(e->kind==5){
        lastOutput=e->path;
        enable(OPEN,true);
        enable(DIAG,true);
      }if(e->kind==3){
        busy=false;
        SendMessageW(ctl[PROGRESS],PBM_SETMARQUEE,FALSE,0);
        refreshEnabled();
      }delete e;
      return 0;
    }
    case WM_CLOSE:if(busy&&MessageBoxW(h,L"任务仍在运行，是否取消并关闭？已成功导出的文件会保留。",L"关闭程序",MB_YESNO|MB_ICONQUESTION)!=IDYES)return 0;
    cancelled=true;
    if(jobThread.joinable())jobThread.join();
    DestroyWindow(h);
    return 0;
    case WM_DESTROY:PostQuitMessage(0);
    return 0;
  }return DefWindowProcW(h,msg,wp,lp);
}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,PWSTR,int show){
  instance=inst;
  SetProcessDPIAware();
  HDC screen=GetDC(nullptr);
  dpi=GetDeviceCaps(screen,LOGPIXELSX);
  ReleaseDC(nullptr,screen);
  RECT fit;
  SystemParametersInfoW(SPI_GETWORKAREA,0,&fit,0);
  dpi=min(dpi,max(72,min(MulDiv(fit.right-fit.left,96,1120),MulDiv(fit.bottom-fit.top,96,735))));
  CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
  INITCOMMONCONTROLSEX cc{
    sizeof(cc),ICC_LISTVIEW_CLASSES|ICC_PROGRESS_CLASS
  };
  InitCommonControlsEx(&cc);
  wchar_t path[32768];
  GetModuleFileNameW(nullptr,path,32768);
  baseDir=fs::path(path).parent_path();
  PWSTR local=nullptr;
  if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData,0,nullptr,&local))){
    configFile=fs::path(local)/L"OpenBabelCN"/L"settings.ini";
    CoTaskMemFree(local);
  }else configFile=baseDir/L"settings.ini";
  SetEnvironmentVariableW(L"BABEL_DATADIR",L"..\\data");
  auto font=[](int size,int weight,const wchar_t* face){
    return CreateFontW(-px(size),0,0,0,weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,face);
  };
  fn=font(14,FW_NORMAL,L"Microsoft YaHei UI");
  fsml=font(12,FW_NORMAL,L"Microsoft YaHei UI");
  fb=font(15,FW_SEMIBOLD,L"Microsoft YaHei UI");
  ft=font(27,FW_SEMIBOLD,L"Microsoft YaHei UI");
  fmono=font(14,FW_NORMAL,L"Consolas");
  whiteBrush=CreateSolidBrush(WHITE);
  bgBrush=CreateSolidBrush(BG);
  WNDCLASSW pc{
  };
  pc.lpfnWndProc=previewProc;
  pc.hInstance=inst;
  pc.lpszClassName=L"MoleculePreviewCN";
  pc.hCursor=LoadCursor(nullptr,IDC_ARROW);
  RegisterClassW(&pc);
  WNDCLASSEXW wc{
  };
  wc.cbSize=sizeof(wc);
  wc.hInstance=inst;
  wc.lpfnWndProc=windowProc;
  wc.lpszClassName=L"OpenBabelCNMain";
  wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
  wc.hIcon=(HICON)LoadImageW(inst,MAKEINTRESOURCEW(1),IMAGE_ICON,0,0,LR_DEFAULTSIZE);
  wc.hIconSm=wc.hIcon;
  RegisterClassExW(&wc);
  RECT area;
  SystemParametersInfoW(SPI_GETWORKAREA,0,&area,0);
  int width=min(px(1240),(int)(area.right-area.left)),height=min(px(790),(int)(area.bottom-area.top));
  HWND h=CreateWindowExW(0,wc.lpszClassName,L"OpenBabel 中文工作台",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN,(area.right-width)/2,(area.bottom-height)/2,width,height,nullptr,nullptr,inst,nullptr);
  ShowWindow(h,show);
  UpdateWindow(h);
  MSG m;
  while(GetMessageW(&m,nullptr,0,0)>0){
    if(!IsDialogMessageW(h,&m)){
      TranslateMessage(&m);
      DispatchMessageW(&m);
    }
  }CoUninitialize();
  return (int)m.wParam;
}
