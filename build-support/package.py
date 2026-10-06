from pathlib import Path
import argparse, shutil, subprocess, os
p=argparse.ArgumentParser();p.add_argument('--build',required=True);p.add_argument('--format-engine');p.add_argument('--destination',required=True);a=p.parse_args()
root=Path(__file__).resolve().parents[1];build=Path(a.build).resolve();out=Path(a.destination).resolve();(out/'engine').mkdir(parents=True,exist_ok=True)
shutil.copy2(build/'bin'/'OpenBabelCN.exe',out/'OpenBabelCN.exe')
for f in ['obengine.exe','obabel.exe']:shutil.copy2(build/'bin'/f,out/'engine'/f)
data=root/'vendor'/'openbabel'/'data';shutil.copytree(data,out/'engine'/'data',dirs_exist_ok=True)
exe=Path(a.format_engine).resolve() if a.format_engine else out/'engine'/'obengine.exe'
r=subprocess.run([str(exe),'--formats'],capture_output=True,env=dict(os.environ,BABEL_DATADIR=str(data)),check=True);(out/'engine'/'formats.tsv').write_bytes(r.stdout)
for f in ['使用说明.html','先读我.txt','验证说明.txt']:
 if (root/f).exists():shutil.copy2(root/f,out/f)
for f in ['licenses','examples']:
 if (root/f).exists():shutil.copytree(root/f,out/f,dirs_exist_ok=True)
print(out)
