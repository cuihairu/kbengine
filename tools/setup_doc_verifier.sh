#!/usr/bin/env bash

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VENV_DIR="$ROOT/.venv-docs"
BUILD_ROOT="$ROOT/.tmp/build"
CHMLIB_VERSION="0.40"
CHMLIB_ARCHIVE="chmlib-${CHMLIB_VERSION}.tar.gz"
CHMLIB_URL="https://www.jedrea.com/chmlib/${CHMLIB_ARCHIVE}"
CHMLIB_SRC_DIR="$BUILD_ROOT/chmlib-${CHMLIB_VERSION}"
CHMLIB_INSTALL_DIR="$BUILD_ROOT/chmlib-install"

mkdir -p "$BUILD_ROOT"

if [ ! -d "$VENV_DIR" ]; then
  python3 -m venv "$VENV_DIR"
fi

"$VENV_DIR/bin/python" -m pip install --upgrade pip >/dev/null
"$VENV_DIR/bin/python" -m pip install beautifulsoup4 pdfplumber >/dev/null

if [ ! -f "$CHMLIB_INSTALL_DIR/lib/libchm.a" ]; then
  if [ ! -f "$BUILD_ROOT/$CHMLIB_ARCHIVE" ]; then
    curl -L "$CHMLIB_URL" -o "$BUILD_ROOT/$CHMLIB_ARCHIVE"
  fi

  if [ ! -d "$CHMLIB_SRC_DIR" ]; then
    tar -xzf "$BUILD_ROOT/$CHMLIB_ARCHIVE" -C "$BUILD_ROOT"
  fi

  (
    cd "$CHMLIB_SRC_DIR"
    ./configure --prefix="$CHMLIB_INSTALL_DIR" >/dev/null
    make -j4 >/dev/null
    make install >/dev/null
  )
fi

export CPPFLAGS="-I$CHMLIB_INSTALL_DIR/include"
export LDFLAGS="-L$CHMLIB_INSTALL_DIR/lib"
export DYLD_LIBRARY_PATH="$CHMLIB_INSTALL_DIR/lib${DYLD_LIBRARY_PATH:+:$DYLD_LIBRARY_PATH}"

"$VENV_DIR/bin/python" -m pip install pychm >/dev/null

echo "文档核对环境已准备完成。"
echo "运行命令："
echo "  DYLD_LIBRARY_PATH=.tmp/build/chmlib-install/lib ./.venv-docs/bin/python tools/verify_authoritative_docs.py"
