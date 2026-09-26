#!/usr/bin/env python3
"""Small, standalone build for the pinned Lua IIgs release SDK (Python 3.9+)."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import urllib.request
import zipfile

ROOT = Path(__file__).resolve().parents[1]
DEPS = ROOT / '.deps'
BUILD = ROOT / 'build'
SDK_NAME = 'lua-iigs-0.3.0-sdk.zip'
SDK_SHA256 = 'bc12d3da7b0d5d11e67d3d6997ae245b1ca06684c267e5dc19f6a6082119fcda'
SDK_URL = 'https://github.com/artgreen/lua-iigs/releases/download/v0.3.0/' + SDK_NAME
DEMO_INPUTS = ('config.lua', 'coltest.lua', 'stattest.lua', 'shopdemo.lua', 'policy.lua', 'stock.csv')
UNITS = ('main', 'collection', 'luacollection', 'luastatus', 'luafuncs', 'luags', 'inventory')


def digest(data):
    return hashlib.sha256(data).hexdigest()


def tool(name, variable):
    requested = os.environ.get(variable) or name
    found = shutil.which(requested)
    if not found and not os.environ.get(variable):
        local = Path.home() / '.local/bin' / name
        if local.is_file() and os.access(local, os.X_OK):
            found = str(local)
    if not found:
        raise RuntimeError('Missing tool: ' + requested)
    return found


def run(args, cwd=ROOT, capture=False):
    result = subprocess.run([str(a) for a in args], cwd=cwd, check=True,
                            stdout=subprocess.PIPE if capture else None)
    return result.stdout if capture else None


def compiler():
    sdk = os.environ.get('GOLDEN_GATE') or str(ROOT / '.orca-sdk-2.2.1')
    sdk = Path(sdk).expanduser().resolve()
    cc = sdk / 'Languages/cc'
    version = re.search(rb'ORCA/C (\d+\.\d+\.\d+)', cc.read_bytes()) if cc.is_file() else None
    if not version or tuple(map(int, version[1].split(b'.'))) < (2, 2, 0):
        raise RuntimeError('Set GOLDEN_GATE to an ORCA/C 2.2.x SDK; see local.mk.example')
    os.environ['GOLDEN_GATE'] = str(sdk)
    return tool('iix', 'IIX'), version[1].decode()


def sdk():
    DEPS.mkdir(exist_ok=True)
    archive = DEPS / SDK_NAME
    if not archive.exists():
        print('Downloading pinned Lua IIgs v0.3.0 SDK', flush=True)
        with urllib.request.urlopen(SDK_URL, timeout=60) as response:
            data = response.read()
        if digest(data) != SDK_SHA256:
            raise RuntimeError('Downloaded SDK checksum mismatch')
        archive.write_bytes(data)
    if digest(archive.read_bytes()) != SDK_SHA256:
        raise RuntimeError('Cached SDK checksum mismatch; remove .deps/' + SDK_NAME)
    with zipfile.ZipFile(archive) as z:
        manifest = json.loads(z.read('SDK-MANIFEST.json'))['containers']['library']
        shk = z.read('LUALIB.SHK')
    if digest(shk) != manifest['archive']['sha256']:
        raise RuntimeError('SDK library container checksum mismatch')
    full = DEPS / 'full'
    if not full.exists():
        with tempfile.TemporaryDirectory(prefix='install-', dir=DEPS) as temp:
            stage = Path(temp)
            (stage / 'LUALIB.SHK').write_bytes(shk)
            run([tool('nulib2', 'NULIB2'), '-x', 'LUALIB.SHK'], cwd=stage)
            (stage / 'LUALIB.SHK').unlink()
            for member in manifest['members']:
                name = member['name']
                source = stage / name
                if digest(source.read_bytes()) != member['sha256']:
                    raise RuntimeError('SDK extraction mismatch: ' + name)
                source.rename(stage / name.lower())
            stage.rename(full)
    # Revalidate every input on reuse, including the generated parseconf.h.
    for member in manifest['members']:
        if digest((full / member['name'].lower()).read_bytes()) != member['sha256']:
            raise RuntimeError('Installed SDK changed: ' + member['name'])
    return full


def doctor():
    iix, version = compiler()
    print('ORCA/C:', version, '\nGoldenGate:', iix)
    print('NuLib2:', tool('nulib2', 'NULIB2'))
    print('Lua SDK: v0.3.0, SHA-256', SDK_SHA256)
    for name, var in [('acx', 'ACX'), ('cp2', 'CP2')]:
        try:
            print('Packaging:', tool(name, var))
        except RuntimeError:
            print('Optional packaging tool missing:', name)


def build():
    iix, version = compiler()
    sdk()
    obj = BUILD / 'obj'
    if obj.exists():
        shutil.rmtree(obj)
    obj.mkdir(parents=True)
    executable = BUILD / 'luademo'
    executable.unlink(missing_ok=True)
    inputs = {}
    for source in sorted(list(ROOT.glob('*.h')) + [ROOT / (u + '.c') for u in UNITS]):
        raw = source.read_bytes()
        inputs[source.name] = digest(raw)
        (obj / source.name).write_bytes(raw.replace(b'\r\n', b'\n').replace(b'\n', b'\r'))
    for unit in UNITS:
        run([iix, 'compile', '-I', '-P', '-D', '+O', unit + '.c', 'cc=-i../../.deps/full'], cwd=obj)
    run([iix, 'link', *UNITS, '../../.deps/full/lvm', '../../.deps/full/lua.lib', 'KEEP=../luademo'], cwd=obj)
    run([iix, 'chtyp', '-t', 'exe', executable])
    record = {'sdk': SDK_NAME, 'sdk_sha256': SDK_SHA256, 'orca_c': version,
              'sources': inputs, 'demo_inputs': {n: digest((ROOT / n).read_bytes()) for n in DEMO_INPUTS},
              'executable_sha256': digest(executable.read_bytes())}
    (BUILD / 'BUILD-MANIFEST.json').write_text(json.dumps(record, indent=2) + '\n')
    print('Built', executable)
    return executable


def test(executable):
    iix, _ = compiler()
    obj = BUILD / 'obj'
    (obj / 'host.c').write_bytes((ROOT / 'tests/host.c').read_bytes().replace(b'\n', b'\r'))
    run([iix, 'compile', '-I', '-P', '-D', '+O', 'host.c', 'cc=-i../../.deps/full'], cwd=obj)
    run([iix, 'link', 'host', 'luags', '../../.deps/full/lvm', '../../.deps/full/lua.lib', 'KEEP=../hosttest'], cwd=obj)
    run([sys.executable, ROOT / 'tests/run.py', iix, executable])


def attrs(cp2, container, name, expected_type, aux=0):
    out = run([cp2, 'get-attr', container, name], capture=True).decode()
    filetype = re.search(r'File Type\s*:\s*\S+\s+0x([0-9a-fA-F]+)', out)
    auxtype = re.search(r'Aux Type\s*:\s*0x([0-9a-fA-F]+)', out)
    if not filetype or not auxtype or (int(filetype[1], 16), int(auxtype[1], 16)) != (expected_type, aux):
        raise RuntimeError('Incorrect ProDOS metadata: ' + name)


def package(executable):
    acx, cp2, nulib = tool('acx', 'ACX'), tool('cp2', 'CP2'), tool('nulib2', 'NULIB2')
    dist = ROOT / 'dist'
    dist.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(prefix='package-', dir=BUILD) as temp:
        stage = Path(temp)
        image, archive = stage / 'luagsdemo.po', stage / 'LUAGSDEMO.SHK'
        members = {'LUADEMO': (executable.read_bytes(), 'EXE', 0xB5)}
        for name in DEMO_INPUTS:
            data = (ROOT / name).read_bytes().replace(b'\r\n', b'\n').replace(b'\n', b'\r')
            members[name.upper()] = (data, 'TXT', 0x04)
        members['README.TXT'] = ((ROOT / 'docs/INSTALL.txt').read_bytes().replace(b'\n', b'\r'), 'TXT', 0x04)
        members['LUALICENSE.TXT'] = ((DEPS / 'full/license.txt').read_bytes(), 'TXT', 0x04)
        run([acx, 'create', '--prodos', '--prodos-order', '-s', '800K', '-n', 'LUAGSDEMO', '-d', image])
        for name, (data, kind, _) in members.items():
            source = stage / name
            source.write_bytes(data)
            run([acx, 'import', '-d', image, '--raw', '-t', kind, '--aux', '0x0000', '-n', name, source])
        run([cp2, 'create-file-archive', archive])
        run([cp2, 'copy', image, archive])
        for container in (image, archive):
            listing = run([cp2, 'catalog', container], capture=True).decode()
            names = set(re.findall(r'^[A-Z0-9?]{3}\s+\$[0-9A-F]{4}\s.*?\s\*?(\S+)\s*$', listing, re.M))
            if names != set(members):
                raise RuntimeError('Unexpected package members: ' + repr(names))
            for name, (data, _, filetype) in members.items():
                command = [acx, 'export', '-d', container, '--raw', name] if container == image else [nulib, '-p', container, name]
                if run(command, capture=True) != data:
                    raise RuntimeError('Package bytes differ: ' + name)
                attrs(cp2, container, name, filetype)
        for source in (image, archive):
            shutil.copyfile(source, dist / source.name)
    for name in ('BUILD-MANIFEST.json', 'TEST-REPORT.json'):
        shutil.copyfile(BUILD / name, dist / name)
    names = ('luagsdemo.po', 'LUAGSDEMO.SHK', 'BUILD-MANIFEST.json', 'TEST-REPORT.json')
    (dist / 'SHA256SUMS').write_text(''.join(digest((dist / n).read_bytes()) + '  ' + n + '\n' for n in names))
    print('Verified transfer packages:', dist)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['sdk', 'doctor', 'build', 'run', 'test', 'package', 'clean'])
    command = parser.parse_args().command
    if command == 'clean':
        if BUILD.exists(): shutil.rmtree(BUILD)
        return
    if command == 'sdk': sdk(); return
    if command == 'doctor': doctor(); return
    executable = build()
    if command == 'run': run([compiler()[0], '--memcheck', executable])
    if command in ('test', 'package'): test(executable)
    if command == 'package': package(executable)


if __name__ == '__main__':
    try:
        main()
    except (OSError, RuntimeError, ValueError, subprocess.CalledProcessError) as error:
        print('Error:', error, file=sys.stderr)
        sys.exit(1)
