"""SDK dependency checks run offline without the IIgs compiler."""
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import zipfile

spec = importlib.util.spec_from_file_location('demo', Path(__file__).resolve().parents[2] / 'tools/demo.py')
demo = importlib.util.module_from_spec(spec)
spec.loader.exec_module(demo)


class SDKChecks(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.patch = patch.object(demo, 'DEPS', self.root)
        self.patch.start()
        self.addCleanup(self.patch.stop)

    def fixture(self, installed=True):
        header, shk = b'/* matching SDK header */\r', b'test archive'
        manifest = {'containers': {'library': {
            'archive': {'sha256': demo.digest(shk)},
            'members': [{'name': 'PARSECONF.H', 'sha256': demo.digest(header)}]}}}
        archive = self.root / demo.SDK_NAME
        with zipfile.ZipFile(archive, 'w') as z:
            z.writestr('SDK-MANIFEST.json', json.dumps(manifest))
            z.writestr('LUALIB.SHK', shk)
        pin = patch.object(demo, 'SDK_SHA256', demo.digest(archive.read_bytes()))
        pin.start(); self.addCleanup(pin.stop)
        if installed:
            (self.root / 'full').mkdir()
            (self.root / 'full/parseconf.h').write_bytes(header)
        return archive

    def test_cached_sdk_needs_no_network_or_extractor(self):
        self.fixture()
        with patch.object(demo.urllib.request, 'urlopen', side_effect=AssertionError('network')), \
             patch.object(demo, 'run', side_effect=AssertionError('extractor')):
            self.assertEqual(demo.sdk(), self.root / 'full')

    def test_corrupt_cached_zip_is_rejected(self):
        archive = self.fixture()
        archive.write_bytes(archive.read_bytes() + b'corruption')
        with self.assertRaisesRegex(RuntimeError, 'Cached SDK checksum mismatch'):
            demo.sdk()

    def test_modified_installed_header_is_rejected(self):
        self.fixture()
        (self.root / 'full/parseconf.h').write_bytes(b'wrong configuration')
        with self.assertRaisesRegex(RuntimeError, 'Installed SDK changed'):
            demo.sdk()

    def test_missing_installed_header_is_rejected(self):
        self.fixture()
        (self.root / 'full/parseconf.h').unlink()
        with self.assertRaises(FileNotFoundError):
            demo.sdk()

    def test_failed_extraction_never_activates_partial_sdk(self):
        self.fixture(installed=False)
        with patch.object(demo, 'tool', return_value='nulib2'), \
             patch.object(demo, 'run', side_effect=RuntimeError('extraction failed')):
            with self.assertRaisesRegex(RuntimeError, 'extraction failed'):
                demo.sdk()
        self.assertFalse((self.root / 'full').exists())
        self.assertEqual(list(self.root.glob('install-*')), [])


if __name__ == '__main__':
    unittest.main()
