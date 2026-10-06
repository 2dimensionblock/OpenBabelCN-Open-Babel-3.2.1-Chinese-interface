"""Extract the supplied source and apply the documented build and coordinate write-back patches."""
from pathlib import Path
import hashlib, zipfile
root=Path(__file__).resolve().parents[1]
archive=root/'vendor'/'openbabel-openbabel-3-2-1.zip'
source=root/'vendor'/'openbabel'
expected='b97271d69c50556eb3698edd84d760c706b425985ae527d6b71c31ed9fbc86a8'
if hashlib.sha256(archive.read_bytes()).hexdigest()!=expected:raise ValueError('Unexpected Open Babel source archive')
if not source.exists():
 with zipfile.ZipFile(archive) as z:
  # The distribution includes the complete unchanged upstream archive.
  for i in z.infolist():
   relative=Path(*Path(i.filename).parts[1:])
   if not relative.parts:continue
   destination=(source/relative).resolve()
   if source.resolve() not in destination.parents:raise ValueError('Unsafe archive entry')
   if i.is_dir():destination.mkdir(parents=True,exist_ok=True)
   else:destination.parent.mkdir(parents=True,exist_ok=True);destination.write_bytes(z.read(i))
p=source/'data'/'bin2hex.pl';s=p.read_text();s=s.replace('$| = 1;', '$| = 0; # Buffered output for reliable and faster header generation.');p.write_text(s)
p=source/'src'/'formats'/'libinchi'/'inchi_dll.c';s=p.read_text();s=s.replace('#if( defined(__GNUC__) && __GNUC__ >= 3 && defined(__MINGW32__) && defined(_WIN32) )','#if( defined(__GNUC__) && __GNUC__ >= 3 && defined(__MINGW32__) && defined(_WIN32) && defined(BUILD_LINK_AS_DLL) )');p.write_text(s)
p=source/'src'/'ops'/'gen3d.cpp';s=p.read_text()
s=s.replace('if (speed == 5)\n      return true; // done','if (speed == 5) {\n      *pmol = molCopy;\n      return true; // done\n    }').replace('if (!pFF)\n      return true;','if (!pFF) {\n      *pmol = molCopy;\n      return true;\n    }').replace('if (!pFF || !pFF->Setup(molCopy)) return true;', 'if (!pFF || !pFF->Setup(molCopy)) { *pmol = molCopy; return true; }').replace('pFF->GetCoordinates(molCopy);\n      return true; // no conformer searching','pFF->GetCoordinates(molCopy);\n      *pmol = molCopy;\n      return true; // no conformer searching');p.write_text(s)
print('Source ready:',source)
