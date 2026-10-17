import importlib.util
import tempfile
import unittest
from pathlib import Path, PurePosixPath


ROOT = Path(__file__).resolve().parents[2]


def load_module():
    spec = importlib.util.spec_from_file_location(
        "verify_authoritative_docs", ROOT / "tools" / "verify_authoritative_docs.py"
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class VerifyAuthoritativeDocsTests(unittest.TestCase):
    def setUp(self):
        self.module = load_module()

    def test_exact_page_rejects_any_text_change(self):
        relative_path = PurePosixPath("client/index.md")

        self.assertFalse(
            self.module.verify_api_page("original", "changed", relative_path)
        )

    def test_enhanced_page_requires_same_headings(self):
        relative_path = PurePosixPath("client/KBEngine.md")
        current = "# Client\n\n## player\n\nEnhanced behavior."
        generated = "# Client\n\n## player\n\nOriginal behavior."

        self.assertTrue(
            self.module.verify_api_page(current, generated, relative_path)
        )

    def test_enhanced_page_rejects_missing_heading(self):
        relative_path = PurePosixPath("client/KBEngine.md")
        current = "# Client\n\n## player"
        generated = "# Client\n\n## player\n\n## findEntity"

        self.assertFalse(
            self.module.verify_api_page(current, generated, relative_path)
        )

    def test_rewritten_basetypes_page_keeps_all_chm_types(self):
        relative_path = PurePosixPath("basetypes.md")
        current = "# 基本数据类型\n\nUse `UINT8` or `VECTOR3`.\n"
        generated = "# 基本数据类型\n\nUINT8\nUINT16\nVECTOR2\nVECTOR3\n"

        self.assertFalse(
            self.module.verify_api_page(current, generated, relative_path)
        )


if __name__ == "__main__":
    unittest.main()
