# -*- mode: python ; coding: utf-8 -*-

block_cipher = None

a = Analysis(
    ['qt_gif_viewer_fixed.py'],
    pathex=[],
    binaries=[],
    datas=[
        ('vectorlying.webp', '.'),
        ('vectorsit.webp', '.'),
        ('vectormove.webp', '.'),
        ('vectorpick.webp', '.'),
        ('vectorwait.webp', '.'),
    ],
    hiddenimports=[],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)

dlls_to_exclude = [
    '_bz2.pyd',
    '_ctypes.pyd',
    '_decimal.pyd',
    '_hashlib.pyd',
    '_lzma.pyd',
    '_ssl.pyd',
    '_uuid.pyd',
    'api-ms-win-core-console-l1-1-0.dll',
    'api-ms-win-core-datetime-l1-1-0.dll',
    'api-ms-win-core-debug-l1-1-0.dll',
    'api-ms-win-core-errorhandling-l1-1-0.dll',
    'api-ms-win-core-file-l1-1-0.dll',
    'api-ms-win-core-file-l1-2-0.dll',
    'api-ms-win-core-file-l2-1-0.dll',
    'api-ms-win-core-handle-l1-1-0.dll',
    'api-ms-win-core-heap-l1-1-0.dll',
    'api-ms-win-core-interlocked-l1-1-0.dll',
    'api-ms-win-core-libraryloader-l1-1-0.dll',
    'api-ms-win-core-localization-l1-2-0.dll',
    'api-ms-win-core-memory-l1-1-0.dll',
    'api-ms-win-core-namedpipe-l1-1-0.dll',
    'api-ms-win-core-processenvironment-l1-1-0.dll',
    'api-ms-win-core-processthreads-l1-1-0.dll',
    'api-ms-win-core-processthreads-l1-1-1.dll',
    'api-ms-win-core-profile-l1-1-0.dll',
    'api-ms-win-core-rtlsupport-l1-1-0.dll',
    'api-ms-win-core-string-l1-1-0.dll',
    'api-ms-win-core-synch-l1-1-0.dll',
    'api-ms-win-core-synch-l1-2-0.dll',
    'api-ms-win-core-sysinfo-l1-1-0.dll',
    'api-ms-win-core-timezone-l1-1-0.dll',
    'api-ms-win-core-util-l1-1-0.dll',
    'api-ms-win-crt-conio-l1-1-0.dll',
    'api-ms-win-crt-convert-l1-1-0.dll',
    'api-ms-win-crt-environment-l1-1-0.dll',
    'api-ms-win-crt-filesystem-l1-1-0.dll',
    'api-ms-win-crt-heap-l1-1-0.dll',
    'api-ms-win-crt-locale-l1-1-0.dll',
    'api-ms-win-crt-math-l1-1-0.dll',
    'api-ms-win-crt-process-l1-1-0.dll',
    'api-ms-win-crt-runtime-l1-1-0.dll',
    'api-ms-win-crt-stdio-l1-1-0.dll',
    'api-ms-win-crt-string-l1-1-0.dll',
    'api-ms-win-crt-time-l1-1-0.dll',
    'api-ms-win-crt-utility-l1-1-0.dll',
    'libcrypto-1_1.dll',
    'libssl-1_1.dll',
    'unicodedata.pyd',
    'VCRUNTIME140.dll',
    'VCRUNTIME140_1.dll',
    'd3dcompiler_47.dll',
    'libEGL.dll',
    'libGLESv2.dll',
    'MSVCP140.dll',
    'MSVCP140_1.dll',
    'opengl32sw.dll',
    'Qt5DBus.dll',
    'Qt5Network.dll',
    'Qt5Qml.dll',
    'Qt5QmlModels.dll',
    'Qt5Quick.dll',
    'Qt5Svg.dll',
    'Qt5WebSockets.dll',
    'qtuiotouchplugin.dll',
    'qsvgicon.dll',
    'qicns.dll',
    'qico.dll',
    'qjpeg.dll',
    'qsvg.dll',
    'qtga.dll',
    'qtiff.dll',
    'qwbmp.dll',
    'qminimal.dll',
    'qoffscreen.dll',
    'qwebgl.dll',
    'qxdgdesktopportal.dll',
    'qwindowsvistastyle.dll',
    'qtbase_ar.qm',
    'qtbase_bg.qm',
    'qtbase_ca.qm',
    'qtbase_cs.qm',
    'qtbase_da.qm',
    'qtbase_de.qm',
    'qtbase_en.qm',
    'qtbase_es.qm',
    'qtbase_fi.qm',
    'qtbase_fr.qm',
    'qtbase_gd.qm',
    'qtbase_he.qm',
    'qtbase_hu.qm',
    'qtbase_it.qm',
    'qtbase_ja.qm',
    'qtbase_ko.qm',
    'qtbase_lv.qm',
    'qtbase_pl.qm',
    'qtbase_ru.qm',
    'qtbase_sk.qm',
    'qtbase_tr.qm',
    'qtbase_uk.qm',
    'qtbase_zh_TW.qm',
    'qt_ar.qm',
    'qt_bg.qm',
    'qt_ca.qm',
    'qt_cs.qm',
    'qt_da.qm',
    'qt_de.qm',
    'qt_en.qm',
    'qt_es.qm',
    'qt_fa.qm',
    'qt_fi.qm',
    'qt_fr.qm',
    'qt_gd.qm',
    'qt_gl.qm',
    'qt_he.qm',
    'qt_help_ar.qm',
    'qt_help_bg.qm',
    'qt_help_ca.qm',
    'qt_help_cs.qm',
    'qt_help_da.qm',
    'qt_help_de.qm',
    'qt_help_en.qm',
    'qt_help_es.qm',
    'qt_help_fr.qm',
    'qt_help_gl.qm',
    'qt_help_hu.qm',
    'qt_help_it.qm',
    'qt_help_ja.qm',
    'qt_help_ko.qm',
    'qt_help_pl.qm',
    'qt_help_ru.qm',
    'qt_help_sk.qm',
    'qt_help_sl.qm',
    'qt_help_tr.qm',
    'qt_help_uk.qm',
    'qt_help_zh_CN.qm',
    'qt_help_zh_TW.qm',
    'qt_hu.qm',
    'qt_it.qm',
    'qt_ja.qm',
    'qt_ko.qm',
    'qt_lt.qm',
    'qt_lv.qm',
    'qt_pl.qm',
    'qt_pt.qm',
    'qt_ru.qm',
    'qt_sk.qm',
    'qt_sl.qm',
    'qt_sv.qm',
    'qt_tr.qm',
    'qt_uk.qm',
    'qt_zh_CN.qm',
    'qt_zh_TW.qm'
]

a.binaries = [x for x in a.binaries if os.path.split(x[0])[1] not in dlls_to_exclude]

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='VectorViewer',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon='NONE',
) 