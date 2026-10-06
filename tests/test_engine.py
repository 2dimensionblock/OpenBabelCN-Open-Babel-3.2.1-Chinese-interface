"""Behavior tests; runs the same engine on Linux or Windows. Python stdlib only."""
import argparse, csv, json, math, os, pathlib, subprocess, tempfile, time, shutil
p=argparse.ArgumentParser();p.add_argument('--engine',required=True);p.add_argument('--data',required=True);p.add_argument('--report',required=True);args=p.parse_args()
engine=pathlib.Path(args.engine).resolve();data=pathlib.Path(args.data).resolve();results=[]
root=pathlib.Path(tempfile.mkdtemp(prefix='obcn-tests-'));shutil.copytree(data,root/'data')
env=dict(os.environ,BABEL_DATADIR='../data')
def job(name,content,options,expect=None,binary=False):
 d=root/name;d.mkdir();(d/'input.dat').write_bytes(content if binary else content.encode())
 opts={'informat':'smi','outformat':'sdf',**options};(d/'job.ini').write_text('\n'.join(k+'='+str(v) for k,v in opts.items())+'\n')
 r=subprocess.run([str(engine),'--job','job.ini'],cwd=d,env=env,capture_output=True,text=True,timeout=90)
 if expect: assert r.returncode!=0 and 'ERROR\t'+expect in r.stdout, (name,r.returncode,r.stdout,r.stderr)
 else: assert r.returncode==0 and 'OK\t' in r.stdout,(name,r.returncode,r.stdout,r.stderr)
 return d,r

def check(name,fn):
 start=time.monotonic()
 try:fn();results.append({'test':name,'passed':True,'seconds':round(time.monotonic()-start,2)});print('PASS',name,flush=True)
 except Exception as e:results.append({'test':name,'passed':False,'error':str(e)});print('FAIL',name,str(e)[:1000],flush=True)

def formats():
 r=subprocess.run([str(engine),'--formats'],capture_output=True,text=True,env=dict(os.environ,BABEL_DATADIR=str(data)),timeout=30);assert r.returncode==0
 f={s.split('\t')[0]:s.split('\t')[1:] for s in r.stdout.splitlines() if '\t' in s}
 for k in ['sdf','smi','pdb','pdbqt','mol2','xyz','inchi','svg']:assert k in f,k
 (root/'formats.tsv').write_text(r.stdout)
check('核心格式注册',formats)
example='CCO 乙醇\nCC(=O)Oc1ccccc1C(=O)O 阿司匹林\nCn1c(=O)c2c(ncn2C)n(C)c1=O 咖啡因\n'
def multi():
 d,r=job('multi',example,{'coordinates':'2d'});s=(d/'result.sdf').read_text();assert s.count('$$$$')==3;assert '乙醇' in s and '阿司匹林' in s
 d2,r=job('roundtrip',s,{'informat':'sdf','outformat':'smi'});assert len((d2/'result.smi').read_text().strip().splitlines())==3
check('多分子二维转换与往返',multi)
def props():
 d,r=job('props',example,{'properties':'1'});rows=list(csv.DictReader((d/'result.csv').open(encoding='utf-8-sig',newline='')))
 assert len(rows)==3;assert [x['分子式'] for x in rows]==['C2H6O','C9H8O4','C8H10N4O2'];assert abs(float(rows[0]['相对分子质量'])-46.068)<.01
 assert rows[0]['氢键供体数']=='1';assert rows[0]['氢键受体数（HBA1）']=='1';assert abs(float(rows[0]['拓扑极性表面积（平方埃）'])-20.23)<.1
check('中文属性表与已知分子性质',props)
def hydrogens():
 d,r=job('addh','CCO\n',{'hydrogen':'all'});s=(d/'result.sdf').read_text();assert int(s.splitlines()[3][:3])==9
 d2,r=job('removeh',s,{'informat':'sdf','hydrogen':'remove'});assert int((d2/'result.sdf').read_text().splitlines()[3][:3])==3
 d3,r=job('removeh_after_3d','CCO\n',{'coordinates':'3d','hydrogen':'remove'});assert int((d3/'result.sdf').read_text().splitlines()[3][:3])==3
check('添加与去除显式氢',hydrogens)
def ph():
 d,r=job('ph','CC(=O)O\n',{'hydrogen':'ph','ph':'7.4','outformat':'can'});assert '[O-]' in (d/'result.can').read_text()
check('酸碱度质子化',ph)
def optimize():
 d,r=job('optimize','CCO ethanol\nCC(=O)Oc1ccccc1C(=O)O aspirin\n',{'coordinates':'3d','quality':'fast','hydrogen':'all','minimize':'1','steps':'100','forcefield':'MMFF94'});s=(d/'result.sdf').read_text();assert s.count('$$$$')==2;assert 'Energy' in s
 for mol in s.split('$$$$')[:2]:
  lines=mol.strip('\r\n').splitlines();n=int(lines[3][:3]);xyz=[[float(row[i:i+10]) for i in (0,10,20)] for row in lines[4:4+n]];assert all(math.isfinite(c) for row in xyz for c in row);assert any(abs(row[2])>.001 for row in xyz)
check('三维生成与 MMFF94 优化',optimize)
def dock():
 d,r=job('ligand','CCCCO ligand\n',{'coordinates':'3d','hydrogen':'polar','charge':'gasteiger','outformat':'pdbqt'});s=(d/'result.pdbqt').read_text();assert 'ROOT' in s and 'TORSDOF' in s and 'BRANCH' in s
 d,r=job('rigid','CCCCO receptor_test\n',{'coordinates':'3d','charge':'gasteiger','outformat':'pdbqt','rigid':'1'});s=(d/'result.pdbqt').read_text();assert 'ROOT' not in s and 'BRANCH' not in s and 'ATOM' in s
check('柔性与刚性 PDBQT',dock)
def charge():
 d,r=job('charge','CCO\n',{'charge':'gasteiger','outformat':'mol2'});s=(d/'result.mol2').read_text();rows=s.split('@<TRIPOS>ATOM')[1].split('@<TRIPOS>BOND')[0].strip().splitlines();charges=[float(x.split()[-1]) for x in rows];assert all(math.isfinite(x) for x in charges) and any(abs(x)>.01 for x in charges)
check('部分电荷写入 MOL2',charge)
def split():
 d,r=job('split',example,{'split':'1','outformat':'mol'});assert len(list(d.glob('result_*.mol')))==3
 job('single_only',example,{'outformat':'mol'},expect='SINGLE_ONLY')
check('分子拆分与单分子格式保护',split)
def largest():
 d,r=job('largest','CCO.[Na+]\n',{'largest':'1','outformat':'can'});assert 'Na' not in (d/'result.can').read_text()
check('最大连通片段',largest)
def invalid():
 job('bad','not_a_smiles\n',{},expect='EMPTY');job('partial','CCO\nNOT_A_MOLECULE\n',{},expect='PARSE');job('format','CCO\n',{'informat':'unsupported'},expect='INPUT_FORMAT');job('need3d','CCO\n',{'minimize':'1'},expect='NEED3D')
check('无效输入与参数失败处理',invalid)
def inchi():
 d,r=job('inchi','CCO\n',{'outformat':'inchi'});assert 'InChI=1S/C2H6O/' in (d/'result.inchi').read_text()
check('InChI 生成',inchi)
def unicode():
 # Engine accepts relative ASCII working names inside Unicode directories.
 d,r=job('中文路径与空格 test','CCO 测试分子\n',{'properties':'1'});assert '测试分子' in (d/'result.csv').read_text(encoding='utf-8-sig')
check('中文路径与名称',unicode)
report={'engine':str(engine),'platform':os.name,'total':len(results),'passed':sum(x['passed'] for x in results),'results':results,'workspace':str(root)}
pathlib.Path(args.report).write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8');print(json.dumps({'passed':report['passed'],'total':report['total'],'workspace':str(root)},ensure_ascii=False))
raise SystemExit(0 if report['passed']==report['total'] else 1)
